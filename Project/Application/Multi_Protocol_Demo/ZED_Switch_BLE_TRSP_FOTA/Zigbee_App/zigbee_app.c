/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file zigbee_app.c
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

#include "zigbee_app.h"
#include "zigbee_lib_api.h"
#include "zigbee_zcl_msg_handler.h"
#include "zigbee_evt_handler.h"
/* BSP APIs */
#include "bsp_button.h"
#include "multi_app.h"
#include "bsp_led.h"

//=============================================================================
//                Global Variables
//=============================================================================
extern xQueueHandle g_app_msg_q;
SemaphoreHandle_t semaphore_zb;
SemaphoreHandle_t semaphore_btn;

//=============================================================================
//                Private Definitions of const value
//=============================================================================
/* Settings for the send step command */
#define LIGHT_CONTROL_DIMM_STEP 15
#define LIGHT_CONTROL_DIMM_TRANSACTION_TIME 2
//=============================================================================
//                Private ENUM
//=============================================================================

//=============================================================================
//                Private Struct
//=============================================================================

typedef struct ZIGBEE_APP_JOIN_INFO_T
{
    uint8_t join_type;
    uint8_t join_cnt;
} zigbee_app_join_info_t;

//=============================================================================
//                Private Global Variables
//=============================================================================
zigbee_app_join_info_t g_join_info = {APP_ZB_JOIN, 0};
//=============================================================================
//                Functions
//=============================================================================

