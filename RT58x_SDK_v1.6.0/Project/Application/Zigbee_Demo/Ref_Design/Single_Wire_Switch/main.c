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
#include "task_pci.h"
#include "zigbee_app.h"

uint8_t g_8005_reset_flag = 0;
uint8_t gcount = 0;
extern void zigbee_uart_init(void);
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

/**************************************************************************************************
 *    LOCAL FUNCTIONS
 *************************************************************************************************/

/**************************************************************************************************
 *    GLOBAL FUNCTIONS
 *************************************************************************************************/
/*
* this function is ISR. so it should be as short as possible
* You SHOULD NOT call any function that will block the ISR
*
*/

void user_gpio7_isr_handler(uint32_t pin, void *isr_param)
{
    /*
     * This is a very STUPID idea --- call printf in ISR.
     * Here we use it just for showing debug information easily...
     * In normal code, you SHOULD AVOID this poor design...
     */

    printf("pin %ld\n", pin);
    g_8005_reset_flag = 1;

    /*we also use gpio 31 to show interrupt happen*/
    //gpio_pin_toggle(8);
    gpio_pin_write(8, 0); //low
    return;
}

/*this is pin mux setting*/
static void init_default_pin_mux(void)
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

void  plug_gpio_init(void)
{
    gpio_cfg_output(5);
    gpio_cfg_output(6);

    gpio_cfg_output(28);
    gpio_cfg_output(29);

    gpio_cfg_output(30);
    gpio_cfg_output(31);

    //gpio_cfg_output(7);
    //gpio_cfg_output(8);
    //gpio_cfg_output(9);
    //gpio_cfg_output(10);
    //gpio_cfg_output(14);
    //gpio_cfg_output(15);

    gpio_pin_write(5, 0);
    gpio_pin_write(6, 0);
    gpio_pin_write(28, 0);
    gpio_pin_write(29, 0);
    gpio_pin_write(30, 0);
    gpio_pin_write(31, 0);
    //Delay_ms(5);
    gpio_set_debounce_time(DEBOUNCE_SLOWCLOCKS_1024);
    //pin_set_pullopt(7, PULL_NONE);
    //pin_set_pullopt(7,MODE_PULLDOWN_100K);
    //gpio_cfg_input(7, GPIO_PIN_INT_EDGE_RISING);
    //gpio_debounce_enable(7);
    //gpio_register_isr( 7, user_gpio7_isr_handler, NULL);
    //gpio_int_enable(7);
    gpio_cfg_output(8);
    gpio_pin_write(8, 0); //low

}
int main(void)
{
    init_default_pin_mux();
    plug_gpio_init();
    zigbee_uart_init();
    //bsp_init((BSP_INIT_DEBUG_CONSOLE | BSP_INIT_LEDS), NULL);
    //gpio_pin_write(7,1);  //low
    /*we should set pinmux here or in SystemInit */
    //init_default_pin_mux();
    //gpio_pin_write(7,0);  //hight

    Lpm_Set_Low_Power_Level(LOW_POWER_LEVEL_SLEEP0);
    Lpm_Enable_Low_Power_Wakeup(LOW_POWER_WAKEUP_GPIO);
    Lpm_Enable_Low_Power_Wakeup(LOW_POWER_WAKEUP_32K_TIMER);

    task_pci_init();
    zigbee_app_init();

    /* Start the scheduler. */
    //gpio_pin_write(7,1);  //hight
    vTaskStartScheduler();
    while (1)
    {
    }
}

