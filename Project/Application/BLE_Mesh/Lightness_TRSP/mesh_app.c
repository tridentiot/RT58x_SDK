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

/* user define element & model*/
#include "ble_mesh_element.h"

/* Mesh Library api */
#include "ble_mesh_lib_api.h"

/* Utility Library api */
#include "util_log.h"
#include "bsp.h"
#include "bsp_button.h"
#include "bsp_console.h"

#include "mesh_app.h"
#include "mesh_mdl_handler.h"

//=============================================================================
//                Public Global Variables Declaration
//=============================================================================
extern ble_mesh_element_param_t g_element_info[];

xQueueHandle app_msg_q;
//=============================================================================
//                Private Definitions of const value
//=============================================================================

#define MAX_TRSP_DATA_LEN           (377)

#define INVALID_DST_ADDRESS         (0x0000)

// Advertising device name
#define DEVICE_NAME                 'B', 'L', 'E', '_', 'M', 'E', 'S', 'H'

//=============================================================================
//                Private ENUM
//=============================================================================
typedef enum
{
    APP_GATT_PROVISION_EVT,
    APP_MESH_PROVISION_EVT,
    APP_PROVISIONING_EVT,
    APP_PROVISION_DONE_EVT,
    APP_IDLE_EVT,
} app_main_evt_t;

//=============================================================================
//                Private Struct
//=============================================================================

typedef struct
{
    const char          *p_cmd_example;
    const char          *p_cmd_header;
    void                (*p_cmd_process)(uint8_t *p_data);
} user_cmd_t;

//=============================================================================
//                Private Function Declaration
//=============================================================================
static uint8_t app_mesh_event_handler(uint32_t data, mesh_tlv_t *p_mesh_tlv);
static uint8_t app_button_handler(uint32_t data, mesh_tlv_t *p_mesh_tlv);
static uint8_t app_uart_handler(uint32_t data, mesh_tlv_t *p_mesh_tlv);
static void user_data_dst_address_set(uint8_t *p_rx_data);
static void user_data_tx_ack_set(uint8_t *p_rx_data);

//=============================================================================
//                Private Global Variables
//=============================================================================
uint8_t (* const app_event_handler[])(uint32_t data, mesh_tlv_t *p_mesh_tlv) =
{
    app_mesh_event_handler,
    app_button_handler,
    app_uart_handler,
};

static user_cmd_t g_user_cmd_list[] =
{
    {
        .p_cmd_example = "DstAddr0x001c",
        .p_cmd_header =  "DstAddr0x",
        .p_cmd_process = user_data_dst_address_set,
    },
    {
        .p_cmd_example = "TxAckEnable0",
        .p_cmd_header =  "TxAckEnable",
        .p_cmd_process = user_data_tx_ack_set,
    },
};


static uint32_t app_main_event = APP_GATT_PROVISION_EVT;

static mesh_app_cfg_t gt_app_cfg;
static TimerHandle_t unprov_idc_tmr;

static uint16_t user_data_dst_addr = INVALID_DST_ADDRESS;
static uint16_t user_data_appkey_idx = 0xFFFF;
static uint8_t user_data_req_ack = 0;

static uint8_t user_cmd_num = (sizeof(g_user_cmd_list) / sizeof(user_cmd_t));


static const uint8_t         DEVICE_NAME_STR[] = {DEVICE_NAME};

//=============================================================================
//                Private Function
//=============================================================================

static uint8_t parse_hex_digit(char c)
{
    uint8_t result = 0xff;

    if ((c >= '0') && (c <= '9'))
    {
        result = c - '0';
    }
    else if ((c >= 'a') && (c <= 'f'))
    {
        result = c - 'a' + 10;
    }
    else if ((c >= 'A') && (c <= 'F'))
    {
        result = c - 'A' + 10;
    }

    return result;
}

