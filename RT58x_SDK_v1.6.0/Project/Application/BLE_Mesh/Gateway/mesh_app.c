/**
 * Copyright (c) 2021 All Rights Reserved.
 */
/** @file mesh_app.c
 *
 * @author Rex
 * @version 0.1
 * @date 2021/10/19
 * @license
 * @description
 */

//=============================================================================
//                Include
//=============================================================================
/* OS API */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Mesh Library api */
#include "mesh_api.h"
#include "ble_mesh_lib_api.h"

/* Utility Library api */
#include "util_log.h"

#include "mmdl_common.h"
#include "uart_handler.h"
#include "ble_mesh_gateway.h"
#include "cfgmdl_client.h"

#include "mesh_app.h"
#include "bsp.h"

#if (MODULE_ENABLE(SUPPORT_DEBUG_CONSOLE_CLI))
extern void app_model_evt_parse(mesh_app_mdl_evt_msg_idc_t *pt_msg_idc);
extern void cfg_model_evt_parse(mesh_cfg_mdl_evt_msg_idc_t *pt_msg_idc);
extern void auto_prov_device_start(uint16_t primary_addr, uint8_t *p_device_uuid);
extern void auto_prov_device_complete(uint16_t *p_primary_addr, mesh_prov_complete_idc_t *p_prov_complete_idc);
#endif

//=============================================================================
//                Public Global Variables Declaration
//=============================================================================

xQueueHandle app_msg_q;

//=============================================================================
//                Private Definitions of const value
//=============================================================================

// Advertising device name
#define DEVICE_NAME                 'M', 'E', 'S', 'H', '_', 'G', 'W'

//=============================================================================
//                Private ENUM
//=============================================================================
typedef enum
{
    APP_INIT_EVT,
    APP_IDLE_EVT,
} app_main_evt_t;

//=============================================================================
//                Private Struct
//=============================================================================

//=============================================================================
//                Private Function Declaration
//=============================================================================
static uint8_t app_mesh_event_handler(uint32_t data, mesh_tlv_t *p_mesh_tlv);
static uint8_t app_button_handler(uint32_t data, mesh_tlv_t *p_mesh_tlv);

//=============================================================================
//                Private Global Variables
//=============================================================================
static uint32_t app_main_event = APP_INIT_EVT;
static mesh_app_cfg_t gt_app_cfg;
static const uint8_t  DEVICE_NAME_STR[] = {DEVICE_NAME};

uint8_t (* const app_event_handler[])(uint32_t data, mesh_tlv_t *p_mesh_tlv) =
{
    app_mesh_event_handler,     //APP_BLE_MESH_EVT
    app_button_handler,         //APP_BUTTON_EVT
    ble_mesh_gateway_cmd_proc,  //APP_QUEUE_UART_MSG_EVT
};

//=============================================================================
//                Private Functions
//=============================================================================
static void app_model_evt_msg_handler(mesh_app_mdl_evt_msg_idc_t *pt_msg_idc)
{
#if (MODULE_ENABLE(SUPPORT_DEBUG_CONSOLE_CLI))
    app_model_evt_parse(pt_msg_idc);
#else
    uint32_t i;
    info("Recv APP Model event messages[%04X][0x%04X -> 0x%04X] ... \n", BE2LE16(pt_msg_idc->opcode), pt_msg_idc->src_addr, pt_msg_idc->dst_addr);
    info("Parameter:\n");
    for (i = 0; i < pt_msg_idc->parameter_len ; i++)
    {
        info("%02x ", pt_msg_idc->parameter[i]);
    }
    info("\n");

#endif
    ble_mesh_gateway_cmd_send(DEVICE_APPLICATION_SVC_CMD, pt_msg_idc->src_addr, pt_msg_idc->appkey_index,
                              pt_msg_idc->opcode, pt_msg_idc->parameter, pt_msg_idc->parameter_len);


}

static void cfg_model_evt_msg_handler(mesh_cfg_mdl_evt_msg_idc_t *pt_msg_idc)
{
#if (MODULE_ENABLE(SUPPORT_DEBUG_CONSOLE_CLI))
    cfg_model_evt_parse(pt_msg_idc);
#else
    uint32_t i;
    info("Recv Configure Model event messages[%04X][0x%04X] ... \n", pt_msg_idc->opcode, pt_msg_idc->src_addr);
    info("Parameter:\n");
    for (i = 0; i < pt_msg_idc->parameter_len ; i++)
    {
        info("%02x ", pt_msg_idc->parameter[i]);
    }
    info("\n");
#endif

    ble_mesh_gateway_cmd_send(DEVICE_CONFIGURATION_SVC_CMD, pt_msg_idc->src_addr, NULL,
                              pt_msg_idc->opcode, pt_msg_idc->parameter, pt_msg_idc->parameter_len);

}

