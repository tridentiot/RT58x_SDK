/**
 * @file app_uart_handler.c
 * @author Rex Huang (rex.huang@rafaelmicro.com)
 * @brief
 * @version 0.1
 * @date 2022-03-24
 *
 * @copyright Copyright (c) 2022
 *
 */



//=============================================================================
//                Include
//=============================================================================
#include "util_queue.h"
#include "mem_mgmt.h"
#include "hosal_uart.h"
#include "app_uart_handler.h"
#include "ota_download_cmd_handler.h"
//=============================================================================
//                Private Definitions of const value
//=============================================================================
#define UART_HANDLER_RX_CACHE_SIZE          128
#define RX_BUFF_SIZE                        484
#define MAX_UART_BUFFER_SIZE                256
//=============================================================================
//                Private ENUM
//=============================================================================

//=============================================================================
//                Private Struct
//=============================================================================
typedef struct uart_io
{
    uint16_t start;
    uint16_t end;

    uint32_t recvLen;
    uint8_t uart_cache[RX_BUFF_SIZE];
} uart_io_t;
typedef enum
{
    UART1_EVENT_NONE                       = 0,

    UART1_EVENT_UART_IN                    = 0x00000001,
    UAR1T_EVENT_UART_TX_DOWN               = 0x00000002,

    UART1_EVENT_ALL                        = 0xffffffff,
} uart1_event_t;
//=============================================================================
//                Private Function Declaration
//=============================================================================

//=============================================================================
//                Private Global Variables
//=============================================================================
static uart1_event_t g_uart1_evt_var;
static uart_io_t g_uart_rx_io = { .start = 0, .end = 0, };
HOSAL_UART_DEV_DECL(uart1_dev, 1, 28, 29, UART_BAUDRATE_Baud115200)

static uart_handler_parm_t uart_parm;
static uint8_t uart_buf[MAX_UART_BUFFER_SIZE] = { 0 };
//=============================================================================
//                Public Global Variables
//=============================================================================

//=============================================================================
//                Private Definition of Compare/Operation/Inline function/
//=============================================================================

//=============================================================================
//                Functions
//=============================================================================
static int __uart_rx_callback(void *p_arg)
{
    uint32_t len = 0;
    if (g_uart_rx_io.start >= g_uart_rx_io.end)
    {
        g_uart_rx_io.start += hosal_uart_receive(p_arg, g_uart_rx_io.uart_cache + g_uart_rx_io.start,
                              RX_BUFF_SIZE - g_uart_rx_io.start - 1);
        if (g_uart_rx_io.start == (RX_BUFF_SIZE - 1))
        {
            g_uart_rx_io.start = hosal_uart_receive(p_arg, g_uart_rx_io.uart_cache,
                                                    (RX_BUFF_SIZE + g_uart_rx_io.end - 1) % RX_BUFF_SIZE);
        }
    }
    else if (((g_uart_rx_io.start + 1) % RX_BUFF_SIZE) != g_uart_rx_io.end)
    {
        g_uart_rx_io.start += hosal_uart_receive(p_arg, g_uart_rx_io.uart_cache,
                              g_uart_rx_io.end - g_uart_rx_io.start - 1);
    }

    if (g_uart_rx_io.start != g_uart_rx_io.end)
    {

        len = (g_uart_rx_io.start + RX_BUFF_SIZE - g_uart_rx_io.end) % RX_BUFF_SIZE;
        if (g_uart_rx_io.recvLen != len)
        {
            g_uart_rx_io.recvLen = len;
            //            UART1_NOTIFY_ISR(UART1_EVENT_UART_IN);
        }
    }

    return 0;
}

