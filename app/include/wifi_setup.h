/*
 * wifi_setup.h
 *
 *  Created on: 2018年9月28日
 *      Author: admin
 */

#ifndef APP_INCLUDE_WIFI_SETUP_H_
#define APP_INCLUDE_WIFI_SETUP_H_

#include "c_types.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "ip_addr.h"

//设置AP SSID为esp8266_XXXXXX,X为MAC地址后三个地址
#define ESP_AP_SSID      "QT_%02x%02x%02x"
#define ESP_AP_PASSWORD  "12345678"

bool wifi_softap_setup( void );

#endif /* APP_INCLUDE_WIFI_SETUP_H_ */
