// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#ifndef __ZIGBEE_APP_H__
#define __ZIGBEE_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zigbee_stack_api.h"

#define BUTTON_1_EP                 0x01
#define BUTTON_2_EP                 0x02
#define BUTTON_3_EP                 0x03

#define ZB_NWK_NOT_FOUND_INDICATOR (BSP_LED_1)
#define ZB_NWK_FINDING_INDICATOR   (BSP_LED_0)

#define ZB_APP_QUEUE_SIZE           8
#define ZB_BTN_QUEUE_SIZE           5
#define ZB_APP_TRY_JOIN_CNT         3
#define ZB_APP_TRY_REJOIN_CNT       5

typedef enum
{
    APP_INIT_EVT    = 0,
    APP_NOT_JOINED_EVT,
    APP_ZB_JOINED_EVT,
    APP_START_REJOIN_EVT,
    APP_IDLE_EVT,
    APP_ZB_START_EVT,
    APP_ZB_RESET_EVT,
    APP_LED_TOGGLE_EVT,
    APP_LED_LEVEL_UP_EVT,
    APP_LED_LEVEL_DOWN_EVT,
    APP_LED_MOVE_TEMP_EVT,
} app_main_evt_t;

typedef enum
{
    APP_ZB_JOIN    = 0,
    APP_ZB_REJOIN,
} app_join_info_t;


extern zb_af_device_ctx_t simple_desc_switch_ctx;
extern SemaphoreHandle_t semaphore_zb;
extern SemaphoreHandle_t semaphore_btn;
extern void zigbee_rejoin_request(void);

void send_toggle(void);
void send_on(void);
void send_off(void);

void send_level_step(uint8_t dir);
void send_move_color(void);

void zb_app_evt_indication_cb(uint32_t data_len);
void zb_event_parse(sys_tlv_t *pt_zb_tlv);
uint8_t zigbee_app_evt_change(uint32_t evt, uint8_t from_isr);
uint8_t zigbee_app_join_continue(uint8_t join_type);
void zigbee_app_join_info_reset(uint8_t try_join);
void zb_app_main(uint32_t event);

#ifdef __cplusplus
};
#endif
#endif /* __ZIGBEE_APP_H__ */
