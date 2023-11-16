/** @file bsp_button.c
 *
 * @license
 * @description
 */

#include <stdint.h>
#include <stdio.h>
#include "bsp.h"
#include "cm3_mcu.h"
#include "bsp_button.h"
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
static bsp_event_callback_t m_callback;
static bsp_button_cfg_t m_button_cfg[BSP_BUTTON_COUNT] =
{
#ifdef BSP_BUTTON_0
    {BSP_BUTTON_0, GPIO_PIN_INT_EDGE_FALLING},
#endif
#ifdef BSP_BUTTON_1
    {BSP_BUTTON_1, GPIO_PIN_INT_EDGE_FALLING},
#endif
#ifdef BSP_BUTTON_2
    {BSP_BUTTON_2, GPIO_PIN_INT_EDGE_FALLING},
#endif
#ifdef BSP_BUTTON_3
    {BSP_BUTTON_3, GPIO_PIN_INT_EDGE_FALLING},
#endif
#ifdef BSP_BUTTON_4
    {BSP_BUTTON_4, GPIO_PIN_INT_EDGE_FALLING},
#endif
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void bsp_button_isr_event(uint32_t pin, void *isr_param)
{
    //      printf("bsp_button_isr_event, pin=%d \r\n", pin);
    if (m_callback == NULL)
    {
        return ;
    }
    if (pin == BSP_BUTTON_0)
    {
        m_callback(BSP_EVENT_BUTTONS_0);
    }
    else if (pin == BSP_BUTTON_1)
    {
        m_callback(BSP_EVENT_BUTTONS_1);
    }
    else if (pin == BSP_BUTTON_2)
    {
        m_callback(BSP_EVENT_BUTTONS_2);
    }
    else if (pin == BSP_BUTTON_3)
    {
        m_callback(BSP_EVENT_BUTTONS_3);
    }
#ifdef BSP_BUTTON_4
    else if (pin == BSP_BUTTON_4)
    {
        m_callback(BSP_EVENT_BUTTONS_4);
    }
#endif
    else if (pin == BSP_EINT_GPIO_9)
    {
        m_callback(BSP_EVENT_GPIO_EINT);
    }

}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int bsp_button_init(bsp_event_callback_t callback)
{
    int     rval = 0;
    int i;
    uint8_t inputgpio = 9;
    uint8_t outputgpio = 15;

    for (i = 0; i < BSP_BUTTON_COUNT; i++)
    {
        gpio_cfg_input((uint32_t)m_button_cfg[i].pin_no, (gpio_pin_int_mode_t)m_button_cfg[i].active_state);
        gpio_register_isr(m_button_cfg[i].pin_no, bsp_button_isr_event, NULL);
        gpio_debounce_enable(m_button_cfg[i].pin_no);
        gpio_int_enable(m_button_cfg[i].pin_no);
    }

    //input config
    //pin_set_pullopt(inputgpio, MODE_PULLUP_100K);   /*set the pin to default pull high 100K mode*/
    //gpio_cfg_input(i, GPIO_PIN_INT_LEVEL_LOW);     /*if you using interrupt low level trigger, please don't enable dobounce for this pin*/
    gpio_cfg_input(inputgpio, GPIO_PIN_INT_EDGE_FALLING);
    gpio_register_isr(inputgpio, bsp_button_isr_event, NULL);
    gpio_debounce_enable(inputgpio);
    gpio_int_enable(inputgpio);    /*enable each gpio pin interrupt*/


    NVIC_SetPriority(Gpio_IRQn, 7);
    m_callback = callback;

    //output GPIO-15 config
    //      pin_set_pullopt(outputgpio, PULL_UP_10K);
    gpio_cfg_output(outputgpio);

    //      gpio_pin_clear(outputgpio);  //set GPIO-15 output low
    //    while(i--);  //delay for a while
    /*default output high.*/
    gpio_pin_set(outputgpio);  //set GPIO-15 output high

    return rval;
}

int bsp_button_state_get(uint32_t button)
{
    return gpio_pin_get(button);
}
