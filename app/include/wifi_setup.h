/*
 * wifi_setup.h
 *
 *  Created on: 2018��9��28��
 *      Author: admin
 */

#ifndef APP_INCLUDE_WIFI_SETUP_H_
#define APP_INCLUDE_WIFI_SETUP_H_

#include "c_types.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "ip_addr.h"

//����AP SSIDΪesp8266_XXXXXX,XΪMAC��ַ��������ַ
#define ESP_AP_SSID      "QT_%02x%02x%02x"
#define ESP_AP_PASSWORD  "12345678"

bool wifi_softap_setup( void );

#endif /* APP_INCLUDE_WIFI_SETUP_H_ */