static int __uart_read(uint8_t *p_data)
{
    uint32_t byte_cnt = 0;
    hosal_uart_ioctl(&uart1_dev, HOSAL_UART_DISABLE_INTERRUPT, (void *)NULL);

    if (g_uart_rx_io.start != g_uart_rx_io.end)
    {
        if (g_uart_rx_io.start > g_uart_rx_io.end)
        {
            mem_memcpy(p_data, g_uart_rx_io.uart_cache + g_uart_rx_io.end, g_uart_rx_io.start - g_uart_rx_io.end);
            byte_cnt = g_uart_rx_io.start - g_uart_rx_io.end;
            g_uart_rx_io.end = g_uart_rx_io.start;
        }
        else
        {
            mem_memcpy(p_data, g_uart_rx_io.uart_cache + g_uart_rx_io.end, RX_BUFF_SIZE - g_uart_rx_io.end);
            byte_cnt = RX_BUFF_SIZE - g_uart_rx_io.end;
            g_uart_rx_io.end = RX_BUFF_SIZE - 1;

            if (g_uart_rx_io.start)
            {
                mem_memcpy(&p_data[byte_cnt], g_uart_rx_io.uart_cache, g_uart_rx_io.start);
                byte_cnt += g_uart_rx_io.start;
                g_uart_rx_io.end = (RX_BUFF_SIZE + g_uart_rx_io.start - 1) % RX_BUFF_SIZE;
            }
        }
    }

    g_uart_rx_io.start = g_uart_rx_io.end = 0;
    g_uart_rx_io.recvLen = 0;
    hosal_uart_ioctl(&uart1_dev, HOSAL_UART_ENABLE_INTERRUPT, (void *)NULL);
    return byte_cnt;
}

void app_uart_handler_recv()
{
    /*  */
    static uint16_t total_len = 0;
    static uint16_t offset = 0;
    uint8_t rx_buf[UART_HANDLER_RX_CACHE_SIZE] = { 0 };
    int len;

    uint16_t msgbufflen = 0;
    uint32_t parser_status = 0;
    int i = 0;
    /*  */
    do
    {
        if (total_len > MAX_UART_BUFFER_SIZE)
        {
            total_len = 0;
        }
        len = __uart_read(rx_buf);

        if (len)
        {
            mem_memcpy(uart_buf + total_len, rx_buf, len);
            total_len += len;
            for (i = 0; i < UART_HANDLER_PARSER_CB_NUM; i++)
            {
                if (uart_parm.UartParserCB[i] == NULL)
                {
                    continue;
                }
                parser_status = uart_parm.UartParserCB[i](uart_buf, total_len, &msgbufflen, &offset);
                if ((parser_status == UART_DATA_VALID) || (parser_status == UART_DATA_VALID_CRC_OK))
                {
                    if (uart_parm.UartRecvCB[i] == NULL)
                    {
                        break;
                    }
                    uint8_t *buf = mem_malloc(msgbufflen);
                    if (NULL == buf)
                    {
                        break;
                    }
                    mem_memcpy(buf, uart_buf + offset, msgbufflen);
                    uart_parm.UartRecvCB[i](buf, msgbufflen);

                    if (parser_status == UART_DATA_VALID_CRC_OK)
                    {
                        total_len -= (msgbufflen + offset) ;
                    }
                    else
                    {
                        total_len -= msgbufflen;
                    }


                    if (total_len > 0)
                    {
                        uint8_t *remainingdata = mem_malloc(total_len);
                        if (!remainingdata)
                        {
                            break;
                        }
                        mem_memcpy(remainingdata, uart_buf + offset, total_len);
                        mem_memcpy(uart_buf, remainingdata, total_len);

                        if (remainingdata)
                        {
                            mem_free(remainingdata);
                        }
                        break;
                    }
                    break;
                }
                else if (parser_status == UART_DATA_CS_ERROR)
                {
                    total_len = 0;
                    //msgbufflen = 0;
                }
                else
                {
                    //offset = 0;
                    msgbufflen = 0;
                }
            }
        }

    } while (0);
}

void app_uart_msg_recv(uint8_t *data, uint16_t lens)
{
    ota_download_cmd_proc(data, lens);
    if (data)
    {
        mem_free(data);
    }
    return;
}

void app_uart_handler_init()
{
    uart_handler_parm_t uart_handler_param = {0};

    hosal_uart_init(&uart1_dev);

    hosal_uart_callback_set(&uart1_dev, HOSAL_UART_RX_CALLBACK, __uart_rx_callback, &uart1_dev);
    /* Configure UART to interrupt mode */
    hosal_uart_ioctl(&uart1_dev, HOSAL_UART_MODE_SET, (void *)HOSAL_UART_MODE_INT_RX);

    uart_handler_param.UartParserCB[0] = ota_download_cmd_parser;
    uart_handler_param.UartRecvCB[0] = app_uart_msg_recv;
    mem_memcpy(&uart_parm, &uart_handler_param, sizeof(uart_handler_parm_t));
}

void app_uart_handler_send(uint8_t *pdata, uint32_t len)
{
    hosal_uart_send(&uart1_dev, pdata, len);
}
