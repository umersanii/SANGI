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
        self.stats_topic = mqtt_config.get('stats_topic', 'sangi/github/stats')
        
        self.mqtt_connection = None
        self.is_connected = False
        
        # Discord stats topic
        self.discord_stats_topic = mqtt_config.get('discord_stats_topic', 'sangi/discord/stats')
        
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
    
    def publish_github_stats(self, stats):
        """
        Publish GitHub statistics to SANGI
        
        Args:
            stats: Dictionary containing GitHub statistics
        
        Returns:
            bool: True if published successfully
        """
        if not self.is_connected:
            self.logger.warning("Not connected to MQTT, attempting reconnect...")
            if not self.connect():
                return False
        
        try:
            # Build compact payload optimized for ESP32
            # Only include essential stats to fit in buffer
            payload = {
                'type': 'github_stats',
                'username': stats.get('username', 'unknown'),
                'repos': stats.get('public_repos', 0),
                'followers': stats.get('followers', 0),
                'following': stats.get('following', 0),
                'contributions': stats.get('total_contributions', 0),
                'commits': stats.get('total_commits', 0),
                'prs': stats.get('total_prs', 0),
                'issues': stats.get('total_issues', 0),
                'stars': stats.get('total_stars', 0),
                'timestamp': int(time.time())
            }
            
            payload_json = json.dumps(payload)
            
            # Check payload size (ESP32 MQTT buffer is 1024 bytes)
            if len(payload_json) > 900:
                self.logger.warning(f"Payload size ({len(payload_json)} bytes) is large, may cause issues")
            
            # Publish
            self.logger.debug(f"Publishing to {self.stats_topic}: {payload_json}")
            
            publish_future, packet_id = self.mqtt_connection.publish(
                topic=self.stats_topic,
                payload=payload_json,
                qos=mqtt.QoS.AT_LEAST_ONCE
            )
            
            # Wait for publish to complete
            publish_future.result()
            
            self.logger.info(f"Published GitHub stats for user: {stats.get('username', 'unknown')}")
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to publish GitHub stats: {e}", exc_info=True)
            self.is_connected = False
            return False
    
    def publish_emotion(self, emotion_id, ssid=None):
        """
        Publish emotion change command to SANGI
        
        Args:
            emotion_id: Emotion ID (0-15)
            ssid: Optional SSID for validation
        
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
                'emotion': emotion_id,
                'timestamp': int(time.time())
            }
            
            # Add SSID if provided
            if ssid:
                payload['ssid'] = ssid
            
            payload_json = json.dumps(payload)
            
            # Publish to emotion topic
            emotion_topic = 'sangi/emotion/set'
            
            self.logger.debug(f"Publishing to {emotion_topic}: {payload_json}")
            
            publish_future, packet_id = self.mqtt_connection.publish(
                topic=emotion_topic,
                payload=payload_json,
                qos=mqtt.QoS.AT_LEAST_ONCE
            )
            
            # Wait for publish to complete
            publish_future.result()
            
            self.logger.info(f"Published emotion change: {emotion_id}")
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to publish emotion: {e}", exc_info=True)
            self.is_connected = False
            return False
    
    def publish_discord_stats(self, stats):
        """
        Publish Discord message statistics to SANGI
        
        Args:
            stats: Dictionary containing Discord message statistics
        
        Returns:
            bool: True if published successfully
        """
        if not self.is_connected:
            self.logger.warning("Not connected to MQTT, attempting reconnect...")
            if not self.connect():
                return False
        
        try:
            # Build compact payload optimized for ESP32
            payload = {
                'type': 'discord_stats',
                'username': stats.get('username', 'unknown'),
                'user_id': stats.get('user_id', 0),
                'total_new_messages': stats.get('total_new_messages', 0),
                'dms': stats.get('dms', 0),
                'mentions': stats.get('mentions', 0),
                'unique_senders': stats.get('unique_senders', 0),
                'timestamp': int(time.time()),
                'messages': stats.get('messages', [])[:3]  # Only send last 3 messages for preview
            }
            
            payload_json = json.dumps(payload)
            
            # Check payload size (ESP32 MQTT buffer is 1024 bytes)
            if len(payload_json) > 900:
                self.logger.warning(f"Discord payload size ({len(payload_json)} bytes) is large, truncating messages")
                # Remove message previews if too large
                payload['messages'] = []
                payload_json = json.dumps(payload)
            
            # Publish
            self.logger.debug(f"Publishing to {self.discord_stats_topic}: {payload_json}")
            
            publish_future, packet_id = self.mqtt_connection.publish(
                topic=self.discord_stats_topic,
                payload=payload_json,
                qos=mqtt.QoS.AT_LEAST_ONCE
            )
            
            # Wait for publish to complete
            publish_future.result()
            
            self.logger.info(f"Published Discord stats: {stats.get('total_new_messages', 0)} new messages")
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to publish Discord stats: {e}", exc_info=True)
            self.is_connected = False
            return False

