"""
MQTT publisher for sending notifications to SANGI via AWS IoT Core
"""

import json
import logging
import time
from awscrt import mqtt, io
from awsiot import mqtt_connection_builder


class MQTTPublisher:
    """Publish notifications to SANGI via AWS IoT Core MQTT"""
    
    def __init__(self, config):
        """
        Initialize MQTT publisher
        
        Args:
            config: Configuration dictionary with MQTT settings
        """
        self.config = config
        self.logger = logging.getLogger('MQTTPublisher')
        
        mqtt_config = config.get('mqtt', {})
        self.endpoint = mqtt_config.get('endpoint')
        self.client_id = mqtt_config.get('client_id', 'sangi-notification-monitor')
        self.cert_path = mqtt_config.get('certificate_path')
        self.key_path = mqtt_config.get('private_key_path')
        self.ca_path = mqtt_config.get('root_ca_path')
        self.topic = mqtt_config.get('topic', 'sangi/notification/push')
        
        self.mqtt_connection = None
        self.is_connected = False
        
        self.logger.info("MQTTPublisher initialized")
    
    def _on_connection_interrupted(self, connection, error, **kwargs):
        """Callback when connection is interrupted"""
        self.is_connected = False
        self.logger.warning(f"Connection interrupted. Error: {error}")
    
    def _on_connection_resumed(self, connection, return_code, session_present, **kwargs):
        """Callback when connection is resumed"""
        self.is_connected = True
        self.logger.info(f"Connection resumed. Return code: {return_code}")
    
    def connect(self):
        """Establish MQTT connection to AWS IoT Core"""
        try:
            # Create event loop group
            event_loop_group = io.EventLoopGroup(1)
            host_resolver = io.DefaultHostResolver(event_loop_group)
            client_bootstrap = io.ClientBootstrap(event_loop_group, host_resolver)
            
            # Build MQTT connection
            self.mqtt_connection = mqtt_connection_builder.mtls_from_path(
                endpoint=self.endpoint,
                cert_filepath=self.cert_path,
                pri_key_filepath=self.key_path,
                client_bootstrap=client_bootstrap,
                ca_filepath=self.ca_path,
                on_connection_interrupted=self._on_connection_interrupted,
                on_connection_resumed=self._on_connection_resumed,
                client_id=self.client_id,
                clean_session=False,
                keep_alive_secs=30
            )
            
            self.logger.info(f"Connecting to {self.endpoint} with client ID '{self.client_id}'...")
            
            # Connect
            connect_future = self.mqtt_connection.connect()
            connect_future.result()
            
            self.is_connected = True
            self.logger.info("MQTT connection established")
            
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to connect to MQTT: {e}", exc_info=True)
            self.is_connected = False
            return False
    
    def disconnect(self):
        """Disconnect from MQTT"""
        if self.mqtt_connection and self.is_connected:
            try:
                disconnect_future = self.mqtt_connection.disconnect()
                disconnect_future.result()
                self.is_connected = False
                self.logger.info("MQTT disconnected")
            except Exception as e:
                self.logger.error(f"Error disconnecting MQTT: {e}")
    
    def publish_notification(self, notif_type, title, message):
        """
        Publish notification to SANGI
        
        Args:
            notif_type: Type of notification (discord, github, whatsapp, etc.)
            title: Notification title
            message: Notification message
        
        Returns:
            bool: True if published successfully
        """
        if not self.is_connected:
            self.logger.warning("Not connected to MQTT, attempting reconnect...")
            if not self.connect():
                return False
        
        try:
            # Build payload
            payload = {
                'type': notif_type,
                'title': title,
                'message': message,
                'timestamp': int(time.time())
            }
            
            payload_json = json.dumps(payload)
            
            # Publish
            self.logger.debug(f"Publishing to {self.topic}: {payload_json}")
            
            publish_future, packet_id = self.mqtt_connection.publish(
                topic=self.topic,
                payload=payload_json,
                qos=mqtt.QoS.AT_LEAST_ONCE
            )
            
            # Wait for publish to complete
            publish_future.result()
            
            self.logger.info(f"Published {notif_type} notification: {title}")
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to publish notification: {e}", exc_info=True)
            self.is_connected = False
            return False
    
    def test_connection(self):
        """Test MQTT connection by publishing a test message"""
        return self.publish_notification(
            'system',
            'Notification Monitor',
            'Service started'
        )
