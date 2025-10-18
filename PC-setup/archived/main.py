import json
import time
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient

# Emotion mapping (matches SANGI's EmotionState enum)
EMOTIONS = {
    'idle': 0,
    'happy': 1,
    'sad': 2,
    'angry': 3,
    'surprised': 4,
    'confused': 5,
    'sleepy': 6,
    'thinking': 7,
    'excited': 8,
    'love': 9
}

class SANGIEmotionController:
    def __init__(self, cert_path, key_path, root_ca_path, endpoint, client_id):
        self.client = AWSIoTMQTTClient(client_id)
        self.client.configureEndpoint(endpoint, 8883)
        self.client.configureCredentials(root_ca_path, key_path, cert_path)
        
        # Connection settings
        self.client.configureAutoReconnectBackoffTime(1, 32, 20)
        self.client.configureOfflinePublishQueueing(-1)
        self.client.configureDrainingFrequency(2)
        self.client.configureConnectDisconnectTimeout(10)
        self.client.configureMQTTOperationTimeout(5)
        
        self.client.connect()
        print("✅ Connected to AWS IoT Core")
    
    def set_emotion(self, emotion_name):
        """Send emotion command to SANGI"""
        if emotion_name not in EMOTIONS:
            print(f"❌ Unknown emotion: {emotion_name}")
            return False
        
        payload = json.dumps({
            'emotion': EMOTIONS[emotion_name],
            'source': 'workspace_monitor',
            'timestamp': int(time.time())
        })
        
        self.client.publish('sangi/emotion/set', payload, 1)
        print(f"📤 Set SANGI emotion → {emotion_name.upper()}")
        return True
    
    def disconnect(self):
        self.client.disconnect()

# Example usage
if __name__ == '__main__':
    controller = SANGIEmotionController(
        cert_path='/path/to/device.cert.pem',
        key_path='/path/to/device.private.key',
        root_ca_path='/path/to/AmazonRootCA1.pem',
        endpoint='your-endpoint.iot.region.amazonaws.com',
        client_id='pc-publisher'
    )
    
    # Direct emotion control
    controller.set_emotion('happy')
    time.sleep(5)
    controller.set_emotion('thinking')