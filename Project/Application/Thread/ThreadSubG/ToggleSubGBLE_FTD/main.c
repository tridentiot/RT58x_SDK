#include "openthread-core-RT58x-config.h"
#include <openthread-core-config.h>
#include <openthread/config.h>
#include <openthread/thread.h>

#include "openthread-system.h"
#include "cm3_mcu.h"
#include "rfb.h"
#include "bsp.h"
#include "bsp_button.h"
#include "app.h"
#include "app_uart_handler.h"
#include "ble_app.h"
#include "thread_app.h"
#include "rtc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uart_stdio.h"
/* Utility Library APIs */
#include "util_log.h"
#include "util_printf.h"
#if OPENTHREAD_CONFIG_RADIO_915MHZ_OQPSK_SUPPORT
#define RFB_DATA_RATE FSK_300K // Supported Value: [FSK_50K; FSK_100K; FSK_150K; FSK_200K; FSK_300K]
extern void rafael_radio_subg_datarate_set(uint8_t datarate);
#endif

#define RFB_CCA_THRESHOLD 75 // Default: 75 (-75 dBm)
#define RTC_WAKE_UP_INTERVEL 3 //minutes

extern void rafael_radio_cca_threshold_set(uint8_t datarate);

static void gpio6_handler();

static void rtc_arm_isr(uint32_t rtc_status);

/* pin mux setting init*/
static void pin_mux_init(void)
{
    int i;

    /*set all pin to gpio, except GPIO16, GPIO17 */
    for (i = 0; i < 32; i++)
    {
        if ((i != 16) && (i != 17))
        {
            pin_set_mode(i, MODE_GPIO);
        }
    }
    return;
}

static void bsp_btn_event_handle(bsp_event_t event)
{
    switch (event)
    {
    case BSP_EVENT_BUTTONS_0:
        app_commission_erase();
        NVIC_SystemReset();
        break;
    case BSP_EVENT_BUTTONS_1:
        //send sensor data
        if (app_commission_data_check() == true)
        {
            thread_app_sensor_data_generate(APP_SENSOR_CONTROL_EVENT);
        }
        else
        {
            app_commission_erase();
            NVIC_SystemReset();
        }
        break;
    default:
        break;
    }
}

void bsp_uart0_isr_event_handle(bsp_event_t event)
{
    otSysEventSignalPending();
}

void bsp_uart1_isr_event_handle(bsp_event_t event)
{
    otSysEventSignalPending();
}

int main()
{
#if OPENTHREAD_CONFIG_RADIO_915MHZ_OQPSK_SUPPORT
    rafael_radio_subg_datarate_set(RFB_DATA_RATE);
#endif
    rafael_radio_cca_threshold_set(RFB_CCA_THRESHOLD);

    /* pinmux init */
    pin_mux_init();

    /* led init */
    gpio_cfg_output(20);
    gpio_cfg_output(21);
    gpio_cfg_output(22);
    gpio_pin_write(20, 1);
    gpio_pin_write(21, 1);
    gpio_pin_write(22, 1);

    /*uart 0 init*/
    uart_stdio_init(otSysEventSignalPending);
    utility_register_stdout(uart_stdio_write_ch, uart_stdio_write);
    util_log_init();

    bsp_init(BSP_INIT_BUTTONS, bsp_btn_event_handle);

    app_commission_t commission;

    app_commission_get(&commission);

    if (commission.started == 0xFF)
    {
        ble_app_init();
    }
    else if (commission.started == 0x01)
    {
        if (app_commission_data_check() == true)
        {
            thread_app_task_start();
        }
        else
        {
            app_commission_erase();
            NVIC_SystemReset();
        }
    }
    else
    {
        app_commission_erase();
        NVIC_SystemReset();
    }
    uart1_task_start();
    vTaskStartScheduler();

    while (1)
    {
    }
    return 0;
}
