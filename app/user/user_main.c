/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "c_types.h"
#include "ip_addr.h"
#include "osapi.h"
#include "espconn.h"
#include "mem.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "web.h"

//设置AP SSID为esp8266_XXXXXX,X为MAC地址后三个地址
#define ESP_AP_SSID      "QT_%02x%02x%02x"
#define ESP_AP_PASSWORD  "12345678"


struct ringbuf_t
{
    uint8_t *buf;
    uint8_t *head, *tail;
    size_t size;
};
#define MAX_CON_CMD_SIZE     80
typedef struct ringbuf_t *ringbuf_t;
static ringbuf_t console_rx_buffer;
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


/*
*******************************************************
 ����ESP8266ΪAPģʽ��������IP��DHCP��������ַ��
��������
���أ���
*******************************************************
*/
bool wifi_softap_setup( void ){

    bool ap_init_result;   //��¼wifi_softap��ʼ�����

	ap_init_result = wifi_set_opmode(SOFTAP_MODE);

    ap_init_result &= wifi_softap_dhcps_stop();

    struct ip_info info;
    IP4_ADDR(&info.ip, 192, 168, 10, 254);  // set IP
    IP4_ADDR(&info.gw, 192, 168, 10, 254);    // set gateway
    IP4_ADDR(&info.netmask, 255, 255, 255, 0); // set netmask
    ap_init_result &= wifi_set_ip_info(SOFTAP_IF, &info);

    struct dhcps_lease esp_dhcps_lease;
    IP4_ADDR(&esp_dhcps_lease.start_ip,192,168,10,1);
    IP4_ADDR(&esp_dhcps_lease.end_ip,192,168,10,2);
    ap_init_result &= wifi_softap_set_dhcps_lease(&esp_dhcps_lease);

    ap_init_result &= wifi_softap_dhcps_start();


    char softap_mac[6];
    wifi_get_macaddr(STATION_IF, softap_mac);

    struct softap_config *config = (struct softap_config *)os_zalloc(sizeof(struct softap_config ));
    wifi_softap_get_config(config); // Get soft-AP config first.
    os_sprintf(config->ssid, ESP_AP_SSID,softap_mac[3],softap_mac[4],softap_mac[5]);
    os_sprintf(config->password, ESP_AP_PASSWORD);
    config->authmode = AUTH_WPA2_PSK;
    config->ssid_len = 0;        // or its actual SSID length
    config->max_connection = 2;

    ap_init_result &= wifi_softap_set_config(config); // Set ESP8266 soft-AP config

    os_free(config);


    if(ap_init_result)
    	os_printf("softap setup success!\n");
    else
    	os_printf("softap setup fail!\n");

    return ap_init_result;
}

int ICACHE_FLASH_ATTR parse_str_into_tokens(char *str, char **tokens, int max_tokens)
{
char    *p, *q, *end;
int     token_count = 0;
bool    in_token = false;

   // preprocessing
   for (p = q = str; *p != 0; p++) {
	if (*(p) == '%' && *(p+1) != 0 && *(p+2) != 0) {
	   // quoted hex
		uint8_t a;
		p++;
		if (*p <= '9')
		    a = *p - '0';
		else
		    a = toupper(*p) - 'A' + 10;
		a <<= 4;
		p++;
		if (*p <= '9')
		    a += *p - '0';
		else
		    a += toupper(*p) - 'A' + 10;
		*q++ = a;
	} else if (*p == '\\' && *(p+1) != 0) {
	   // next char is quoted - just copy it, skip this one
	   *q++ = *++p;
	} else if (*p == 8) {
	   // backspace - delete previous char
	   if (q != str) q--;
	} else if (*p <= ' ') {
	   // mark this as whitespace
	   *q++ = 0;
	} else {
	   *q++ = *p;
	}
   }

   end = q;
   *q = 0;

   // cut into tokens
   for (p = str; p != end; p++) {
	if (*p == 0) {
	   if (in_token) {
		in_token = false;
	   }
	} else {
	   if (!in_token) {
		tokens[token_count++] = p;
		if (token_count == max_tokens)
		   return token_count;
		in_token = true;
	   }
	}
   }
   return token_count;
}


