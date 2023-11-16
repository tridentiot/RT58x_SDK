/** @file
 *
 * @brief
 *
 */


/**************************************************************************************************
 *    INCLUDES
 *************************************************************************************************/
#include "cm3_mcu.h"
#include "project_config.h"

/* OS Wrapper APIs*/
#include "sys_arch.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ble_api.h"
#include "zigbee_app.h"
#include "ble_app.h"
#include "multi_app.h"

#include "bsp_console.h"
#include "zigbee_gateway.h"

#include "mfs.h"
/**************************************************************************************************
 *    MACROS
 *************************************************************************************************/
/**************************************************************************************************
 *    CONSTANTS AND DEFINES
 *************************************************************************************************/

/**************************************************************************************************
 *    TYPEDEFS
 *************************************************************************************************/

/**************************************************************************************************
 *    GLOBAL VARIABLES
 *************************************************************************************************/
xQueueHandle g_app_msg_q;

static zigbee_cfg_t gt_zb_app_cfg;
static ble_cfg_t gt_ble_app_cfg;

//=============================================================================
//                Private Global Variables
//=============================================================================
/**************************************************************************************************
 *    EXTERNS
 *************************************************************************************************/

/**************************************************************************************************
 *    LOCAL FUNCTIONS
 *************************************************************************************************/
static void app_main_task(void *arg)
{
    multi_app_queue_t t_app_q;
    ble_err_t status;

    // BLE application init
    status = ble_init();
    if (status != BLE_ERR_OK)
    {
        info_color(LOG_RED, "[DEBUG_ERR] ble_app_ble_init() fail: %d\n", status);
        while (1);
    }

    gw_table_initial();
    //zigbee application init
    zigbee_app_evt_change(APP_INIT_EVT, false);
    // start adv
    app_request_set(APP_TRSP_P_HOST_ID, APP_REQUEST_ADV_START, false);

    for (;;)
    {
        if (xQueueReceive(g_app_msg_q, &t_app_q, 20) == pdTRUE)
        {
            switch (t_app_q.param_type)
            {
            case QUEUE_TYPE_APP_REQ:
            {
                switch (t_app_q.event)
                {
                case APP_QUEUE_BLE_EVT:
                case APP_QUEUE_ISR_BLE_EVT:
                {
                    (t_app_q.event == APP_QUEUE_BLE_EVT) ?
                    xSemaphoreGive(semaphore_app) :
                    xSemaphoreGive(semaphore_isr);
                    ble_app_main(&t_app_q.param.ble_app_req);
                }
                break;

                case APP_QUEUE_ZIGBEE_EVT:
                {
                    zb_app_main(t_app_q.param.zb_app_req);
                    xSemaphoreGive(semaphore_zb);
                }
                break;

                case APP_QUEUE_GW_CMD_PROC_EVT:
                {
                    zigbee_gateway_cmd_proc(((sys_tlv_t *)t_app_q.param.pt_data)->value, ((sys_tlv_t *)t_app_q.param.pt_data)->length);
                    vPortFree((sys_tlv_t *)t_app_q.param.pt_data);
                    xSemaphoreGive(semaphore_zb);
                }
                break;

                case APP_QUEUE_ZB_MSG_EVT:
                {
                    uint8_t cmd_len;
                    gateway_cmd *p_cmd = (gateway_cmd *)t_app_q.param.pt_data;

                    xSemaphoreGive(semaphore_zb);
                    ble_notify_zb_event(t_app_q.param.pt_data, t_app_q.data_len);

                    cmd_len = p_cmd->hdr.len;
                    cmd_len -= sizeof(gateway_cmd_pd);

                    gw_event_check(p_cmd->pd.command_id, p_cmd->pd.address, cmd_len, t_app_q.param.pt_data + GW_CMD_HEADER_LEN);
                    vPortFree((sys_tlv_t *)t_app_q.param.pt_data);

                }
                break;

                }

            }
            break;

            case QUEUE_TYPE_ZIGBEE:
            {
                zb_event_parse((sys_tlv_t *)t_app_q.param.pt_data);
                xSemaphoreGive(semaphore_zb);
            }
            break;

            case QUEUE_TYPE_BLE:
            {
                ble_event_parse((ble_tlv_t *)t_app_q.param.pt_data);
                xSemaphoreGive(semaphore_cb);
            }
            break;

            default:
                break;
            }
        }
    }
}


/**************************************************************************************************
 *    GLOBAL FUNCTIONS
 *************************************************************************************************/

void app_init(void)
{
    zb_cmd_handler_parm_t zb_cmd_handler_param = {0};
    /* Initil LED, Button, Console or UART */
    bsp_init(BSP_INIT_DEBUG_CONSOLE, NULL);

    /* Retarget stdout for utility & initial utility logging */
    utility_register_stdout(bsp_console_stdout_char, bsp_console_stdout_string);
    util_log_init();

    //util_log_on(UTIL_LOG_PROTOCOL);

    // Zigbee Stack init
    gt_zb_app_cfg.p_zigbee_device_contex_t = &simple_desc_gateway_ctx;
    gt_zb_app_cfg.pf_evt_indication = zb_app_evt_indication_cb;
    zigbee_stack_init(&gt_zb_app_cfg);

    // BLE Stack init
    gt_ble_app_cfg.pf_evt_indication = ble_app_evt_indication_cb;
    ble_host_stack_init(&gt_ble_app_cfg);

    info_color(LOG_BLUE, "Initial ZigBee/BLE stack\n");

    g_app_msg_q = xQueueCreate(BLE_APP_QUEUE_SIZE + ZB_APP_QUEUE_SIZE, sizeof(multi_app_queue_t));

    semaphore_cb = xSemaphoreCreateCounting(BLE_APP_CB_QUEUE_SIZE, BLE_APP_CB_QUEUE_SIZE);
    semaphore_isr = xSemaphoreCreateCounting(APP_ISR_QUEUE_SIZE, APP_ISR_QUEUE_SIZE);
    semaphore_app = xSemaphoreCreateCounting(APP_REQ_QUEUE_SIZE, APP_REQ_QUEUE_SIZE);

    semaphore_zb = xSemaphoreCreateCounting(ZB_APP_QUEUE_SIZE, ZB_APP_QUEUE_SIZE);
    info("Create app task\n");
    xTaskCreate((TaskFunction_t)app_main_task, "app", APP_STACK_SIZE, NULL, TASK_PRIORITY_APP, NULL);

    zb_cmd_handler_param.ZbCmdParserCB[0] = zigbee_gateway_cmd_parser;
    zb_cmd_handler_param.ZbCmdParserCB[1] = xmodem_parser;
    zb_cmd_handler_param.ZbCmdParserCB[2] = NULL;
    zb_cmd_handler_param.ZbCmdRecvCB[0] = zigbee_gateway_cmd_recv;
    zb_cmd_handler_param.ZbCmdRecvCB[1] = xmodem_recv;
    zb_cmd_handler_param.ZbCmdRecvCB[2] = NULL;
    zb_cmd_handler_init(&zb_cmd_handler_param);

}


