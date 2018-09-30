#include "web_server.h"
#include "web.h"


static ringbuf_t console_rx_buffer;

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

//char *strstr(const char *string, const char *needle);
//char *strtok ( char * str, const char * delimiters );
//char *strtok_r(char *s, const char *delim, char **last);

/********************************************************
 *
 * 根据客户端浏览器请求生成网页并发送
 *
 *********************************************************/
void ICACHE_FLASH_ATTR read_sent_page(void *arg)
{
	struct espconn *pespconn = (struct espconn *)arg;

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




static void ICACHE_FLASH_ATTR web_client_recv_cb(void *arg, char *data, unsigned short length)
{
    struct espconn *pespconn = (struct espconn *)arg;
    char *kv, *sv;
    bool do_reset = false;
    char *token[1];
    char *str;
    os_printf("recv form host:%s\n",data);
    read_sent_page(pespconn);
    //recv form host:GET / HTTP/1.1
    //Host: 192.168.10.254
    //recv form host:GET /favicon.ico HTTP/1.1

    str = strstr(data, "area1");
    //os_printf("recv data=%s\n",str);
    if (str != NULL)
    {
        //str = strtok(str+3," ");

        char* keyval = strtok_r(str,"&",&kv);
        while (keyval != NULL)
        {
            char *key = strtok_r(keyval,"=", &sv);
            char *val = strtok_r(NULL, "=", &sv);

            keyval = strtok_r (NULL, "&", &kv);
            os_printf("web_client_recv_cb(): key:%s:val:%s:\n",key,val);
            if (val != NULL)
            {

                if (strcmp(key, "area1") == 0)
                {
                    do_reset = true;
                }
                else if (strcmp(key, "area2") == 0)
                {
                    do_reset = true;
                }
                else if (strcmp(key, "am") == 0)
                {
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
            //ringbuf_memcpy_into(console_rx_buffer, "reset", os_strlen("reset"));
            //console_handle_command(pespconn);
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
}

void ICACHE_FLASH_ATTR init_webserver()
{
	struct espconn *pCon;

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
