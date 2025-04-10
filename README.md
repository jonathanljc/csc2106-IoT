# CSC2106 - IoT

## Setting Up the Thread Border Router on Raspberry Pi 4 and nRF52840 end devices

This guide outlines the steps to set up an **OpenThread Border Router (OTBR)** on a **Raspberry Pi 4** and the **nRF52840** end devices

### **Prerequisites**
- Raspberry Pi 4 running **Raspberry Pi OS**.
- Internet connectivity (via `wlan0` or `eth0`).
- At least 2x nRF53840 dongles

## **Installation Steps**
### **Step 1: Install the necessary applications and SDKs (Windows)**
1. Install [nRF Command Line Tools](https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools/Download) and [nRF Connect for Desktop](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-Desktop/Download)
2. In Visual Studio Code, install the **nRF Connect for VS Code Extension Pack** extension
3. Open the nRF Connect for Desktop app, install **Toolchain Manager** and **Programmer**
4. Once installed, open **Toolchain Manager** and install the latest **nRF Connect SDK**
    - This may take awhile to install
    - Verify your installation by clicking the clicking the dropdown menu beside the SDK, click "Open Bash" and run `west --version`

### **Step 2: Building the CLI and Coprocessor applications**
**Note:** The CLI file has been modified to receive Temperature Data via UART and BLE protocol. The original CLI file can be obtained via the samples code provided by Nordic itself.
See image below for example:
![image](https://github.com/user-attachments/assets/191048bf-00dc-400c-a424-dad3670ea05e)

- Alternatively, you can use the precompiled hex binary files (original, without modifications) prepared for both **CLI (edge device)** and **Co-Processor (OTBR)**:
  - **merged.hex (Co-Processor) – flash this to the dongle(TBR) using nRF Connect for Desktop**
  - **zephyr.hex (CLI) – flash this to the dongle(edge device) using nRF Connect for Desktop**
  

**Otherwise Modified CLI:**

1. Clone this repository and open it in VSCode
2. Open the nRF Connect extension on the left, there should be 2 applications (cli and coprocessor) under the "Applications" menu
3. In this "Applications" menu, select "+ Add Build configuration". A new screen will appear.
4. In this screen, under "Board Target", select "nrf52840dongle/nrf52840".
5. Leave everything else as default and press "Build Configuration".
    - It will begin to build the application
    - Do the same for both cli and coprocessor
    - **cli is for end device and coprocessor is for Border Router**

### **Step 4: Flashing the nRF52840 dongles**
1. After building the 2 applications, open Programmer in nRF Connect for Desktop application
2. Plug in the nRF52840 and select the board on the top left "Select Device"
3. Press "Add File" to add the applcation .hex file
    - For cli (end device), the location is `/csc2106-IoT/cli/build/cli/zephyr/zephyr.hex`
    - For coprocessor (OTBR), the location is `/csc2106-IoT/coprocessor/build/coprocessor/zephyr/zephyr.hex`
4. Once addded, press "Write" and wait for completion
5. Do the same for the other application
    - e.g. if you did cli, do the same for coprocessor and vice-versa
    - When switching applications, press the "Clear Files" button to clear the file memory layout so that only 1 of cli/coprocessor is flashed, not both at the same time

### **Step 5a: Set up OTBR**
1. Turn on your Raspberry Pi4b and SSH into it
2. Run the following commands to clone the OTBR application and install the service:
```bash
git clone https://github.com/openthread/ot-br-posix.git --depth 1
cd ot-br-posix
./script/bootstrap
INFRA_IF_NAME=wlan0 ./script/setup  # For Wi-Fi
INFRA_IF_NAME=eth0 ./script/setup   # For Ethernet
```
3. Verify that the OTBR service is successfully installed, and then reboot:
```bash
sudo service otbr-agent status  # Check if OTBR service exists
sudo reboot
```
4. Plug in the nRF52840 dongle that has the coprocessor installed
    - You can verify the otbr-agent service again, it should be "Running" now with `sudo service otbr-agent status`
    - Troubleshooting - You can restart the service with `sudo service otbr-agent restart`
5. Create the Thread Network is the following commands:
```bash
sudo ot-ctl dataset init new
sudo ot-ctl dataset commit active
sudo ot-ctl ifconfig up
sudo ot-ctl thread start
```
6. Verify the Thread Network is now successfully created:
```bash
sudo ot-ctl state          # Check OTBR state (should be 'leader')
sudo ot-ctl netdata show   # Display Thread network data
sudo ot-ctl ipaddr         # List assigned IP addresses
```

7. Copy the information of the OTBR dataset
    - Run `sudo ot-ctl dataset active`
    - Take note of the Wake-Up Channel number, Ext PAN ID, PAN ID, Network Name, Network Key, PSKc and Security Policy. (Ideally, write in a notepad)

### **Step 5b: Set up end device**
1. Plug in the nRF52840 dongle with the cli application into your laptop
    - Take note of the COMXX number. You can find this in Programmer in nRF Connect for Desktop with "Select Device". The COM number is can be seen in the dropdown menu when selecting the device
2. Open PuTTY, select "Serial", enter the COM number (e.g. COM14) and set the Speed to "115200". Open the connection.
3. Once connected, start configuring the node device to join the Thread network
    - Run the following commands with the parameters that was noted down in Step 5a (from the OTBR Pi)
    - `ot dataset channel <number>`
    - `ot dataset panid <id>`
    - `ot dataset expanid <id>`
    - `ot dataset networkname <name>`
    - `ot dataset pskc <psk>`
    - `ot dataset securitypolicy <policy>`
    - `ot dataset commit active`
    - `ot ifconfig up`
    - `ot thread start`
    - Example usage:
    ```bash
    ot dataset channel 19
    ot dataset panid 0x001e
    ot dataset extpanid 820a67d9ce258658
    ot dataset networkname OpenThread-001e
    ot dataset networkkey be5c7f0c38d3da24716efe0770567f77
    ot dataset pskc 1d88795a53617bf0fa29dd4de897f450
    ot dataset securitypolicy 672 onrc 0
    ot dataset commit active
    ot ifconfig up
    ot thread start
    ```
    **REMEMBER TO CHANGE THE VALUES IN THIS EXAMPLE TO MATCH YOUR OTBR PI DONGLE**
    
4. Once done, verify that your node is connected to the network with `ot state`

### **Step 6: Test UDP Messages**
1. Open the UDP sockets in both the end devices nodes and OTBR
    - In OTBR: `sudo ot-ctl udp open`
    - In nodes: `ot udp open`
2. In OTBR, get the IPv6 address of the OTBR.
    - Run `sudo ot-ctl netdata show` and take note the Prefixes
    - Run `sudo ot-ctl ipaddr` and take note of the address WITH the Prefix from previous command
    - This is the OTBR address
3. In OTBR, install netcat and start listening
    - Install netcat with `sudo apt install netcat-openbsd`
    - Start listening with `nc -6 -u -l 1234` (1234 is the port number)
4. In end device node, send a message to the OTBR
    - Run `ot udp send <IPv6 of OTBR> <Port> <message>`
    - Example: `ot udp send fd97:b3f:1a22:1:7ef1:9f0f:c300:f9fc 1234 'hello'`
5. The message should be received in the OTBR netcat

## **Installing applications on OTBR Pi**
1. Download the udp_to_mqtt.py from the repository
2. Install, run and configure the Home Assistant container with Docker
    - Install docker with `sudo apt install docker.io`
    - Download and Run the Home Assistant image
    ```bash
    sudo docker run -d \
    --name homeassistant \
    --privileged \
    --restart=unless-stopped \
    -e TZ=Asia/Singapore \
    -v /dockerHA:/config \
    -v /run/dbus:/run/dbus:ro \
    --network=host \
    ghcr.io/home-assistant/home-assistant:stable
    ```
    - Download the `configuration.yaml` and replace it in `/dockerHA/configuration` in your OTBR Pi
    - Restart the container for the changes to take affect with `sudo docker restart homeassistant`
3. Install, run and configure the MQTT broker
    - Install with `sudo apt install mosquitto mosquitto-clients -y`
    - Download the `mosquitto.conf` from the repository and replace the one in `/etc/mosquitto/mosquitto.conf`
    - Run the broker using `sudo mosquitto -d -c "/etc/mosquitto/mosquitto.conf"`
4. Run the `udp_to_mqtt.py` script
    - Would need to install `paho-mqtt` with `sudo apt install python3-paho-mqtt`
5. Open a browser in another machine (WITHIN THE SAME NETWORK) and access the Home Assistant dashboard with `http://<PI IP>:8123`

## **DHT-11 Sensor Set-Up**
### step 1 (ideally to create your folder directory in your rpi-4 desktop for easier access):
```bash
mkdir dht_test
cd dht_test
python3 -m venv myenv
ls -l
source myenv/bin/activate
python3 -m pip install adafruit-circuitpython-dht
pip install paho-mqtt
```
### step 2:
```bash
nano dht_basic.py
```
- copy-paste DHT-11 code (change accordingly to your gpio you want to use in that default code is gpio-4)
- ctr + o (to write into file)
- ctr + x (to exit the file)

### step 3:
```bash
python dht_basic.py
```

## **IR-TX to IR-RX**
- Now the dongle nrf52840 is **configured** with both **BLE + Thread** capabilities. (ble configuration can be found under the csc2106-IoT/cli/src/ble.c folder directory path)
- Current flow is basically from TBR -> MQTT client -> nrf52840 dongle BLE protocol -> Send notification message "Triggered".
- M5stick(IR Tx) node which is **configured** with BLE receive the notification message "Trigger" -> Trigger IR to send signal (Encode signal "0x20DF10EF" 32bit)
- M5stick(IR Rx) node which is **not configured** with BLE recieve the signal and decode the message -> Turn on motor (Decode signal == Encode signal)

## **Additional Resources**
For more details, refer to the official OpenThread documentation:
- [OpenThread Border Router Setup Guide](https://openthread.io/codelabs/openthread-border-router#5)
- [Thread Tools Nordic Docs](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/protocols/thread/tools.html#configuring-a-radio-co-processor)
- [Thread UDP API](https://openthread.io/reference/group/api-udp)
