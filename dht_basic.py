# Complete Project Details: https://RandomNerdTutorials.com/raspberry-pi-dht11-dht22-python/
# Based on Adafruit_CircuitPython_DHT Library Example

import serial
import time
import board
import adafruit_dht

# Sensor data pin is connected to GPIO 4
sensor = adafruit_dht.DHT11(board.D4)

ser=serial.Serial('/dev/ttyACM0', 115200, timeout=1)

while True:
    try:
        # Print the values to the serial port
        temperature_c = sensor.temperature
        humidity = sensor.humidity
        sensor_data = f"Temp: {temperature_c}, Humid: {humidity}"
        print(sensor_data)
        ser.write(sensor_data.encode("utf-8"))
        ser.write(b'\n')
    except RuntimeError as error:
        # Errors happen fairly often, DHT's are hard to read, just keep going
        print(error.args[0])
        time.sleep(2.0)
        continue
    except Exception as error:
        sensor.exit()
        raise error

    time.sleep(2.0)
