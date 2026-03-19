"""MQTT client for publishing to SANGI via AWS IoT Core."""

import json
import logging
import time
from awscrt import mqtt, io
from awsiot import mqtt_connection_builder

from sangi_notify.mqtt import topics


class MQTTClient:
    """Publish messages to SANGI via AWS IoT Core MQTT."""

    def __init__(self, config: dict):
        self.logger = logging.getLogger("MQTTClient")

        mqtt_config = config.get("mqtt", {})
        self.endpoint = mqtt_config.get("endpoint")
        self.client_id = mqtt_config.get("client_id", "sangi-notification-monitor")
        self.cert_path = mqtt_config.get("certificate_path")
        self.key_path = mqtt_config.get("private_key_path")
        self.ca_path = mqtt_config.get("root_ca_path")

        self.mqtt_connection = None
        self.is_connected = False

        self.logger.info("MQTTClient initialized")

    # ---- connection lifecycle ----

    def _on_connection_interrupted(self, connection, error, **kwargs):
        self.is_connected = False
        self.logger.warning(f"Connection interrupted. Error: {error}")

    def _on_connection_resumed(self, connection, return_code, session_present, **kwargs):
        self.is_connected = True
        self.logger.info(f"Connection resumed. Return code: {return_code}")

    def connect(self) -> bool:
        try:
            event_loop_group = io.EventLoopGroup(1)
            host_resolver = io.DefaultHostResolver(event_loop_group)
            client_bootstrap = io.ClientBootstrap(event_loop_group, host_resolver)

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
                keep_alive_secs=30,
            )

            self.logger.info(f"Connecting to {self.endpoint} with client ID '{self.client_id}'...")
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
        if self.mqtt_connection and self.is_connected:
            try:
                disconnect_future = self.mqtt_connection.disconnect()
                disconnect_future.result()
                self.is_connected = False
                self.logger.info("MQTT disconnected")
            except Exception as e:
                self.logger.error(f"Error disconnecting MQTT: {e}")

    # ---- publishing helpers ----

    def _publish(self, topic: str, payload: dict) -> bool:
        if not self.is_connected:
            self.logger.warning("Not connected to MQTT, attempting reconnect...")
            if not self.connect():
                return False

        try:
            payload_json = json.dumps(payload)

            if len(payload_json) > 900:
                self.logger.warning(f"Payload size ({len(payload_json)} bytes) is large, may cause issues")

            self.logger.debug(f"Publishing to {topic}: {payload_json}")

            publish_future, _ = self.mqtt_connection.publish(
                topic=topic,
                payload=payload_json,
                qos=mqtt.QoS.AT_LEAST_ONCE,
            )
            publish_future.result()
            return True

        except Exception as e:
            self.logger.error(f"Failed to publish to {topic}: {e}", exc_info=True)
            self.is_connected = False
            return False

    def publish_notification(self, notif_type: str, title: str, message: str) -> bool:
        payload = {
            "type": notif_type,
            "title": title,
            "message": message,
            "timestamp": int(time.time()),
        }
        ok = self._publish(topics.NOTIFICATION_PUSH, payload)
        if ok:
            self.logger.info(f"Published {notif_type} notification: {title}")
        return ok

    def publish_github_stats(self, stats: dict) -> bool:
        payload = {
            "type": "github_stats",
            "username": stats.get("username", "unknown"),
            "repos": stats.get("public_repos", 0),
            "followers": stats.get("followers", 0),
            "following": stats.get("following", 0),
            "contributions": stats.get("total_contributions", 0),
            "commits": stats.get("total_commits", 0),
            "prs": stats.get("total_prs", 0),
            "issues": stats.get("total_issues", 0),
            "stars": stats.get("total_stars", 0),
            "timestamp": int(time.time()),
        }
        ok = self._publish(topics.GITHUB_STATS, payload)
        if ok:
            self.logger.info(f"Published GitHub stats for user: {stats.get('username', 'unknown')}")
        return ok

    def publish_emotion(self, emotion_id: int, ssid: str = None) -> bool:
        payload = {"emotion": emotion_id, "timestamp": int(time.time())}
        if ssid:
            payload["ssid"] = ssid
        ok = self._publish(topics.EMOTION_SET, payload)
        if ok:
            self.logger.info(f"Published emotion change: {emotion_id}")
        return ok

    def publish_discord_stats(self, stats: dict) -> bool:
        payload = {
            "type": "discord_stats",
            "username": stats.get("username", "unknown"),
            "user_id": stats.get("user_id", 0),
            "total_new_messages": stats.get("total_new_messages", 0),
            "dms": stats.get("dms", 0),
            "mentions": stats.get("mentions", 0),
            "unique_senders": stats.get("unique_senders", 0),
            "timestamp": int(time.time()),
            "messages": stats.get("messages", [])[:3],
        }

        payload_json = json.dumps(payload)
        if len(payload_json) > 900:
            self.logger.warning(f"Discord payload size ({len(payload_json)} bytes) is large, truncating messages")
            payload["messages"] = []

        ok = self._publish(topics.DISCORD_STATS, payload)
        if ok:
            self.logger.info(f"Published Discord stats: {stats.get('total_new_messages', 0)} new messages")
        return ok

    def test_connection(self) -> bool:
        return self.publish_notification("system", "Notification Monitor", "Service started")
