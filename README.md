# CSC2106 - IoT

## Setting Up the Thread Border Router on Raspberry Pi 4

This guide outlines the steps to set up an **OpenThread Border Router (OTBR)** on a **Raspberry Pi 4**.

### **Prerequisites**
- Raspberry Pi 4 running **Raspberry Pi OS**.
- A **Thread Radio Co-Processor (RCP)** (e.g., Nordic nRF52840 Dongle).
- Internet connectivity (via `wlan0` or `eth0`).

## **Installation Steps**

### **Step 1: Clone and Install OTBR**
```bash
git clone https://github.com/openthread/ot-br-posix.git --depth 1
cd ot-br-posix
./script/bootstrap
INFRA_IF_NAME=wlan0 ./script/setup  # For Wi-Fi
INFRA_IF_NAME=eth0 ./script/setup   # For Ethernet
```

### **Step 2: Start OTBR Service and Reboot**
```bash
sudo service otbr-agent status  # Check if OTBR service is running
sudo reboot
```

### **Step 3: Flash RCP Firmware**
- Flash the **RCP sample code** onto the **Thread dongle**.
- Unplug the dongle, then plug it back into the Raspberry Pi 4.

### **Step 4: Restart OTBR Service**
```bash
sudo service otbr-agent restart
sudo service otbr-agent status
```

### **Step 5: Initialize the Thread Network**
```bash
sudo ot-ctl dataset init new
sudo ot-ctl dataset commit active
sudo ot-ctl ifconfig up
sudo ot-ctl thread start
```

### **Step 6: Verify Network Configuration**
```bash
sudo ot-ctl state          # Check OTBR state (should be 'leader')
sudo ot-ctl netdata show   # Display Thread network data
sudo ot-ctl ipaddr         # List assigned IP addresses
```

### **Step 7: OpenThread CLI**

On leader node (Thread Border Router), run this command to check your leader dataset.
```bash
sudo ot-ctl dataset active
```
On End Device node, run the respective command based on your dataset must be same as your leader dataset.
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
Check Status:
```bash
ot state
```

## **Additional Resources**
For more details, refer to the official OpenThread documentation:
[OpenThread Border Router Setup Guide](https://openthread.io/codelabs/openthread-border-router#5)



