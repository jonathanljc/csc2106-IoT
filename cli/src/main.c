/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

 #include <zephyr/kernel.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <zephyr/logging/log.h>
 #include <openthread/platform/radio.h>
 #include <openthread/thread.h>
 #include <openthread/udp.h>
 #include <zephyr/drivers/uart.h>
 #include <zephyr/usb/usb_device.h>
 #include <openthread/instance.h>
 #include "ble.h"
 
 LOG_MODULE_REGISTER(cli_sample, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);
 
 static const char UDP_DEST_ADDR[] = "fd40:3477:6aa4:1:9da4:1bf2:475e:efe9"; // OTBR IPv6 Address
 static const uint16_t UDP_PORT = 1234;  // Choose a suitable UDP port
 
 static char udpPayload[50];  // Buffer for UDP payload
 static otInstance *instance;
 static otUdpSocket udpSocket;
 
 static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
 
 void udpSend(void) {
	 otError error;
	 otMessage *message;
	 otMessageInfo messageInfo;
 
	 message = otUdpNewMessage(instance, NULL);
	 if (message == NULL) {
		 LOG_ERR("Failed to allocate message");
		 return;
	 }
 
	 error = otMessageAppend(message, udpPayload, strlen(udpPayload));
	 if (error != OT_ERROR_NONE) {
		 LOG_ERR("Failed to append message: %d", error);
		 otMessageFree(message);
		 return;
	 }
 
	 memset(&messageInfo, 0, sizeof(messageInfo));
	 otIp6AddressFromString(UDP_DEST_ADDR, &messageInfo.mPeerAddr);
	 messageInfo.mPeerPort = UDP_PORT;
 
	 error = otUdpSend(instance, &udpSocket, message, &messageInfo);
	 if (error != OT_ERROR_NONE) {
		 LOG_ERR("Failed to send UDP message: %d", error);
		 otMessageFree(message);
	 } else {
		 LOG_INF("UDP message sent: %s", udpPayload);
		 memset(udpPayload, 0, sizeof(udpPayload));  // Clear buffer after sending
	 }
 }
 
 void uart_cb(const struct device *dev, void *user_data) {
	 static int idx = 0;
	 uint8_t c;
 
	 while (uart_irq_update(dev) && uart_irq_rx_ready(dev)) {
		 uart_fifo_read(dev, &c, 1);
		 if (c == '\n' || idx >= sizeof(udpPayload) - 1) {
			 udpPayload[idx] = '\0';
			 LOG_INF("Received UART data: %s", udpPayload);
 
			 // Check if the message is "FAN"
			 if (strcmp(udpPayload, "FAN") == 0) {
				 LOG_INF("Triggering notification for FAN");
				 trigger_notification("Triggered");
			 } else {
				 udpSend();
			 }
			 idx = 0;
		 } else {
			 udpPayload[idx++] = c;
		 }
	 }
 }
 
 void main(void) {
	 LOG_INF("Thread UDP Sender + BLE starting...");
 
	 /* Initialize BLE (from ble.c/ble.h) */
	 ble_enable();
	 LOG_INF("BLE initialized and advertising...");
 
	 if (!device_is_ready(uart_dev)) {
		 LOG_ERR("UART device not ready");
		 return;
	 }
 
	 uart_irq_callback_user_data_set(uart_dev, uart_cb, NULL);
	 uart_irq_rx_enable(uart_dev);
 
	 instance = otInstanceInitSingle();
	 if (instance == NULL) {
		 LOG_ERR("Failed to initialize OpenThread instance");
		 return;
	 }
 
	 otError error = otIp6SetEnabled(instance, true);
	 if (error != OT_ERROR_NONE) {
		 LOG_ERR("Failed to enable IPv6: %d", error);
		 return;
	 }
 
	 error = otThreadSetEnabled(instance, true);
	 if (error != OT_ERROR_NONE) {
		 LOG_ERR("Failed to enable Thread: %d", error);
		 return;
	 }
 
	 memset(&udpSocket, 0, sizeof(udpSocket));
	 error = otUdpOpen(instance, &udpSocket, NULL, NULL);
	 if (error != OT_ERROR_NONE) {
		 LOG_ERR("Failed to open UDP socket: %d", error);
		 return;
	 }
 
	 LOG_INF("Waiting for UART data...");
	 while (1) {
		 k_sleep(K_MSEC(1000));
	 }
 }
 