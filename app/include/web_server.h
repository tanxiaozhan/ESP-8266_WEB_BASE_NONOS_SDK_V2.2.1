/*
 * web_server.h
 *
 *  Created on: 2018Äê9ÔÂ28ÈÕ
 *      Author: txz
 */

#ifndef APP_INCLUDE_WEB_SERVER_H_
#define APP_INCLUDE_WEB_SERVER_H_


#include "c_types.h"
#include "ip_addr.h"
#include "osapi.h"
#include "mem.h"
#include "espconn.h"


#define MAX_CON_CMD_SIZE     80

struct ringbuf_t
{
    uint8_t *buf;
    uint8_t *head, *tail;
    size_t size;
};

typedef struct ringbuf_t *ringbuf_t;


#endif /* APP_INCLUDE_WEB_SERVER_H_ */
