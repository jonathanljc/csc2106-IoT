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

static const char UDP_DEST_ADDR[] = "fd40:3477:6aa4:1:9da4:1bf2:475e:efe9";  // IPv6 Address of OTBR
// fd34:3efd:9d7:1:b089:a484:57bd:518f // IPv6 Address of OTBR Jon

static const uint16_t UDP_PORT = 1234;  // Choose a suitable UDP port

static char udpPayload[50];  // Payload to send over UDP

void generateRandomPayload(void)
{
    // float temp = 20.0 + (rand() % 1500) / 100.0;  // Random temperature between 20.0 and 35.0
    // float humid = 50.0 + (rand() % 5000) / 100.0; // Random humidity between 50.0 and 100.0
	// snprintf(udpPayload, sizeof(udpPayload), "Temp: 25, Humid: 50");

	// // TODO: make this randomise work (currently got error), but when have actual sensor, don't even need it
	// // snprintf(udpPayload, sizeof(udpPayload), "Temp: %.2f, Humid: %.2f", temp, humid);
	// Random temperature between 20.0 and 35.0

	float temp = 20.0f + (rand() % 1500) / 100.0f;
 
	// Random humidity between 50.0 and 100.0
	float humid = 50.0f + (rand() % 5000) / 100.0f;

	// Print them into udpPayload as floats with 2 decimal places
	// This may trigger a "float to double promotion" warning, which is safe to ignore
	snprintf(udpPayload, sizeof(udpPayload), "Temp: %.2f, Humid: %.2f", temp, humid);
}

static otInstance *instance;
static otUdpSocket udpSocket;

LOG_MODULE_REGISTER(cli_sample, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);

void udpSend(otInstance *instance)
{
    otError error;
    otMessage *message;
    otMessageInfo messageInfo;

	generateRandomPayload();

    // Create a new message
    message = otUdpNewMessage(instance, NULL);
    if (message == NULL)
    {
        LOG_ERR("Failed to allocate message");
        return;
    }

    // Append the payload to the message
    error = otMessageAppend(message, udpPayload, strlen(udpPayload));
    if (error != OT_ERROR_NONE)
    {
        LOG_ERR("Failed to append message: %d", error);
        otMessageFree(message);
        return;
    }

    // Set the message info
    memset(&messageInfo, 0, sizeof(messageInfo));
    otIp6AddressFromString(UDP_DEST_ADDR, &messageInfo.mPeerAddr);
    messageInfo.mPeerPort = UDP_PORT;

	// Send the message
	error = otUdpSend(instance, &udpSocket, message, &messageInfo);
	if (error != OT_ERROR_NONE)
	{
		LOG_ERR("Failed to send UDP message: %d", error);
		otMessageFree(message);
	}
}
	
int main(void)
	{
		// Initialize OpenThread instance
		instance = otInstanceInitSingle();
		if (instance == NULL)
		{
			LOG_ERR("Failed to initialize OpenThread instance");
			return -1;
		}
	
		// Bring up the IPv6 interface
		otError error = otIp6SetEnabled(instance, true);
		if (error != OT_ERROR_NONE)
		{
			LOG_ERR("Failed to bring up IPv6 interface: %d", error);
			return -1;
		}

		// Start the Thread network
		error = otThreadSetEnabled(instance, true);
		if (error != OT_ERROR_NONE)
		{
			LOG_ERR("Failed to start Thread network: %d", error);
			return -1;
		}

		// Initialize the UDP socket
		memset(&udpSocket, 0, sizeof(udpSocket));
		otUdpOpen(instance, &udpSocket, NULL, NULL);
	
		while (1)
		{
			udpSend(instance);
			k_sleep(K_SECONDS(2)); // Send a message every 2 seconds
		}
	
		return 0;
	}
