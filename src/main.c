/*
 *
 * TCP HTTP Test
 *
 */

#include <zephyr.h>
#include <bsd.h>
#include <net/socket.h>
#include <modem/lte_lc.h>

#include <drivers/gpio.h>
#include <stdio.h>
#include <unistd.h>

#include "cJSON.h"

#define LED_PORT	DT_ALIAS_LED0_GPIOS_CONTROLLER
#define LED1 DT_GPIO_LEDS_LED0_GPIOS_PIN
#define LED2 DT_GPIO_LEDS_LED1_GPIOS_PIN
#define LED3 DT_GPIO_LEDS_LED2_GPIOS_PIN
#define LED4 DT_GPIO_LEDS_LED3_GPIOS_PIN

// curl 172.105.18.205:42501

#define HTTP_HEAD   \
    "GET / HTTP/1.1\r\n" \
    "Host: 172.105.18.205:42501\r\n" \
    "User-Agent: curl/7.64.1\r\n" \
    "Accept: */*\r\n\r\n"

static int server_socket;
static struct sockaddr_storage server;

LOG_MODULE_REGISTER(app, CONFIG_TEST1_LOG_LEVEL);

#if defined(CONFIG_BSD_LIBRARY)

/**@brief Recoverable BSD library error. */
void bsd_recoverable_error_handler(uint32_t err)
{
  printk("bsdlib recoverable error: %u\n", err);
}

/**@brief Irrecoverable BSD library error. */
void bsd_irrecoverable_error_handler(uint32_t err)
{
  printk("bsdlib irrecoverable error: %u\n", err);

  __ASSERT_NO_MSG(false);
}

#endif /* defined(CONFIG_BSD_LIBRARY) */

struct device *led_device;

static void init_led()
{

    led_device = device_get_binding(LED_PORT);

    /* Set LED pin as output */
    gpio_pin_configure(led_device, LED1, GPIO_OUTPUT);
    gpio_pin_configure(led_device, LED2, GPIO_OUTPUT);
    gpio_pin_configure(led_device, LED3, GPIO_OUTPUT);
    gpio_pin_configure(led_device, LED4, GPIO_OUTPUT);

}

static void led_on(char led)
{
    gpio_pin_set(led_device, led, 1);
}
static void led_off(char led)
{
    gpio_pin_set(led_device, led, 0);
}

static void led_on_off(char led, bool on_off)
{
    if (on_off)
    {
        led_on(led);
    } else {
        led_off(led);
    }
}

static void init_modem(void)
{
    int err;

    err = lte_lc_init_and_connect();
    __ASSERT(err == 0, "ERROR: LTE link init and connect %d\n", err);

    err = lte_lc_psm_req(false);
    __ASSERT(err == 0, "ERROR: psm %d\n", err);

    err = lte_lc_edrx_req(false);
    __ASSERT(err == 0, "ERROR: edrx %d\n", err);

}

static int tcp_ip_resolve(void)
{
    struct addrinfo *addrinfo;

    struct addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM};

    char ipv4_addr[NET_IPV4_ADDR_LEN];

    if (getaddrinfo(CONFIG_SERVER_HOST, NULL, &hints, &addrinfo) != 0)
    {
        LOG_ERR("ERROR: getaddrinfo failed\n");
        return -EIO;
    }

    if (addrinfo == NULL)
    {
        LOG_ERR("ERROR: Address not found\n");
        return -ENOENT;
    }

    struct sockaddr_in *server_ipv4 = ((struct sockaddr_in *)&server);

    server_ipv4->sin_addr.s_addr = ((struct sockaddr_in *)addrinfo->ai_addr)->sin_addr.s_addr;
    server_ipv4->sin_family = AF_INET;
    server_ipv4->sin_port = htons(CONFIG_SERVER_PORT);

    inet_ntop(AF_INET, &server_ipv4->sin_addr.s_addr, ipv4_addr, sizeof(ipv4_addr));
    LOG_INF("Server IPv4 Address %s\n", log_strdup(ipv4_addr));

    freeaddrinfo(addrinfo);

    return 0;
}

int connect_to_server()
{
    int err;

    server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0)
    {
        LOG_ERR("Failed to create CoAP socket: %d.\n", errno);
        return -errno;
    }

    err = connect(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
    if (err < 0)
    {
        LOG_ERR("Connect failed : %d\n", errno);
        return -errno;
    }

    return 0;
}

static void action_json_msg(char *msgbuf)
{

    bool found_json = false;

    char *start = "{";
    char *end = "}";
    int end_pos = (strlen(msgbuf) - 2);

    char *p_msgbuf = msgbuf;

    if (*p_msgbuf == *start)
    {
        p_msgbuf += end_pos;

        if (*p_msgbuf == *end)
        {
            found_json = true;
            for (int j = 0;j < 6;j++) {
                p_msgbuf++;
                *p_msgbuf = '\0';
            }
        }
        else
        {
            for (int j = 0;j < 1;j++) {
                LOG_INF("No JSON end found [%c]", *p_msgbuf);
                p_msgbuf++;
            }
        }
    }

    if (found_json == false)
    {
        LOG_ERR("ERROR: No JSON Data found [[%s]]\n",log_strdup(msgbuf));
        return;
    }

    cJSON *monitor_json = cJSON_Parse(msgbuf);
    if (monitor_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            LOG_ERR("ERROR: cJSON Parse : [%s]\n", log_strdup(error_ptr));
            return;
        }
    }

    cJSON *value_name = cJSON_GetObjectItemCaseSensitive(monitor_json, "ActionName");
    if (cJSON_IsString(value_name) && (value_name->valuestring != NULL))
    {
        if (strcmp((value_name->valuestring),"BSDTest") == 0) {
            cJSON *value_led1 = cJSON_GetObjectItemCaseSensitive(monitor_json, "LED1");
            if (cJSON_IsString(value_name) && (value_name->valuestring != NULL)) {
                led_on_off(LED3, value_led1->valueint);
            }
            cJSON *value_led2 = cJSON_GetObjectItemCaseSensitive(monitor_json, "LED2");
            if (cJSON_IsString(value_name) && (value_name->valuestring != NULL)) {
                led_on_off(LED4, value_led2->valueint);
            }
        }
    }

     cJSON_Delete(monitor_json);
}

