/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

 #include <zephyr/kernel.h>
 #include <zephyr/bluetooth/bluetooth.h>
 #include <zephyr/bluetooth/gatt.h>
 #include <string.h>
 #include <stdio.h>
 #include "ble.h"
 
 /* Fallback in case CONFIG_BT_DEVICE_NAME isn't defined */
 #ifndef CONFIG_BT_DEVICE_NAME
 #define CONFIG_BT_DEVICE_NAME "Thread+BLE Device"
 #endif
 
 #define DEVICE_NAME CONFIG_BT_DEVICE_NAME
 #define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
 
 #define ADV_INT_MIN 0x01e0 /* 300 ms */
 #define ADV_INT_MAX 0x0260 /* 380 ms */
 
 /* Advertising Data */
 static const struct bt_data ad[] = {
	 BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	 BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
 };
 
 /* Connection callbacks */
 static void connected_cb(struct bt_conn *conn, uint8_t err)
 {
	 if (err) {
		 printk("Connection failed (err %u)\n", err);
	 } else {
		 printk("Central connected.\n");
	 }
 }
 
 static void disconnected_cb(struct bt_conn *conn, uint8_t reason)
 {
	 printk("Central disconnected (reason 0x%02x)\n", reason);
 }
 
 static struct bt_conn_cb conn_callbacks = {
	 .connected = connected_cb,
	 .disconnected = disconnected_cb,
 };
 
 /* Custom GATT Service and Characteristic UUIDs */
 static struct bt_uuid_128 custom_service_uuid = BT_UUID_INIT_128(
	 0x12, 0x34, 0x56, 0x78,
	 0x12, 0x34, 0x56, 0x78,
	 0x12, 0x34, 0x56, 0x78,
	 0x12, 0x34, 0x56, 0x78);
 
 static struct bt_uuid_128 custom_char_uuid = BT_UUID_INIT_128(
	 0xab, 0xcd, 0xef, 0x01,
	 0xab, 0xcd, 0xef, 0x01,
	 0xab, 0xcd, 0xef, 0x01,
	 0xab, 0xcd, 0xef, 0x01);
 
 /* Storage for the custom characteristic value */
 static uint8_t custom_char_value[20] = "Hello M5Stick!";
 
 /* Read callback for the custom characteristic */
 static ssize_t read_custom_char(struct bt_conn *conn,
				 const struct bt_gatt_attr *attr,
				 void *buf, uint16_t len, uint16_t offset)
 {
	 const char *value = attr->user_data;
	 return bt_gatt_attr_read(conn, attr, buf, len, offset, value, strlen(value));
 }
 
 /* Write callback for the custom characteristic */
 static ssize_t write_custom_char(struct bt_conn *conn,
				  const struct bt_gatt_attr *attr,
				  const void *buf, uint16_t len,
				  uint16_t offset, uint8_t flags)
 {
	 char *value = attr->user_data;
 
	 if (offset + len > sizeof(custom_char_value)) {
		 return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	 }
 
	 memcpy(value + offset, buf, len);
	 /* Add a null terminator if possible */
	 if (offset + len < sizeof(custom_char_value)) {
		 value[offset + len] = '\0';
	 }
 
	 printk("Custom Characteristic updated: %s\n", value);
	 return len;
 }
 
 /* Define the custom GATT service with one read/write/notify characteristic */
 BT_GATT_SERVICE_DEFINE(custom_service,
    BT_GATT_PRIMARY_SERVICE(&custom_service_uuid),
    BT_GATT_CHARACTERISTIC(&custom_char_uuid.uuid,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                           read_custom_char, write_custom_char, custom_char_value),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

 /*
  * trigger_notification() sends a notification with the provided message.
  * Adjust the index if your service structure is different.
  */
 void trigger_notification(const char *msg)
 {
	 /* 
	  * BT_GATT_SERVICE_DEFINE creates an array named 'custom_service' containing
	  * the attributes for the service. The characteristic value attribute is usually
	  * at index 2 (primary service is index 0, characteristic declaration index 1).
	  */
	 struct bt_gatt_attr *chrc_attr = (struct bt_gatt_attr *)&custom_service.attrs[2];
	 printk("trigger_notification() called with msg: %s\n", msg);
	 bt_gatt_notify(NULL, chrc_attr, msg, strlen(msg));
	 printk("bt_gatt_notify() complete.\n");
 }
 
 /* BLE initialization and advertising */
 void ble_enable(void)
 {
	 const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
		 BT_LE_ADV_OPT_CONNECTABLE,
		 ADV_INT_MIN,
		 ADV_INT_MAX,
		 NULL
	 );
 
	 int err = bt_enable(NULL);
	 if (err) {
		 printk("Bluetooth init failed (err %d)\n", err);
		 return;
	 }
 
	 bt_conn_cb_register(&conn_callbacks);
 
	 err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
	 if (err) {
		 printk("Advertising failed to start (err %d)\n", err);
		 return;
	 }
 
	 printk("BLE advertising started. Custom GATT service initialized.\n");
 }
 
//-----------------------Below original code(initial code) from the ble.c-------------------------------//

// /*
//  * Copyright (c) 2020 Nordic Semiconductor ASA
//  *
//  * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
//  */

// #include <zephyr/kernel.h>
// #include <zephyr/bluetooth/gatt.h>

// #include "ble.h"

// #define DEVICE_NAME CONFIG_BT_DEVICE_NAME
// #define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

// #define ADV_INT_MIN 0x01e0 /* 300 ms */
// #define ADV_INT_MAX 0x0260 /* 380 ms */

// static const struct bt_data ad[] = {
// 	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
// 	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
// };

// static struct bt_conn_cb conn_callbacks;

// void ble_enable(void)
// {
// 	const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
// 		BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_ONE_TIME, ADV_INT_MIN, ADV_INT_MAX, NULL);

// 	bt_enable(NULL);
// 	bt_conn_cb_register(&conn_callbacks);
// 	bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
// }