static void ble_trsps_evt_msg_handler(uint16_t len, uint8_t *p_trsps_data)
{
    uint16_t i;
    bool tx_success;

    info("Receive BLE TRSPS data:\n");
    for (i = 0; i < len ; i++)
    {
        info("%02x ", p_trsps_data[i]);
    }
    info("\n");

    tx_success = ble_trsps_data_set(len, p_trsps_data);

    info("Loopback received BLE TRSPS data %d:\n", tx_success);
}

static uint8_t app_mesh_event_handler(uint32_t data, mesh_tlv_t *p_mesh_tlv)
{
    uint8_t memory_free = true;
#if (MODULE_ENABLE(SUPPORT_DEBUG_CONSOLE_CLI))
    static uint16_t primary_addr = 0x0100;
#endif

    if (p_mesh_tlv->type == TYPE_MESH_UNPROV_DEVICE_IDC)
    {
#if (MODULE_ENABLE(SUPPORT_DEBUG_CONSOLE_CLI))
        mesh_unprov_device_idc_t *p_unprov_device_idc = (mesh_unprov_device_idc_t *)p_mesh_tlv->value;

        auto_prov_device_start(primary_addr, p_unprov_device_idc->uuid);
#endif
        ble_mesh_gateway_cmd_send(MESH_NWK_SVC_CMD, NULL, NULL, MESH_NWK_OPCODE_UNPROV_DEVICE_LIST, p_mesh_tlv->value, p_mesh_tlv->length);

    }
    else if (p_mesh_tlv->type == TYPE_MESH_PROVISION_COMPLETE_IDC)
    {
#if (MODULE_ENABLE(SUPPORT_DEBUG_CONSOLE_CLI))
        mesh_prov_complete_idc_t *p_prov_complete_idc = (mesh_prov_complete_idc_t *)p_mesh_tlv->value;

        auto_prov_device_complete(&primary_addr, p_prov_complete_idc);
#endif
        ble_mesh_gateway_cmd_send(MESH_NWK_SVC_CMD, NULL, NULL, MESH_NWK_OPCODE_DEVICE_PROV_STAUS, p_mesh_tlv->value, p_mesh_tlv->length);

    }
    else if (p_mesh_tlv->type == TYPE_MESH_APP_MDL_EVT_MSG_IDC)
    {
        app_model_evt_msg_handler((mesh_app_mdl_evt_msg_idc_t *)p_mesh_tlv->value);
    }
    else if (p_mesh_tlv->type == TYPE_MESH_CFG_MDL_EVT_MSG_IDC)
    {
        cfg_model_evt_msg_handler((mesh_cfg_mdl_evt_msg_idc_t *)p_mesh_tlv->value);
    }
    else if (p_mesh_tlv->type == TYPE_MESH_BLE_SVC_TRSPS_WRITE_IDC)
    {
        ble_trsps_evt_msg_handler(p_mesh_tlv->length, p_mesh_tlv->value);
    }
    else if (p_mesh_tlv->type == TYPE_MESH_BLE_TRSPS_CONNECT_CREATE_IDC)
    {
        info_color(LOG_YELLOW, "BLE TRSPS connection creating, status %d\n", p_mesh_tlv->value[0]);
    }
    else if (p_mesh_tlv->type == TYPE_MESH_BLE_TRSPS_CONNECT_CANCEL_IDC)
    {
        info_color(LOG_YELLOW, "BLE TRSPS connection cancel, status %d\n", p_mesh_tlv->value[0]);
    }
    else if (p_mesh_tlv->type == TYPE_MESH_BLE_TRSPS_CONNECTED_IDC)
    {
        info_color(LOG_YELLOW, "BLE TRSPS connected\n");
    }
    else if (p_mesh_tlv->type == TYPE_MESH_BLE_TRSPS_DISCONNECTED_IDC)
    {
        info_color(LOG_YELLOW, "BLE TRSPS disconnected\n");
    }
    else if (p_mesh_tlv->type == TYPE_MESH_FRIEND_ESTABLISED_IDC)
    {
        mesh_friend_established_idc_t *p_friend_establish;

        p_friend_establish = (mesh_friend_established_idc_t *)p_mesh_tlv->value;
        info_color(LOG_YELLOW, "friendship is established, address: 0x%04x, element cnt %d\n", p_friend_establish->lpn_address,
                   p_friend_establish->element_cnt);
    }
    else if (p_mesh_tlv->type == TYPE_MESH_FRIEND_TERMINATED_IDC)
    {
        mesh_friend_terminated_idc_t *p_friend_terminated;

        p_friend_terminated = (mesh_friend_terminated_idc_t *)p_mesh_tlv->value;
        info_color(LOG_BLUE, "friendship is terminated, address: 0x%04x, reason %d\n", p_friend_terminated->lpn_address,
                   p_friend_terminated->terminate_reason);
    }
    return memory_free;
}