void ICACHE_FLASH_ATTR console_handle_command(struct espconn *pespconn)
{


}


static void ICACHE_FLASH_ATTR handle_set_cmd(void *arg, char *cmd, char* val)
{
    struct espconn *pespconn = (struct espconn *)arg;
    int max_current_cmd_size = MAX_CON_CMD_SIZE - (os_strlen(cmd)+1);
    char cmd_line[MAX_CON_CMD_SIZE+1];

    if (os_strlen(val) >= max_current_cmd_size)
    {
        val[max_current_cmd_size]='\0';
    }
    os_sprintf(cmd_line, "%s %s", cmd, val);
    os_printf("web_client_recv_cb(): cmd line:%s\n",cmd_line);

    ringbuf_memcpy_into(console_rx_buffer, cmd_line, os_strlen(cmd_line));
    console_handle_command(pespconn);
}

char *strstr(const char *string, const char *needle);
char *strtok ( char * str, const char * delimiters );
char *strtok_r(char *s, const char *delim, char **last);

static void ICACHE_FLASH_ATTR web_client_recv_cb(void *arg, char *data, unsigned short length)
{
    struct espconn *pespconn = (struct espconn *)arg;
    char *kv, *sv;
    bool do_reset = false;
    char *token[1];
    char *str;

    str = strstr(data, " /?");
    if (str != NULL)
    {
        str = strtok(str+3," ");

        char* keyval = strtok_r(str,"&",&kv);
        while (keyval != NULL)
        {
            char *key = strtok_r(keyval,"=", &sv);
            char *val = strtok_r(NULL, "=", &sv);

            keyval = strtok_r (NULL, "&", &kv);
            os_printf("web_client_recv_cb(): key:%s:val:%s:\n",key,val);
            if (val != NULL)
            {

                if (strcmp(key, "ssid") == 0)
                {
                	parse_str_into_tokens(val, token, 1);
                	handle_set_cmd(pespconn, "set ssid", token[0]);
                	//config.automesh_mode = AUTOMESH_OFF;
                    do_reset = true;
                }
                else if (strcmp(key, "password") == 0)
                {
                	parse_str_into_tokens(val, token, 1);
                    handle_set_cmd(pespconn, "set password", token[0]);
                    do_reset = true;
                }
                else if (strcmp(key, "am") == 0)
                {
                    //config.automesh_mode = AUTOMESH_LEARNING;
                    //config.automesh_checked = 0;
                    do_reset = true;
                }
                else if (strcmp(key, "lock") == 0)
                {
                	//os_memcpy(config.lock_password, config.password, sizeof(config.lock_password));
                    //config.locked = 1;
                }
                else if (strcmp(key, "ap_ssid") == 0)
                {
                	parse_str_into_tokens(val, token, 1);
                    handle_set_cmd(pespconn, "set ap_ssid", token[0]);
                    do_reset = true;
                }
                else if (strcmp(key, "ap_password") == 0)
                {
                	parse_str_into_tokens(val, token, 1);
                    handle_set_cmd(pespconn, "set ap_password", token[0]);
                    do_reset = true;
                }
                else if (strcmp(key, "network") == 0)
                {
                    handle_set_cmd(pespconn, "set network", val);
                    do_reset = true;
                }
                else if (strcmp(key, "unlock_password") == 0)
                {
                    handle_set_cmd(pespconn, "unlock", val);
                }
                else if (strcmp(key, "ap_open") == 0)
                {
                    if (strcmp(val, "wpa2") == 0)
                    {
                        //config.ap_open = 0;
                        do_reset = true;
                    }
                    if (strcmp(val, "open") == 0)
                    {
                        //config.ap_open = 1;
                        do_reset = true;
                    }                }
                else if (strcmp(key, "reset") == 0)
                {
                    do_reset = true;
                }
            }
        }

        //config_save(&config);

        if (do_reset == true)
        {
            do_reset = false;
            ringbuf_memcpy_into(console_rx_buffer, "reset", os_strlen("reset"));
            console_handle_command(pespconn);
        }
    }
}

