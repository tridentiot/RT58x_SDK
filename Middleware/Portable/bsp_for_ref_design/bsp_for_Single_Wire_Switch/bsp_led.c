/** @file bsp_button.c
 *
 * @license
 * @description
 */

#include <stdint.h>
#include "cm3_mcu.h"
#include "bsp.h"

#include "bsp_led.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//==============================================================================
//static bsp_event_callback_t m_callback;
static bsp_led_cfg_t m_led_cfg[3] =
{
#ifdef BSP_LED_0
    {BSP_LED_0},
#endif
#ifdef BSP_LED_1
    {BSP_LED_1},
#endif
#ifdef BSP_LED_2
    {BSP_LED_2},
#endif


};
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int bsp_led_init(bsp_event_callback_t callback)
{
    int rval = 0;
    int i;
    for (i = 0; i < BSP_LED_COUNT; i++)
    {
        gpio_cfg_output(m_led_cfg[i].pin_no);
        bsp_led_Off(m_led_cfg[i].pin_no);
    }

    // default: DRV_20MA
    //-----------------------------------
    //pin_set_drvopt(5,DRV_4MA);
    //pin_set_drvopt(6,DRV_4MA);
    //pin_set_drvopt(28,DRV_4MA);
    //pin_set_drvopt(29,DRV_4MA);
    //pin_set_drvopt(30,DRV_4MA);
    //pin_set_drvopt(31,DRV_4MA);

    //m_callback = callback;
    //gpio_cfg_output(5);
    //gpio_cfg_output(6);

    //gpio_cfg_output(28);
    //gpio_cfg_output(29);

    //gpio_cfg_output(30);
    //gpio_cfg_output(31);

    //gpio_pin_write(5,0);
    //gpio_pin_write(6,0);
    //gpio_pin_write(28,0);
    //gpio_pin_write(29,0);
    //gpio_pin_write(30,0);
    //gpio_pin_write(31,0);


    //gpio_pin_write(5,0);  //low
    //gpio_pin_write(6,1);  //high
    //Delay_ms(20);
    //gpio_pin_write(6,0);  //high

    //gpio_pin_write(28,0);  //low
    //gpio_pin_write(29,1);  //high
    //Delay_ms(20);
    //gpio_pin_write(29,0);  //high

    //gpio_pin_write(30,0);  //low
    //gpio_pin_write(31,1);  //high
    //Delay_ms(20);
    //gpio_pin_write(31,0);  //high

    return rval;
}
void bsp_led_on(uint32_t led)
{
    gpio_pin_write(led, BSP_LED_ACTIVE_STATE);
}
void bsp_led_Off(uint32_t led)
{
    gpio_pin_write(led, !BSP_LED_ACTIVE_STATE);
}
void bsp_led_toggle(uint32_t led)
{
    gpio_pin_toggle(led);
}