int find_json_start(char *data)
{
    int pos = 0;
    char sub[] = "{\"ActionName\"";

    char *p1, *p2, *p3;
    int i=0,j=0;

    p1 = data;
    p2 = sub;

    for(i = 0; i<strlen(data); i++)
    {
        if(*p1 == *p2)
        {
            p3 = p1;
            for(j = 0;j<strlen(sub);j++)
            {
                if(*p3 == *p2)
                {
                    p3++;p2++;
                }
                else
                    break;
            }
            p2 = sub;
            if(j == strlen(sub))
            {
                pos = i+1;
                break;
            }
        }
        p1++;
    }
    return pos;
}

char *clean_string(char *string)
{
    int length = strlen(string);

    char *clean_string = malloc(length+1);
    char *np = clean_string, *op = string;

    memset(clean_string, '\0', length+1);
    do {
        if (!((*op <= ' ') || (*op > '~')))
        {
            *np++ = *op;
        }

    } while (*op++);

    return clean_string;
 }

char *sub_string(char *string, int position)
{
    char *p1;
    int c;
    int length = strlen(string);

    p1 = malloc(length+1);
    memset(p1, '\0', length+1);

    if (p1 == NULL)
    {
        LOG_ERR("ERROR: Unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for (c = 0 ; c < position -1 ; c++)
    {
        string++;
    }

    for (c = 0 ; c < length ; c++)
    {
        *(p1+c) = *string;
        string++;
    }

    *(p1+c) = '\0';

    return p1;
}

int send_tcp_msg() {
    char *p_buff;
    char *p_big_buff;
    int buf_len = 500;
    int big_buf_len = 5000;
    int big_buff_pos;
    int recsize = 0;
    int json_start;

    struct timeval timeout;

    timeout.tv_sec = 2;
    timeout.tv_usec = 500000;  // 2.5 seconds

    p_buff = malloc(buf_len);

    memset(p_buff, '\0', buf_len);
    strcpy(p_buff, HTTP_HEAD);

    setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof timeout);

    int ret = send(server_socket, p_buff, strlen(p_buff), 0);

    LOG_INF("Send packet data [%d] To %s:%d. ret=%d", sizeof(HTTP_HEAD), log_strdup(CONFIG_SERVER_HOST),
            CONFIG_SERVER_PORT, ret);

    big_buff_pos = 0;
    p_big_buff = malloc(big_buf_len);

    memset(p_big_buff, '\0', big_buf_len);
    for (;;) {
        memset(p_buff, '\0', buf_len);
        recsize = recv(server_socket, p_buff, sizeof p_buff, 0);
       // LOG_INF("Recv packet data [%d] buff [[%s]] ", recsize, log_strdup(p_buff));
        if (recsize <= 0) {
            break;
        }
        if (big_buff_pos == 0) {
            strcpy(p_big_buff, p_buff);
        } else {
            strcat(p_big_buff, p_buff);
        }
        memset(p_buff, '\0', buf_len);
        big_buff_pos += recsize;
    }

    //LOG_INF("Recv packet data [%d] [[%s]] ", big_buff_pos, log_strdup(p_big_buff));
    char *p_clean_big_buff = clean_string(p_big_buff);
    json_start = find_json_start(p_clean_big_buff);
    //LOG_INF("Recv packet data JSON start @ [%d] [[%s]] ", json_start, log_strdup(p_big_buff));
    if (json_start > 0) {
        char *json_data = sub_string(p_clean_big_buff, json_start);

        LOG_INF("Recv packet data JSON start @ [%d] [[%s]]", json_start, log_strdup(json_data));

        action_json_msg(json_data);
        free(json_data);

    } else
    {
        LOG_INF("No JSON Data in Recv data packet\n");
    }

    free(p_clean_big_buff);
    free(p_big_buff);
    free(p_buff);

    return 0;
}


void main(void)
{
    init_led();

    LOG_INF("BSD TCP HTTP Test V1.0");
    led_on(LED1);

    LOG_INF("Initializing Modem");
    init_modem();

    int err = tcp_ip_resolve();
    __ASSERT(err == 0, "ERROR: tcp_ip_resolve");


    for (;;) {
        led_off(LED2);
        led_off(LED3);
        led_off(LED4);

        LOG_INF("Connect to %s:%d", log_strdup(CONFIG_SERVER_HOST), CONFIG_SERVER_PORT);
        if (connect_to_server() == 0) {
            led_on(LED2);

            send_tcp_msg();

            close(server_socket);
        } else {
            close(server_socket);
        }
        k_sleep(100);
    }
}
