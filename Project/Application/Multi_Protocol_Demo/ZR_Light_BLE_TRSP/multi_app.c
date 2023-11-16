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

#include "multi_app.h"

#include "ble_api.h"
#include "ble_app.h"

#include "zigbee_lib_api.h"
#include "zigbee_app.h"

#include "bsp_console.h"
#include "bsp_led.h"
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
#define TIME_TO_STOP_ADV_100MS     (600)
/**************************************************************************************************
 *    GLOBAL VARIABLES
 *************************************************************************************************/
xQueueHandle g_app_msg_q;

static zigbee_cfg_t gt_zb_app_cfg;
static ble_cfg_t gt_ble_app_cfg;

static TimerHandle_t tmr_100ms;
uint32_t gu32_timer_100ms_cnt = ~0;
uint32_t gu32_join_state = 0;

/**************************************************************************************************
 *    EXTERNS
 *************************************************************************************************/

/**************************************************************************************************
 *    LOCAL FUNCTIONS
 *************************************************************************************************/
void tmr_100ms_stop(void)
{
    xTimerStop(tmr_100ms, 0);
}
void tmr_100ms_cb(TimerHandle_t t_timer)
{
    if (gu32_join_state)
    {
        bsp_led_Off(BSP_LED_1);
    }
    else
    {
        bsp_led_toggle(BSP_LED_1);
    }

    if (gu32_timer_100ms_cnt != ~0)
    {
        gu32_timer_100ms_cnt++;
        if (gu32_timer_100ms_cnt == TIME_TO_STOP_ADV_100MS)
        {
            app_request_set(APP_TRSP_P_HOST_ID, APP_REQUEST_ADV_STOP, false);
            gu32_timer_100ms_cnt = ~0;
        }
    }
}

static void app_bsp_event_handle(bsp_event_t event)
{
    multi_app_queue_t t_app_q;
    BaseType_t context_switch = pdFALSE;
    switch (event)
    {

    case BSP_EVENT_BUTTONS_0:
    case BSP_EVENT_BUTTONS_1:
    case BSP_EVENT_BUTTONS_2:
    case BSP_EVENT_BUTTONS_3:
    case BSP_EVENT_BUTTONS_4:
        if (xSemaphoreTakeFromISR(semaphore_zb, 0) == pdPASS)
        {
            t_app_q.event = APP_QUEUE_ISR_ZIGBEE_EVT;
            t_app_q.pin = event;
            xQueueSendToBackFromISR(g_app_msg_q, &t_app_q, &context_switch);
        }
        break;

    default:
        break;
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
    zigbee_app_evt_change(APP_INIT_EVT, false);
    zigbee_app_evt_change(APP_ZB_MAC_ADDR_GET_EVT, false);
    zigbee_app_evt_change(APP_ZB_IS_FACTORY_NEW_EVT, false);

    // start adv
    //app_request_set(APP_TRSP_P_HOST_ID, APP_REQUEST_ADV_START, false);

    tmr_100ms = xTimerCreate("100ms", pdMS_TO_TICKS(100), pdTRUE, (void *)0, tmr_100ms_cb);
    xTimerStart(tmr_100ms, 0);

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

                case APP_QUEUE_BLE_EVT:
                case APP_QUEUE_ISR_BLE_EVT:
                {
                    (t_app_q.event == APP_QUEUE_BLE_EVT) ?
                    xSemaphoreGive(semaphore_app) :
                    xSemaphoreGive(semaphore_isr);
                    ble_app_main(&t_app_q.param.ble_app_req);
                }
                break;

                case APP_QUEUE_ISR_ZIGBEE_EVT:
                {
                    xSemaphoreGive(semaphore_zb);
                    if (t_app_q.pin == BSP_EVENT_BUTTONS_3)
                    {
                        zigbee_send_permit_join(short_addr, 180);
                        zigbee_send_permit_join(0xfffc, 180);
                    }
                    if (t_app_q.pin == BSP_EVENT_BUTTONS_4)
                    {
                        zigbee_finding_binding_req(LIGHT_EP, ZB_FB_TARGET_ROLE);
                    }
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
                zb_event_parse((sys_tlv_t *)t_app_q.param.pt_data);
            }
            break;

            case QUEUE_TYPE_BLE:
            {
                xSemaphoreGive(semaphore_cb);
                ble_event_parse((ble_tlv_t *)t_app_q.param.pt_data);
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

    //util_log_on(UTIL_LOG_PROTOCOL);

    info_color(LOG_DEFAULT, "---------------------------------------------\n");
    info_color(LOG_DEFAULT, "  ZigBee light + BLE TRSP (P) demo: start...\n");
    info_color(LOG_DEFAULT, "---------------------------------------------\n");

    // Zigbee Stack init
    gt_zb_app_cfg.p_zigbee_device_contex_t = &simple_desc_light_ctx;
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

    g_app_msg_q = xQueueCreate(BLE_APP_QUEUE_SIZE + ZB_APP_QUEUE_SIZE, sizeof(multi_app_queue_t));
    // application SW timer, tick = 1s
    g_fota_timer = xTimerCreate("FOTA_Timer", pdMS_TO_TICKS(1000), pdTRUE, ( void * ) 0, fota_timer_handler);

    info("Create app task\n");
    xTaskCreate((TaskFunction_t)app_main_task, "app", APP_STACK_SIZE, NULL, TASK_PRIORITY_APP, NULL);

    file_system_init();
}


