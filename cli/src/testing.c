// /*
//  * Copyright (c) 2020 Nordic Semiconductor ASA
//  *
//  * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
//  */

//  #include <zephyr/kernel.h>
//  #include <stdio.h>
//  #include <zephyr/logging/log.h>
//  #include <openthread/platform/radio.h>
//  #include <openthread/thread.h>
//  #include <openthread/udp.h>
//  #include <zephyr/drivers/uart.h>
//  #include <zephyr/usb/usb_device.h>
//  #include <zephyr/drivers/sensor.h> 
 
//  static const char UDP_DEST_ADDR[] = "fd34:3efd:9d7:1:b089:a484:57bd:518f";  // IPv6 Address of OTBR
//  static const uint16_t UDP_PORT = 1234;  // Choose a suitable UDP port
 
//  static char udpPayload[70];  // Payload to send over UDP
//  static otInstance *instance;
//  static otUdpSocket udpSocket;
//  LOG_MODULE_REGISTER(cli_sample, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);
 
//  /* Function to get the current timestamp as a formatted string */
//  static const char *now_str(void)
//  {
//      static char buf[20]; /* HH:MM:SS.MMM */
//      uint32_t now = k_uptime_get_32();
//      unsigned int ms = now % MSEC_PER_SEC;
//      unsigned int s;
//      unsigned int min;
//      unsigned int h;
 
//      now /= MSEC_PER_SEC;
//      s = now % 60U;
//      now /= 60U;
//      min = now % 60U;
//      now /= 60U;
//      h = now;
 
//      snprintf(buf, sizeof(buf), "%02u:%02u:%02u.%03u", h, min, s, ms);
//      return buf;
//  }
 
//  /* Function to read temperature and humidity from DHT-11 */
//  void get_sensor_data(char *buffer, size_t buffer_size)
//  {
//      const struct device *const dht22 = DEVICE_DT_GET_ONE(aosong_dht);
     
//      if (!device_is_ready(dht22)) {
//          LOG_ERR("DHT Sensor %s is NOT ready!", dht22->name);
//          snprintf(buffer, buffer_size, "[%s] Sensor Error", now_str());
//          return;
//      }
 
//      int rc = sensor_sample_fetch(dht22);
//      if (rc != 0) {
//          LOG_ERR("Sensor fetch failed: %d\n", rc);
//          snprintf(buffer, buffer_size, "[%s] Sensor Fetch Error", now_str());
//          return;
//      }
 
//      struct sensor_value temperature;
//      struct sensor_value humidity;
 
//      rc = sensor_channel_get(dht22, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
//      if (rc == 0) {
//          rc = sensor_channel_get(dht22, SENSOR_CHAN_HUMIDITY, &humidity);
//      }
 
//      if (rc != 0) {
//          LOG_ERR("Sensor read failed: %d", rc);
//          snprintf(buffer, buffer_size, "[%s] Sensor Read Error", now_str());
//          return;
//      }
 
//      snprintf(buffer, buffer_size, "[%s] Temp: %.1f°C, Humid: %.1f%% RH\n",
//               now_str(),
//               sensor_value_to_double(&temperature),
//               sensor_value_to_double(&humidity));
//  }
 
//  /* Function to send UDP packet with sensor data */
//  void udpSend(otInstance *instance)
//  {
//      otError error;
//      otMessage *message;
//      otMessageInfo messageInfo;
 
//      get_sensor_data(udpPayload, sizeof(udpPayload));  // Get sensor data
 
//      // Create a new message
//      message = otUdpNewMessage(instance, NULL);
//      if (message == NULL)
//      {
//          LOG_ERR("Failed to allocate message");
//          return;
//      }
 
//      // Append the payload to the message
//      error = otMessageAppend(message, udpPayload, strlen(udpPayload));
//      if (error != OT_ERROR_NONE)
//      {
//          LOG_ERR("Failed to append message: %d", error);
//          otMessageFree(message);
//          return;
//      }
 
//      // Set the message info
//      memset(&messageInfo, 0, sizeof(messageInfo));
//      otIp6AddressFromString(UDP_DEST_ADDR, &messageInfo.mPeerAddr);
//      messageInfo.mPeerPort = UDP_PORT;
 
//      // Send the message
//      error = otUdpSend(instance, &udpSocket, message, &messageInfo);
//      if (error != OT_ERROR_NONE)
//      {
//          LOG_ERR("Failed to send UDP message: %d", error);
//          otMessageFree(message);
//      }
//  }
 
//  int main(void)
//  {
//      // Initialize OpenThread instance
//      instance = otInstanceInitSingle();
//      if (instance == NULL)
//      {
//          LOG_ERR("Failed to initialize OpenThread instance");
//          return -1;
//      }
 
//      // Bring up the IPv6 interface
//      otError error = otIp6SetEnabled(instance, true);
//      if (error != OT_ERROR_NONE)
//      {
//          LOG_ERR("Failed to bring up IPv6 interface: %d", error);
//          return -1;
//      }
 
//      // Start the Thread network
//      error = otThreadSetEnabled(instance, true);
//      if (error != OT_ERROR_NONE)
//      {
//          LOG_ERR("Failed to start Thread network: %d", error);
//          return -1;
//      }
 
//      // Initialize the UDP socket
//      memset(&udpSocket, 0, sizeof(udpSocket));
//      otUdpOpen(instance, &udpSocket, NULL, NULL);
 
//      while (1)
//      {
//          udpSend(instance);
//          k_sleep(K_SECONDS(2)); // Send a message every 2 seconds
//      }
 
//      return 0;
//  }
 