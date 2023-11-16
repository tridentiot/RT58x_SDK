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

#include "FreeRTOS.h"
#include "task.h"

#include "ble_api.h"
#include "zigbee_app.h"
#include "ble_app.h"
#include "multi_app.h"

#include "bsp_console.h"
#include "bsp_led.h"
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

/**************************************************************************************************
 *    EXTERNS
 *************************************************************************************************/

/**************************************************************************************************
 *    LOCAL FUNCTIONS
 *************************************************************************************************/

static void app_bsp_event_handle(bsp_event_t event)
{
    multi_app_queue_t t_app_q;
    BaseType_t context_switch = pdFALSE;

    switch (event)
    {
    case BSP_EVENT_UART_BREAK:
        info_color(LOG_RED, "Recv BREAK!!\n");
        break;

    case BSP_EVENT_BUTTONS_0:
    case BSP_EVENT_BUTTONS_1:
    case BSP_EVENT_BUTTONS_2:
    case BSP_EVENT_BUTTONS_3:
    case BSP_EVENT_BUTTONS_4:
        if (xSemaphoreTakeFromISR(semaphore_btn, 0) == pdPASS)
        {
            t_app_q.param_type = QUEUE_TYPE_APP_REQ;
            t_app_q.event = APP_QUEUE_ISR_BUTTON_EVT;
            t_app_q.pin = event;
            xQueueSendToBackFromISR(g_app_msg_q, &t_app_q, &context_switch);
        }
        break;

    default:
        break;
    }
}

static void app_pin_parse(uint8_t pin)
{
    if (pin == BSP_EVENT_BUTTONS_0)
    {
        send_toggle();
    }
    else if (pin == BSP_EVENT_BUTTONS_1)
    {
        send_level_step(0);
    }
    else if (pin == BSP_EVENT_BUTTONS_2)
    {
        send_level_step(1);
    }
    else if (pin == BSP_EVENT_BUTTONS_3)
    {
        send_move_color();
    }
    else if (pin == BSP_EVENT_BUTTONS_4)
    {
        zigbee_app_join_info_reset(TRUE);
    }
}
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

    //zigbee application init
    zigbee_app_evt_change(APP_INIT_EVT, FALSE);
    // start adv
    app_request_set(APP_TRSP_P_HOST_ID, APP_REQUEST_ADV_START, false);

    bsp_led_on(ZB_NWK_NOT_FOUND_INDICATOR);
    bsp_led_on(ZB_NWK_FINDING_INDICATOR);
    for (;;)
    {
        if (xQueueReceive(g_app_msg_q, &t_app_q, portMAX_DELAY) == pdTRUE)
        {
            switch (t_app_q.param_type)
            {
            case QUEUE_TYPE_APP_REQ:
            {
                switch (t_app_q.event)
                {
                case APP_QUEUE_ISR_BUTTON_EVT:
                {
                    info_color(LOG_YELLOW, "button %d press\n", t_app_q.pin);
                    xSemaphoreGive(semaphore_btn);
                    app_pin_parse(t_app_q.pin);
                }
                break;

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
                    xSemaphoreGive(semaphore_zb);
                    zb_app_main(t_app_q.param.zb_app_req);
                }
                break;
                }

            }
            break;

            case QUEUE_TYPE_ZIGBEE:
            {
                xSemaphoreGive(semaphore_zb);
                zb_event_parse(t_app_q.param.pt_zb_tlv);
            }
            break;

            case QUEUE_TYPE_BLE:
            {
                xSemaphoreGive(semaphore_cb);
                ble_event_parse(t_app_q.param.pt_ble_tlv);
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

    /* Initil LED, Button, Console or UART */
    bsp_init((BSP_INIT_LEDS |
              BSP_INIT_BUTTONS |
              BSP_INIT_DEBUG_CONSOLE), app_bsp_event_handle);

    /* Retarget stdout for utility & initial utility logging */
    utility_register_stdout(bsp_console_stdout_char, bsp_console_stdout_string);
    util_log_init();

    info_color(LOG_DEFAULT, "---------------------------------------------\n");
    info_color(LOG_DEFAULT, "  ZigBee Switch + BLE TRSP (P) demo: start...\n");
    info_color(LOG_DEFAULT, "---------------------------------------------\n");

    gt_zb_app_cfg.p_zigbee_device_contex_t = &simple_desc_switch_ctx;
    gt_zb_app_cfg.pf_evt_indication = zb_app_evt_indication_cb;

    zigbee_stack_init(&gt_zb_app_cfg);

    // BLE Stack init
    gt_ble_app_cfg.pf_evt_indication = ble_app_evt_indication_cb;
    ble_host_stack_init(&gt_ble_app_cfg);

    info_color(LOG_BLUE, "Initial ZigBee/BLE stack\n");

    semaphore_cb = xSemaphoreCreateCounting(BLE_APP_CB_QUEUE_SIZE, BLE_APP_CB_QUEUE_SIZE);
    semaphore_isr = xSemaphoreCreateCounting(APP_ISR_QUEUE_SIZE, APP_ISR_QUEUE_SIZE);
    semaphore_app = xSemaphoreCreateCounting(APP_REQ_QUEUE_SIZE, APP_REQ_QUEUE_SIZE);
    semaphore_zb = xSemaphoreCreateCounting(ZB_APP_QUEUE_SIZE, ZB_APP_QUEUE_SIZE);
    semaphore_btn = xSemaphoreCreateCounting(ZB_BTN_QUEUE_SIZE, ZB_BTN_QUEUE_SIZE);

    g_app_msg_q = xQueueCreate(BLE_APP_QUEUE_SIZE + ZB_APP_QUEUE_SIZE + ZB_BTN_QUEUE_SIZE, sizeof(multi_app_queue_t));

    info("Create app task\n");
    xTaskCreate((TaskFunction_t)app_main_task, "app", APP_STACK_SIZE, NULL, TASK_PRIORITY_APP, NULL);
}