static void unprov_tmr_cb(TimerHandle_t t_timer)
{
    extern void set_duty_cycle(pwm_seq_para_head_t *pwm_para_config, uint16_t current_lv);
    extern pwm_seq_para_head_t pwm_para_config[3];
    extern light_lightness_state_t  el0_light_lightness_state;
    static uint8_t toggle;

    if (pib_is_provisioned())
    {
        set_duty_cycle(&pwm_para_config[0], el0_light_lightness_state.lightness_actual);
        set_duty_cycle(&pwm_para_config[1], el0_light_lightness_state.lightness_actual);
        set_duty_cycle(&pwm_para_config[2], el0_light_lightness_state.lightness_actual);

        xTimerStop(unprov_idc_tmr, 0);
    }
    else
    {
        toggle ^= 1;
        if (toggle)
        {
            set_duty_cycle(&pwm_para_config[0], 0xFFFF);
            set_duty_cycle(&pwm_para_config[1], 0xFFFF);
            set_duty_cycle(&pwm_para_config[2], 0xFFFF);
        }
        else
        {
            set_duty_cycle(&pwm_para_config[0], 0);
            set_duty_cycle(&pwm_para_config[1], 0);
            set_duty_cycle(&pwm_para_config[2], 0);
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
        if (p_buf == NULL)
        {
            info_color(LOG_RED, "[%s] malloc fail, len = %d !\n", __func__, data_len);
            break;
        }

        err = mesh_evt_msg_recv(p_buf, &data_len);

        if (err == 0)
        {
            app_q.event = APP_BLE_MESH_EVT;
            app_q.pt_tlv = (mesh_tlv_t *)p_buf;
            while (xQueueSendToBack(app_msg_q, &app_q, 20) != pdPASS)
            {
                info_color(LOG_RED, "[%s] send fail\n", __func__);
            };
        }
        else
        {
            info_color(LOG_RED, "[%s] sys_err = %d !\n", __func__, err);
            vPortFree(p_buf);
        }
    } while (0);
}

static void user_data_dst_address_set(uint8_t *p_rx_data)
{
    uint8_t dst_addr[4];
    dst_addr[0] = parse_hex_digit((char)p_rx_data[0]);
    dst_addr[1] = parse_hex_digit((char)p_rx_data[1]);
    dst_addr[2] = parse_hex_digit((char)p_rx_data[2]);
    dst_addr[3] = parse_hex_digit((char)p_rx_data[3]);

    user_data_dst_addr = (((dst_addr[0] & 0x0F) << 12) |
                          ((dst_addr[1] & 0x0F) << 8) |
                          ((dst_addr[2] & 0x0F) << 4) | (dst_addr[3] & 0x0F));

    info_color(LOG_GREEN, "destination address for uart data: 0x%04x\n", user_data_dst_addr);
}

static void user_data_tx_ack_set(uint8_t *p_rx_data)
{
    user_data_req_ack = parse_hex_digit((char)p_rx_data[0]);

    info_color(LOG_GREEN, "uart data tx with ack %d\n", user_data_req_ack);
}

static void uart_data_handler(char ch)
{
    static uint8_t rx_buffer[MAX_TRSP_DATA_LEN + 1];
    static uint16_t index = 0;
    uint16_t i, cmd_processed = false;
    mmdl_transmit_info_t tx_info;
    raf_trsp_set_msg_t *p_raf_trsp_set_msg;
    int status;

    rx_buffer[index++] = ch;

    if (index > (MAX_TRSP_DATA_LEN + 1)) /* (+1) length '\r' or '\n' */
    {
        info_color(LOG_RED, "uart data out of memory %d\n", index - 1);
        index = 0;
    }
    else if ((ch == '\n') || (ch == '\r'))
    {
        for (i = 0 ; i < user_cmd_num; i++)
        {
            if ((index == strlen(g_user_cmd_list[i].p_cmd_example) + 1/* (+1) length '\r' or '\n' */) &&
                    (strncmp((char *)rx_buffer, g_user_cmd_list[i].p_cmd_header, strlen(g_user_cmd_list[i].p_cmd_header)) == 0))
            {
                g_user_cmd_list[i].p_cmd_process(rx_buffer + strlen(g_user_cmd_list[i].p_cmd_header));
                cmd_processed = true;
            }
        }

        if (cmd_processed == false)
        {
            if (user_data_dst_addr != INVALID_DST_ADDRESS)
            {
                if (user_data_appkey_idx == 0xFFFF)
                {
                    info_color(LOG_RED, "no APP key bind to Rafael TRSP client model\n");
                }
                else if (pib_is_provisioned())
                {
                    info_color(LOG_YELLOW, "Send Rafael TRSP set, dst addr 0x%04x\n", user_data_dst_addr);

                    for (i = 0; i < (index - 1); i++)
                    {
                        info_color(LOG_YELLOW, "%02x ", rx_buffer[i]);
                    }
                    info_color(LOG_YELLOW, "\n... " );


                    p_raf_trsp_set_msg = pvPortMalloc(sizeof(raf_trsp_set_msg_t) + (index - 1/*(-1) removed '\n' or '\r'*/));
                    if (p_raf_trsp_set_msg == NULL)
                    {
                        info_color(LOG_YELLOW, "fail (no memory)\n");
                    }
                    else
                    {
                        tx_info.dst_addr = user_data_dst_addr;
                        tx_info.src_addr = pib_primary_address_get();
                        tx_info.appkey_index = user_data_appkey_idx;
                        p_raf_trsp_set_msg->data_len = (index - 1/*(-1) removed '\n' or '\r'*/);
                        memcpy(p_raf_trsp_set_msg->data, rx_buffer, p_raf_trsp_set_msg->data_len);

                        if (user_data_req_ack)
                        {
                            status = mmdl_rafael_trsp_send_set(tx_info, p_raf_trsp_set_msg);
                        }
                        else
                        {
                            status = mmdl_rafael_trsp_send_unack_set(tx_info, p_raf_trsp_set_msg);

                        }
                        info_color(LOG_YELLOW, "result %d\n", status);
                        vPortFree(p_raf_trsp_set_msg);
                    }
                }
                else
                {
                    info_color(LOG_RED, "device was not provision\n");
                }

            }
            else
            {
                info_color(LOG_RED, "destination address invalid, uart data ignore\n");
                info_color(LOG_YELLOW, "command example:\n");
                for (i = 0 ; i < user_cmd_num; i++)
                {
                    info_color(LOG_YELLOW, "%s\n", g_user_cmd_list[i].p_cmd_example);
                }
            }
        }
        index = 0;

    }
}

static uint8_t app_button_handler(uint32_t data, mesh_tlv_t *p_mesh_tlv)
{
    uint32_t pin = data;
    uint8_t memory_free = false;

    switch (pin)
    {
    case BSP_EVENT_BUTTONS_0: //Mesh provsion start
    case BSP_EVENT_BUTTONS_1: //Gatt provsion start
        if (app_main_event == APP_PROVISIONING_EVT)
        {
            ble_mesh_provision_stop();
        }

        if (pib_is_provisioned())
        {
            ble_mesh_proxy_disable();
        }
        element_info_init();
        pib_provision_info_local_reset();
        app_main_event = (pin == BSP_EVENT_BUTTONS_0) ? APP_MESH_PROVISION_EVT : APP_GATT_PROVISION_EVT;
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

static uint8_t app_uart_handler(uint32_t data, mesh_tlv_t *p_mesh_tlv)
{
    uint8_t memory_free = false;
    char ch;

    while (bsp_console_stdin_str(&ch, 1) > 0)
    {
        uart_data_handler(ch);
    }
    return memory_free;
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
    mesh_prov_start_cfm_t *p_start_cfm;
    uint8_t memory_free = true, i;

    switch (p_mesh_tlv->type)
    {
    case TYPE_MESH_PROV_START_CFM:
    {
        p_start_cfm = (mesh_prov_start_cfm_t *)p_mesh_tlv->value;
        if (p_start_cfm->status != 0)
        {
            info_color(LOG_RED, "Provision timeout!\n");
            app_main_event = APP_GATT_PROVISION_EVT;
        }
        else
        {
            info_color(LOG_RED, "Provision done! address 0x%04x\n", p_start_cfm->element_address);
            for (i = 0 ; i < ble_mesh_element_count ; i++)
            {
                g_element_info[i].element_address = p_start_cfm->element_address + i;
            }
            app_main_event = APP_PROVISION_DONE_EVT;
        }
    }
    break;

    case TYPE_MESH_NODE_RESET_IDC:
    {
        info_color(LOG_GREEN, "Recv CfgNodeRest ... \n");
        element_info_init();
        app_main_event = APP_GATT_PROVISION_EVT;
    }
    break;

    case TYPE_MESH_BLE_SVC_TRSPS_WRITE_IDC:
    {
        ble_trsps_evt_msg_handler(p_mesh_tlv->length, p_mesh_tlv->value);
    }
    break;

    case TYPE_MESH_APP_MDL_EVT_MSG_IDC:
    {
        mmdl_evt_msg_cb((mesh_app_mdl_evt_msg_idc_t *)p_mesh_tlv->value);
    }
    break;

    case TYPE_MESH_CFG_MDL_APP_BIND_IDC:
    {
        mesh_cfg_mdl_app_bind_idc_t *p_app_bind_idc;
        p_app_bind_idc = (mesh_cfg_mdl_app_bind_idc_t *)p_mesh_tlv->value;

        if (p_app_bind_idc->model_id == MMDL_RAFAEL_TRSP_SR_MDL_ID)
        {
            info_color(LOG_YELLOW, "Appkey index %d bind to Rafael TRSP server model\n", p_app_bind_idc->appkey_index);
        }
        else if (p_app_bind_idc->model_id == MMDL_RAFAEL_TRSP_CL_MDL_ID)
        {
            info_color(LOG_YELLOW, "Appkey index %d bind to Rafael TRSP client model\n", p_app_bind_idc->appkey_index);
            user_data_appkey_idx = p_app_bind_idc->appkey_index;
        }
    }
    break;

    case TYPE_MESH_PROVISION_COMPLETE_IDC:
    {
        mesh_prov_complete_idc_t *p_prov_device;

        p_prov_device = (mesh_prov_complete_idc_t *)p_mesh_tlv->value;
        if (p_prov_device->status == 1)
        {
            info_color(LOG_RED, "Device provision fail  ... \n");
        }
    }
    break;

    case TYPE_MESH_PROV_ADV_ENABLED_IDC:
    {
        mesh_prov_adv_enable_idc_t *p_adv_enable_status;
        p_adv_enable_status = (mesh_prov_adv_enable_idc_t *)p_mesh_tlv->value;
        info_color(LOG_YELLOW, "Provision ADV enable result: %d\n", p_adv_enable_status->status);
    }
    break;

    case TYPE_MESH_PROXY_ENABLE_IDC:
    {
        //mesh_proxy_adv_enable_idc_t *p_adv_enable_status;
        //p_adv_enable_status = (mesh_proxy_adv_enable_idc_t *)p_mesh_tlv->value;
        //info_color(LOG_YELLOW, "Proxy ADV enable result: %d\n", p_adv_enable_status->status);
    }
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
    case APP_MESH_PROVISION_EVT:
        if (pib_is_provisioned() == 0)
        {
            ble_mesh_provision_start(0);
            info("Start mesh provision!\n");
            app_main_event = APP_PROVISIONING_EVT;
            xTimerStart(unprov_idc_tmr, 0);
        }
        else
        {
            app_main_event = APP_IDLE_EVT;
        }
        break;

    case APP_GATT_PROVISION_EVT:
        if (pib_is_provisioned() == 0)
        {
            ble_mesh_provision_start(1);
            info("Start gatt provision!\n");
            app_main_event = APP_PROVISIONING_EVT;
            xTimerStart(unprov_idc_tmr, 0);
        }
        else
        {
            app_main_event = APP_IDLE_EVT;
        }
        break;

    case APP_PROVISIONING_EVT:
        break;

    case APP_PROVISION_DONE_EVT:
        info_color(LOG_CYAN, "Provision Complete!!\n");

        ble_mesh_proxy_enable();
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
    ble_mesh_model_param_t  *p_model;
    app_queue_t app_q;
    uint8_t i, j;

    ble_mesh_device_name_set(sizeof(DEVICE_NAME_STR), (uint8_t *)DEVICE_NAME_STR);
    if (pib_is_provisioned())
    {
        for (i = 0 ; i < ble_mesh_element_count ; i++)
        {
            g_element_info[i].element_address = pib_primary_address_get() + i;

            //search the appkey index which bind to model "MMDL_RAFAEL_TRSP_CL_MDL_ID"
            if ((user_data_appkey_idx == 0xFFFF) &&
                    (search_model(g_element_info + i, MMDL_RAFAEL_TRSP_CL_MDL_ID, &p_model)))
            {
                for (j = 0 ; j < RAF_BLE_MESH_MODEL_BIND_LIST_SIZE ; j++)
                {
                    if (p_model->binding_list[j] != 0xFFFF)
                    {
                        user_data_appkey_idx = p_model->binding_list[j];
                        break;
                    }
                }
            }
        }

        ble_mesh_proxy_enable();
        info_color(LOG_CYAN, "Device provisioned ... primary address 0x%04x\n", pib_primary_address_get());
    }

    for (;;)
    {
        app_main_loop();

        if (xQueueReceive(app_msg_q, &app_q, 0) == pdTRUE)
        {
            if (app_event_handler[app_q.event](app_q.data, app_q.pt_tlv) == true)
            {
                vPortFree(app_q.pt_tlv);
            }
        }
    }
}

//=============================================================================
//                Public Function
//=============================================================================
void app_init(void)
{
    info_color(LOG_CYAN, "BLE Mesh Stack initial ...\n");

    pib_init(NULL, g_element_info, ble_mesh_element_count, NULL);

    mmdl_init();

    gt_app_cfg.pf_app_task_loop = app_main_task;
    gt_app_cfg.u32_stack_size = 256;
    gt_app_cfg.pf_evt_indication = mesh_app_evt_indication_cb;

    mesh_stack_init(&gt_app_cfg);

    app_msg_q = xQueueCreate(16, sizeof(app_queue_t));

    unprov_idc_tmr = xTimerCreate("100ms", pdMS_TO_TICKS(100), pdTRUE, (void *)0, unprov_tmr_cb);

}