static uint8_t app_button_handler(uint32_t data, mesh_tlv_t *p_mesh_tlv)
{
    uint32_t pin = data;
    uint8_t memory_free = false;

    switch (pin)
    {
    case BSP_EVENT_BUTTONS_0:
        ble_trsps_connection_create();
        break;
    case BSP_EVENT_BUTTONS_1:
        ble_trsps_connection_cancel();
        break;
    case BSP_EVENT_BUTTONS_2:
        break;
    case BSP_EVENT_BUTTONS_3:
        break;
    case BSP_EVENT_BUTTONS_4:
        break;
    default:
        break;
    }

    return memory_free;
}

static void app_main_loop(void)
{
    switch (app_main_event)
    {
    case APP_INIT_EVT:
        app_main_event = APP_IDLE_EVT;
        break;

    case APP_IDLE_EVT:
        break;

    default:
        break;
    }
}

static void app_main_task(void)
{
    app_queue_t app_q;

    ble_mesh_device_name_set(sizeof(DEVICE_NAME_STR), (uint8_t *)DEVICE_NAME_STR);

    for (;;)
    {
        app_main_loop();

        if (xQueueReceive(app_msg_q, &app_q, 20) == pdTRUE)
        {
            if (app_event_handler[app_q.event](app_q.data, app_q.pt_tlv) == true)
            {
                vPortFree(app_q.pt_tlv);
            }
        }
    }
}

static void mesh_app_evt_indication_cb(uint32_t data_len)
{
    int err;
    uint8_t *p_buf = pvPortMalloc(data_len);
    app_queue_t app_q;

    do
    {
        if (!p_buf)
        {
            break;
        }
        app_q.event = APP_BLE_MESH_EVT;
        err = mesh_evt_msg_recv(p_buf, &data_len);
        app_q.pt_tlv = (mesh_tlv_t *)p_buf;
        if (err == 0)
        {
            xQueueSendToBack(app_msg_q, &app_q, 0);
        }
        else
        {
            info_color(LOG_RED, "[%s] sys_err = %d !\n", __func__, err);
            vPortFree(p_buf);
        }
    } while (0);
}

static void app_uart_msg_recv(mesh_tlv_t *pt_tlv)
{
    app_queue_t app_q;

    app_q.event = APP_QUEUE_UART_MSG_EVT;
    app_q.pt_tlv = pt_tlv;

    xQueueSendToBack(app_msg_q, &app_q, 0);
}

//=============================================================================
//                Public Functions
//=============================================================================
void app_init(void)
{
    uart_handler_parm_t uart_handler_param = {0};
    ble_gap_addr_t  device_addr = {.addr_type = RANDOM_STATIC_ADDR,
                                   .addr = {0x21, 0x11, 0x22, 0x33, 0x25, 0xC6}
                                  };

    gt_app_cfg.pf_app_task_loop = app_main_task;
    gt_app_cfg.u32_stack_size = 256;
    gt_app_cfg.pf_evt_indication = mesh_app_evt_indication_cb;

    info_color(LOG_CYAN, "RT58x SDK for BLE Mesh ...\n");

    pib_init(NULL, NULL, 1, &device_addr);

    mesh_stack_init(&gt_app_cfg);

    info_color(LOG_CYAN, "BLE Mesh Client Stack initial ...\n");
    app_msg_q = xQueueCreate(16, sizeof(app_queue_t));

    uart_handler_param.UartParserCB[0] = ble_mesh_gateway_cmd_parser;
    uart_handler_param.UartRecvCB[0] = app_uart_msg_recv;

    uart_handler_init(&uart_handler_param);

}


