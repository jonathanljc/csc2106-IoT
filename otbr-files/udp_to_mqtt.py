import socket
import paho.mqtt.client as mqtt
import json
import re
import random

UDP_IP = "::"
UDP_PORT = 1234

MQTT_BROKER = "localhost"
MQTT_TOPIC = "thread/sensor"

sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

client = mqtt.Client()
client.connect(MQTT_BROKER, 1883, 60)
client.loop_start()

print("Listening")

while True:
	data, addr = sock.recvfrom(1024)
	message = data.decode("utf-8", errors="ignore").strip()
	print(f"Received from {addr}: {message}")
	
	match = re.search(r"Temp:\s(\d+).*Humid:\s*(\d+)", message)
	if match:
		temp = float(match.group(1)) 
		humid = float(match.group(2))

		mqtt_payload = json.dumps({"temp": temp, "humid": humid})
		print(f"Publishing: {mqtt_payload}")

		client.publish(MQTT_TOPIC, mqtt_payload)

