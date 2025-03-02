/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <openthread/platform/radio.h>
#include <openthread/thread.h>
#include <openthread/udp.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>

static const char UDP_DEST_ADDR[] = "fd97:b3f:1a22:1:7ef1:9f0f:c300:f9fc";  // IPv6 Address of OTBR

static const uint16_t UDP_PORT = 1234;  // Choose a suitable UDP port

#define UDP_PAYLOAD "Hello, OpenThread!"

static otInstance *instance;
static otUdpSocket udpSocket;

LOG_MODULE_REGISTER(cli_sample, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);

void udpSend(otInstance *instance)
{
    otError error;
    otMessage *message;
    otMessageInfo messageInfo;

    // Create a new message
    message = otUdpNewMessage(instance, NULL);
    if (message == NULL)
    {
        LOG_ERR("Failed to allocate message");
        return;
    }

    // Append the payload to the message
    error = otMessageAppend(message, UDP_PAYLOAD, sizeof(UDP_PAYLOAD));
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

#define WELLCOME_TEXT \
	"\n\r"\
	"\n\r"\
	"OpenThread Command Line Interface is now running.\n\r" \
	"Use the 'ot' keyword to invoke OpenThread commands e.g. " \
	"'ot thread start.'\n\r" \
	"For the full commands list refer to the OpenThread CLI " \
	"documentation at:\n\r" \
	"https://github.com/openthread/openthread/blob/master/src/cli/README.md\n\r"


	
	int main(void)
	{
		LOG_INF(WELLCOME_TEXT);
	
		// Initialize OpenThread instance
		instance = otInstanceInitSingle();
		if (instance == NULL)
		{
			LOG_ERR("Failed to initialize OpenThread instance");
			return -1;
		}
	
		// Initialize the UDP socket
		memset(&udpSocket, 0, sizeof(udpSocket));
		otUdpOpen(instance, &udpSocket, NULL, NULL);
	
		while (1)
		{
			udpSend(instance);
			k_sleep(K_SECONDS(2)); // Send a message every 10 seconds
		}
	
		return 0;
	}
