import socket
import paho.mqtt.client as mqtt
import json
import re
import random

UDP_IP = "::"
UDP_PORT = 1234

MQTT_BROKER = "localhost"
MQTT_TOPICS = ["thread/sensor1", "thread/sensor2", "thread/sensor3", "thread/sensor4"]

sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

client = mqtt.Client()
client.connect(MQTT_BROKER, 1883, 60)
client.loop_start()

print("Listening for UDP packets...")

# Dictionary to track IPv6 addresses and their assigned topics
ipv6_to_topic = {}
next_topic_index = 0  # Round-robin counter for topic assignment

while True:
    data, addr = sock.recvfrom(1024)
    ipv6_addr = addr[0]  # Extract IPv6 address
    message = data.decode("utf-8", errors="ignore").strip()

    print(f"Received from {ipv6_addr}: {message}")

    # Assign topic if the address is new
    if ipv6_addr not in ipv6_to_topic:
        ipv6_to_topic[ipv6_addr] = MQTT_TOPICS[next_topic_index]
        next_topic_index = (next_topic_index + 1) % len(MQTT_TOPICS)  # Cycle through topics
    
    mqtt_topic = ipv6_to_topic[ipv6_addr]

    # Extract temperature and humidity
    match = re.search(r"Temp:\s(\d+).*Humid:\s*(\d+)", message)
    if match:
        temp = float(match.group(1))
        humid = float(match.group(2))

        mqtt_payload = json.dumps({"temp": temp, "humid": humid, "source": ipv6_addr})
        print(f"Publishing to {mqtt_topic}: {mqtt_payload}")

        client.publish(mqtt_topic, mqtt_payload)