void zb_app_main(uint32_t event)
{
    uint8_t  extPANID[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  //address all zero is able to join to any suitable PAN

    switch (event)
    {
    case APP_INIT_EVT:

        if (zigbee_ed_nwk_start_request(ZIGBEE_CHANNEL_ALL_MASK(), 0x0 /*ZIGBEE_CHANNEL_MASK(26)*/, false, 3000, extPANID, !bsp_button_state_get(BSP_BUTTON_0)) != 0) /*0: SYS_ERR_OK*/
        {
            zigbee_app_evt_change(APP_INIT_EVT, FALSE);
        }
        break;

    case APP_NOT_JOINED_EVT:
        bsp_led_on(ZB_NWK_NOT_FOUND_INDICATOR);
        zigbee_join_request();
        break;

    case APP_ZB_JOINED_EVT:
        bsp_led_Off(ZB_NWK_NOT_FOUND_INDICATOR);
        bsp_led_Off(ZB_NWK_FINDING_INDICATOR);
        break;

    case APP_START_REJOIN_EVT:
        zigbee_rejoin_request();
        break;

    case APP_LED_TOGGLE_EVT:
        info_color(LOG_YELLOW, "Toggle from BLE\n");
        send_toggle();
        break;

    case APP_LED_LEVEL_UP_EVT:
        info_color(LOG_YELLOW, "Level up from BLE\n");
        send_level_step(0);
        break;

    case APP_LED_LEVEL_DOWN_EVT:
        info_color(LOG_YELLOW, "Level down from BLE\n");
        send_level_step(1);
        break;

    case APP_LED_MOVE_TEMP_EVT:
        info_color(LOG_YELLOW, "Move color temperature from BLE\n");
        send_move_color();
        break;

    default:
        info_color(LOG_RED, "zigbee app unkown event %d\n", event);
        break;
    }
}

void zb_event_parse(sys_tlv_t *pt_zb_tlv)
{
    switch (pt_zb_tlv->type)
    {
    case ZIGBEE_EVT_TYPE_ZCL_DATA_IDC:
        zigbee_zcl_msg_handler(pt_zb_tlv);
        break;

    default:
        zigbee_evt_handler(pt_zb_tlv);
        break;
    }

    if (pt_zb_tlv)
    {
        vPortFree(pt_zb_tlv);
    }

}

void send_toggle(void)
{
    // on/off no payload
    zigbee_zcl_data_req_t *pt_data_req;

    ZIGBEE_ZCL_DATA_REQ(pt_data_req, 0, ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT, 0, BUTTON_1_EP, ZB_ZCL_CLUSTER_ID_ON_OFF,
                        ZB_ZCL_CMD_ON_OFF_TOGGLE_ID, TRUE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0)

    if (pt_data_req)
    {
        zigbee_zcl_request(pt_data_req, 0);
        vPortFree(pt_data_req);
    }
}

void send_on(void)
{
    // on/off no payload
    zigbee_zcl_data_req_t *pt_data_req;

    ZIGBEE_ZCL_DATA_REQ(pt_data_req, 0, ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT, 0, BUTTON_1_EP, ZB_ZCL_CLUSTER_ID_ON_OFF,
                        ZB_ZCL_CMD_ON_OFF_ON_ID, TRUE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0)


    if (pt_data_req)
    {
        zigbee_zcl_request(pt_data_req, 0);
        sys_free(pt_data_req);
    }
}

void send_off(void)
{
    // on/off no payload
    zigbee_zcl_data_req_t *pt_data_req;

    ZIGBEE_ZCL_DATA_REQ(pt_data_req, 0, ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT, 0, BUTTON_1_EP, ZB_ZCL_CLUSTER_ID_ON_OFF,
                        ZB_ZCL_CMD_ON_OFF_OFF_ID, TRUE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, 0, 0)


    if (pt_data_req)
    {
        zigbee_zcl_request(pt_data_req, 0);
        sys_free(pt_data_req);
    }
}

void send_level_step(uint8_t dir)
{
    // on/off no payload
    zigbee_zcl_data_req_t *pt_data_req;

    ZIGBEE_ZCL_DATA_REQ(pt_data_req, 0, ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT, 0, BUTTON_2_EP, ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL,
                        ZB_ZCL_CMD_LEVEL_CONTROL_STEP, TRUE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, 0, 4)


    if (pt_data_req)
    {
        pt_data_req->cmdFormat[0] = dir;
        pt_data_req->cmdFormat[1] = LIGHT_CONTROL_DIMM_STEP;
        pt_data_req->cmdFormat[2] = LIGHT_CONTROL_DIMM_TRANSACTION_TIME & 0xFF;
        pt_data_req->cmdFormat[3] = (LIGHT_CONTROL_DIMM_TRANSACTION_TIME >> 8) & 0xFF;
        zigbee_zcl_request(pt_data_req, 4);
        vPortFree(pt_data_req);
    }
}

void send_move_color(void)
{
    zigbee_zcl_data_req_t *pt_data_req;
    uint16_t rate_x, rate_y;

    ZIGBEE_ZCL_DATA_REQ(pt_data_req, 0, ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT, 0, BUTTON_3_EP, ZB_ZCL_CLUSTER_ID_COLOR_CONTROL,
                        ZB_ZCL_CMD_COLOR_CONTROL_MOVE_COLOR, TRUE, TRUE, ZCL_FRAME_CLIENT_SERVER_DIR, 0, 4)

    rate_x = sys_random() % 0xFFFF;
    rate_y = sys_random() % 0xFFFF;

    if (pt_data_req)
    {
        pt_data_req->cmdFormat[0] = rate_x & 0xFF;
        pt_data_req->cmdFormat[1] = (rate_x >> 8) & 0xFF;
        pt_data_req->cmdFormat[2] = rate_x & 0xFF;
        pt_data_req->cmdFormat[3] = (rate_y >> 8) & 0xFF;
        zigbee_zcl_request(pt_data_req, 4);
        vPortFree(pt_data_req);
    }
}

void zb_app_evt_indication_cb(uint32_t data_len)
{
    int i32_err;
    uint8_t *pBuf;
    multi_app_queue_t t_app_q;

    if (xSemaphoreTake(semaphore_zb, 0) == pdPASS)
    {
        do
        {
            pBuf = pvPortMalloc(data_len);
            if (!pBuf)
            {
                xSemaphoreGive(semaphore_zb);
                break;
            }
            t_app_q.event = 0;
            i32_err = zigbee_event_msg_recvfrom(pBuf, &data_len);
            t_app_q.param_type = QUEUE_TYPE_ZIGBEE;
            t_app_q.param.pt_zb_tlv = (sys_tlv_t *)pBuf;
            if (i32_err == 0)
            {
                while (xQueueSendToBack(g_app_msg_q, &t_app_q, 20) != pdTRUE);
            }
            else
            {
                info_color(LOG_RED, "[%s] sys_err = %d !\n", __func__, i32_err);
                xSemaphoreGive(semaphore_zb);
                vPortFree(pBuf);
            }
        } while (0);
    }
}

uint8_t zigbee_app_join_continue(uint8_t join_type)
{
    uint8_t conti_join = false, join_threshold;

    join_threshold = (join_type == APP_ZB_JOIN) ? ZB_APP_TRY_JOIN_CNT : ZB_APP_TRY_REJOIN_CNT;
    g_join_info.join_type = join_type;
    if (g_join_info.join_cnt < join_threshold)
    {
        g_join_info.join_cnt++;
        conti_join = true;
    }

    return conti_join;
}

void zigbee_app_join_info_reset(uint8_t try_join)
{
    g_join_info.join_cnt = 0;

    if (try_join == true)
    {
        if (g_join_info.join_type == APP_ZB_JOIN)
        {
            zigbee_app_evt_change(APP_NOT_JOINED_EVT, FALSE);
            info_color(LOG_YELLOW, "do join\n");
        }
        else
        {
            zigbee_app_evt_change(APP_START_REJOIN_EVT, FALSE);
            info_color(LOG_YELLOW, "do rejoin\n");
        }

        bsp_led_on(ZB_NWK_FINDING_INDICATOR);

    }
}

uint8_t zigbee_app_evt_change(uint32_t evt, uint8_t from_isr)
{
    //app_event_state = evt;
    multi_app_queue_t t_app_q;

    if (xSemaphoreTake(semaphore_zb, 0) == pdPASS)
    {
        t_app_q.event = APP_QUEUE_ZIGBEE_EVT; // from Zigbee
        t_app_q.param_type = QUEUE_TYPE_APP_REQ;
        t_app_q.param.zb_app_req = evt;

        if (from_isr == false)
        {
            if (xQueueSendToBack(g_app_msg_q, &t_app_q, 20) != pdTRUE)
            {
                // send error
                xSemaphoreGive(semaphore_zb);
                return false;
            }
        }
        else
        {
            BaseType_t context_switch = pdFALSE;
            xQueueSendToBackFromISR(g_app_msg_q, &t_app_q, &context_switch);

            if (context_switch != pdTRUE)
            {
                // send error
                xSemaphoreGive(semaphore_zb);
                return false;
            }
        }
        return true;
    }
    return false;
}



