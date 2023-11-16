/**
 * @file uart_handler.c
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
#include "sys_arch.h"
#include "bsp_uart.h"

#include "uart_handler.h"
#include "zigbee_app.h"
//=============================================================================
//                Private Definitions of const value
//=============================================================================
#define UART_HANDLER_RX_CACHE_SIZE          16
#define MAX_UART_BUFFER_SIZE                256
//=============================================================================
//                Private ENUM
//=============================================================================

//=============================================================================
//                Private Struct
//=============================================================================
typedef struct
{
    uint8_t *pdata;
    uint32_t len;
} uart_msg_t;
//=============================================================================
//                Private Function Declaration
//=============================================================================

//=============================================================================
//                Private Global Variables
//=============================================================================
static uart_handler_parm_t uart_parm;
static uint8_t uart_buf[MAX_UART_BUFFER_SIZE] = { 0 };
static sys_queue_t uart_msg_q;
//=============================================================================
//                Public Global Variables
//=============================================================================

//=============================================================================
//                Private Definition of Compare/Operation/Inline function/
//=============================================================================

//=============================================================================
//                Functions
//=============================================================================
static void _uart_handler_send(void)
{
    uart_msg_t uart_msg;
    if (sys_arch_queue_tryrecv(&uart_msg_q, &uart_msg) != SYS_ARCH_TIMEOUT)
    {
        bsp_uart_send(uart_msg.pdata, uart_msg.len);
        if (uart_msg.pdata)
        {
            sys_free(uart_msg.pdata);
        }
    }
}
static void _uart_handler_recv(void)
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
        len = bsp_uart_recv(rx_buf, sizeof(rx_buf));
        if (len)
        {
            memcpy(uart_buf + total_len, rx_buf, len);
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
                    sys_tlv_t *pt_tlv = sys_malloc(sizeof(sys_tlv_t) + msgbufflen);

                    if (!pt_tlv)
                    {
                        break;
                    }
                    memcpy(pt_tlv->value, uart_buf + offset, msgbufflen);
                    pt_tlv->length = msgbufflen;
                    uart_parm.UartRecvCB[i](pt_tlv);
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
                        uint8_t *remainingdata = sys_malloc(total_len);
                        if (!remainingdata)
                        {
                            break;
                        }
                        memcpy(remainingdata, uart_buf + offset, total_len);
                        memcpy(uart_buf, remainingdata, total_len);

                        if (remainingdata)
                        {
                            sys_free(remainingdata);
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

static void _uart_handler_task(void *arg)
{
    for (;;)
    {
        _uart_handler_send();
        _uart_handler_recv();
        sys_task_delay(2);
    }
}

void uart_handler_init(uart_handler_parm_t *param)
{
    sys_task_new("uart", _uart_handler_task, NULL, 256, TASK_PRIORITY_APP);
    sys_queue_new(&uart_msg_q, 4, sizeof(uart_msg_t));
    memcpy(&uart_parm, param, sizeof(uart_handler_parm_t));
}

void uart_handler_send(uint8_t *pdata, uint32_t len)
{
    uart_msg_t uart_msg;

    uart_msg.pdata = pdata;
    uart_msg.len = len;

    sys_queue_send_with_timeout(&uart_msg_q, &uart_msg, 0);
}
