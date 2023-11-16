/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file
 *
 * @author Rex
 * @version 0.1
 * @date 2021/12/09
 * @license
 * @description
 */

//=============================================================================
//                Include
//=============================================================================
#include "sys_arch.h"
#include "bsp_uart.h"
//#include "uart_handler.h"
#include "zigbee_app.h"
#include "fsm.h"
#include "util_fsm_assist.h"

#define SOH                     ((uint8_t)0x01)  /* start of 128-byte data packet */
#define STX                     ((uint8_t)0x02)  /* start of 1024-byte data packet */
#define EOT                     ((uint8_t)0x04)  /* end of transmission */
#define ACK                     ((uint8_t)0x06)  /* acknowledge */
#define NAK                     ((uint8_t)0x15)  /* negative acknowledge */
#define CAN                     ((uint32_t)0x18) /* two of these in succession aborts transfer */
#define CRC16                   ((uint8_t)0x43)  /* 'C' == 0x43, request 16-bit CRC */

#define ABORT1                  ((uint8_t)0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  ((uint8_t)0x61)  /* 'a' == 0x61, abort by user */


typedef struct
{
    uint8_t tag;
    uint8_t num;
    uint8_t n_num;
} xmodem_hdr;

typedef struct
{
    uint8_t cs;
} xmodem_end;

typedef void (* xmodem_fsm_action_t)(void *);

#define EVENT_LIST  E_XMODEM_START,         \
                    E_XMODEM_RECV_DATA,     \
                    E_XMODEM_RETRY,         \
                    E_XMODEM_TIMEOUT,       \
                    E_XMODEM_FINISH


#define STATE_LIST  S_IDLE,                 \
                    S_XMODEM_WAIT_DATA,     \
                    S_XMODEM_RECV_DATA


#define ACTION_LIST A_XMODEM_SEND_NAK,      a_send_nak,     \
                    A_XMODEM_SEND_ACK,      a_send_ack

static void a_send_nak(void *p_data);
static void a_send_ack(void *p_data);

typedef enum
{
    DECLARE_ENUM(EVENT_LIST)
} bar_tx_event_ids_t;

typedef enum
{
    DECLARE_ENUM(STATE_LIST)
} bar_tx_state_ids_t;

typedef enum
{
    DECLARE_ENUM_PAIR(ACTION_LIST)
} bar_tx_action_ids_t;

static const xmodem_fsm_action_t xmodem_fsm_actions[] =
{
    DECLARE_HANDLER(ACTION_LIST)
};

static const fsm_transition_t xmodem_fsm_transition_table[] =
{
    FSM_STATE(S_IDLE),
    FSM_TRANSITION(E_XMODEM_START,      FSM_NO_GUARD,   A_XMODEM_SEND_NAK,  S_XMODEM_WAIT_DATA),

    FSM_STATE(S_XMODEM_WAIT_DATA),
    FSM_TRANSITION(E_XMODEM_TIMEOUT,    FSM_NO_GUARD,   A_XMODEM_SEND_NAK,  S_XMODEM_WAIT_DATA),
    FSM_TRANSITION(E_XMODEM_RECV_DATA,  FSM_NO_GUARD,   A_XMODEM_SEND_ACK,  S_XMODEM_WAIT_DATA),
    FSM_TRANSITION(E_XMODEM_RETRY,      FSM_NO_GUARD,   A_XMODEM_SEND_NAK,  S_XMODEM_WAIT_DATA),
    FSM_TRANSITION(E_XMODEM_FINISH,     FSM_NO_GUARD,   A_XMODEM_SEND_ACK,  S_IDLE),
};

static void bar_xmodem_action(fsm_action_id_t action_id, void *p_data);

static const fsm_const_descriptor_t xmodem_fsm_descriptor =
{
    .transition_table = xmodem_fsm_transition_table,
    .transitions_count = sizeof(xmodem_fsm_transition_table) / sizeof(xmodem_fsm_transition_table[0]),
    .initial_state = S_IDLE,
    .guard = NULL,
    .action = bar_xmodem_action,
};

static TimerHandle_t tmr_xmodem;
static fsm_t xmodem_fsm;
static uint32_t retry_cnt = 0;

static uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size)
{
    uint32_t sum = 0;
    const uint8_t *p_data_end = p_data + size;

    while (p_data < p_data_end )
    {
        sum += *p_data++;
    }

    return (sum % 0x100u);
}
static void bar_xmodem_action(fsm_action_id_t action_id, void *p_data)
{
    xmodem_fsm_actions[action_id](p_data);
}

static void a_send_nak(void *p_data)
{
    bsp_uart_stdout_char(NAK);
    xTimerStart(tmr_xmodem, 0);
}

static void a_send_ack(void *p_data)
{
    bsp_uart_stdout_char(ACK);
    xTimerStart(tmr_xmodem, 0);
}

static void tmr_xmodem_cb(TimerHandle_t t_timer)
{
    retry_cnt++;
    if (retry_cnt > 0xFFFF)
    {
        fsm_event_post(&xmodem_fsm, E_XMODEM_FINISH, NULL);
    }
    else
    {
        fsm_event_post(&xmodem_fsm, E_XMODEM_TIMEOUT, NULL);
    }
}

void xmodem_init(void)
{
    do
    {
        retry_cnt = 0;
        fsm_init(&xmodem_fsm, &xmodem_fsm_descriptor);
        tmr_xmodem = xTimerCreate("xmodem", pdMS_TO_TICKS(40), pdFALSE, (void *)0, tmr_xmodem_cb);
        fsm_event_post(&xmodem_fsm, E_XMODEM_START, NULL);
    } while (0);

}
void xmodem_recv(sys_tlv_t *pt_tlv)
{
    uint8_t *p_xmodem_data = pt_tlv->value;
    uint8_t cs = 0;

    if (xTimerIsTimerActive(tmr_xmodem))
    {
        xTimerStop(tmr_xmodem, 0 );
    }

    if (p_xmodem_data[0] == SOH)
    {
        cs = CalcChecksum(&p_xmodem_data[3], 128);

        if (cs == p_xmodem_data[131])
        {
            fsm_event_post(&xmodem_fsm, E_XMODEM_RECV_DATA, NULL);
        }
        else
        {
            //err(">> %s: checksum error:0x%x, correct:0x%x\n", __FUNCTION__, p_xmodem_data[131], cs);
            //util_log_mem(UTIL_LOG_INFO, "  ", (uint8_t *)p_xmodem_data, 132, 0);
            fsm_event_post(&xmodem_fsm, E_XMODEM_RETRY, NULL);
        }
    }
    else if (p_xmodem_data[0] == EOT)
    {
        fsm_event_post(&xmodem_fsm, E_XMODEM_FINISH, NULL);
    }
    sys_free(pt_tlv);
}
zb_cmd_handler_data_sts_t xmodem_parser(uint8_t *pBuf, uint16_t plen, uint16_t *datalen, uint16_t *offset)
{
    /* +-------- Packet in IAP memory -----+-----+---------+-----------+
     * | 0      |  1     |  2    |  3      | ... | n+4     |     n+5   |
     * |--------+--------+-------+---------+-----+---------+-----------+
     * |  start | number | !num  | data[0] | ... | data[n] | checksum  |
     * +--------+--------+-------+---------+-----+---------+-----------+
     * the first byte is left unused for memory alignment reasons                 */

    zb_cmd_handler_data_sts_t t_return = ZB_DATA_INVALID;

    uint16_t i = 0;
    uint16_t idx = 0;
    uint8_t find = 0;

    do
    {
        /* find tag */
        for (i = 0; i < plen; i++)
        {
            if (pBuf[i] == SOH)
            {
                idx = i;
                find = SOH;
                break;
            }
            else if (pBuf[i] == EOT)
            {
                idx = i;
                find = EOT;
                break;
            }
        }

        if (!find)
        {
            break;
        }

        if (find == EOT)
        {
            if (plen > 1)
            {
                bsp_uart_stdout_char(NAK);
                break;
            }
            else
            {
                *datalen = 1;
                *offset = idx;
                t_return = ZB_DATA_VALID;
            }
        }


        if (find == SOH)
        {
            if (plen < 132)
            {
                break;
            }
            plen -= idx;
            *datalen = 132;
            *offset = idx;
            t_return = ZB_DATA_VALID;

        }

    } while (0);

    return t_return;
}
