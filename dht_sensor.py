import serial
import time
import board
import adafruit_dht
import paho.mqtt.client as mqtt

# Sensor data pin is connected to GPIO 4
sensor = adafruit_dht.DHT11(board.D4)
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

# MQTT Subscribing (comment out if it is not the BLE device)
MQTT_BROKER = "192.168.51.178"
SUBSCRIBE_TOPIC = "thread/sensorExample"

def on_message(client, userdata, msg):
    received_message = msg.payload.decode("utf-8")
    print(f"Received MQTT message on {msg.topic}: {received_message}")

    # If the received message is "FAN", send it to the serial connection
    if received_message.strip().upper() == "FAN":
        ser.write(b"FAN\n")
        print("Sent 'FAN' to serial")

client = mqtt.Client()
client.on_message = on_message
client.connect(MQTT_BROKER, 1883, 60)
client.subscribe(SUBSCRIBE_TOPIC)
client.loop_start()
# End of MQTT Subscribing

while True:
    try:
        # Read sensor values
        temperature_c = sensor.temperature
        humidity = sensor.humidity
        sensor_data = f"Temp: {temperature_c}, Humid: {humidity}"
        print(sensor_data)
        
        # Send sensor data over serial
        ser.write(sensor_data.encode("utf-8"))
        ser.write(b'\n')
        
    except RuntimeError as error:
        print(error.args[0])
        time.sleep(2.0)
        continue
    except Exception as error:
        sensor.exit()
        raise error

    time.sleep(2.0)