static void ICACHE_FLASH_ATTR web_client_discon_cb(void *arg)
{
    os_printf("web_client_discon_cb(): client disconnected\n");
    struct espconn *pespconn = (struct espconn *)arg;
}

static void ICACHE_FLASH_ATTR web_client_sent_cb(void *arg)
{
    os_printf("web_client_discon_cb(): data sent to client\n");
    struct espconn *pespconn = (struct espconn *)arg;

    espconn_disconnect(pespconn);
}


/* Called when a client connects to the web config */
static void ICACHE_FLASH_ATTR web_client_connected_cb(void *arg)
{

    struct espconn *pespconn = (struct espconn *)arg;

    os_printf("web_client_connected_cb(): Client connected\r\n");

    /*
    if (!check_connection_access(pespconn, config.config_access)) {
	os_printf("Client disconnected - no config access on this network\r\n");
	espconn_disconnect(pespconn);
	return;
    }
    */

    espconn_regist_disconcb(pespconn,   web_client_discon_cb);
    espconn_regist_recvcb(pespconn,     web_client_recv_cb);
    espconn_regist_sentcb(pespconn,     web_client_sent_cb);
os_printf("espconn_regist OK.\r\n");
    //ringbuf_reset(console_rx_buffer);
    //ringbuf_reset(console_tx_buffer);

  	static const uint8_t index_page_str[] ICACHE_RODATA_ATTR STORE_ATTR = INDEX_PAGE;
os_printf("get index page.\r\n");
  	uint32_t slen = (sizeof(index_page_str) + 4) & ~3;
	uint8_t *index_page = (char *)os_malloc(slen);
	if (index_page == NULL)
	    return;
os_printf("malloc page addr.\r\n");
	os_memcpy(index_page, index_page_str, slen);

	uint8_t *page_buf = (char *)os_malloc(slen+200);
	if (page_buf == NULL)
	    return;
os_printf("malloc buff.\r\n");
/*
os_sprintf(page_buf, index_page, ESP_AP_SSID, ESP_AP_PASSWORD,
		   "",
		   ESP_AP_SSID, ESP_AP_PASSWORD,
		   ""," selected",
		   192,168,10,254);
*/
	os_sprintf(page_buf, index_page);

	os_free(index_page);

os_printf("load web page OK.\r\n");

	espconn_send(pespconn, page_buf, os_strlen(page_buf));

	os_free(page_buf);
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
    //system_phy_freq_trace_enable(at_get_rf_auto_trace_from_flash());
}

void ICACHE_FLASH_ATTR user_init(void)
{
	struct espconn *pCon;


	os_printf("init web!\r\n");
	//uart_init();
	uart0_sendStr("uart0_sendStr\r\n");


	uart_init(BIT_RATE_74880, BIT_RATE_74880);


    for(;;)
    {
    	if( wifi_softap_setup() )     //����ESP8266Ϊapģʽ������ap��IP������DHCP��ַ��Χ
    		break;
    	else
    		os_delay_us(5000);
    }

    uart_tx_one_char(UART0,0x0d);   //��������з�(0x0D,0x0A)�������CLIENT�����ӵ�SLIP��������
    uart_tx_one_char(UART0,0x0a);
    uart0_sendStr("CLIENT");

    pCon = (struct espconn *)os_zalloc(sizeof(struct espconn));
    os_printf("Starting Web Config Server on port 80\r\n");


    /* Equivalent to bind */
    pCon->type  = ESPCONN_TCP;
    pCon->state = ESPCONN_NONE;
    pCon->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    pCon->proto.tcp->local_port = 80;

    /* Register callback when clients connect to the server */
    espconn_regist_connectcb(pCon, web_client_connected_cb);

    /* Put the connection in accept mode */
    espconn_accept(pCon);

}
