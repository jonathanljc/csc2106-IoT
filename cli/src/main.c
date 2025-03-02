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


LOG_MODULE_REGISTER(cli_sample, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);

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

	return 0;
}
