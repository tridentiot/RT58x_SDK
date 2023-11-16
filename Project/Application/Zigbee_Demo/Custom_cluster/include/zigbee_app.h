// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#ifndef __ZIGBEE_APP_H__
#define __ZIGBEE_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zigbee_stack_api.h"

#define UART_TRANSPARENT_CMD 0x66

typedef enum
{
    APP_INIT_EVT    = 0,
    APP_NOT_JOINED_EVT,
    APP_IDLE_EVT,
    APP_ZB_START_EVT,
    APP_ZB_JOINED_EVT,
    APP_ZB_RESET_EVT,
} app_main_evt_t;

typedef enum
{
    APP_QUEUE_ZIGBEE_EVT    = 0,
    APP_QUEUE_BLE_EVT,
    APP_QUEUE_ISR_BUTTON_EVT,
} app_queue_evt_t;

extern zb_af_device_ctx_t simple_desc_cc_ctx;

void zigbee_app_init(void);
void zigbee_app_evt_change(uint32_t evt);


uint32_t get_on_off_status(void);
void set_on_off_status(uint32_t status);
uint32_t get_current_level(void);
void set_current_level(uint32_t level);
#ifdef __cplusplus
};
#endif
#endif /* __ZIGBEE_APP_H__ */
