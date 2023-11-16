// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#ifndef __MULTI_APP_H__
#define __MULTI_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zigbee_stack_api.h"
#include "ble_api.h"
#include "ble_app.h"

#define APP_STACK_SIZE                  256

typedef enum
{
    APP_QUEUE_ZIGBEE_EVT    = 0,
    APP_QUEUE_BLE_EVT,
    APP_QUEUE_ISR_BLE_EVT,
    APP_QUEUE_ISR_BUTTON_EVT,
} app_queue_evt_t;



typedef struct
{
    uint8_t  param_type;
    uint8_t  event;
    uint8_t  pin;
    uint8_t  reserved;
    union
    {
        app_req_param_t  ble_app_req;
        uint32_t  zb_app_req;
        ble_tlv_t *pt_ble_tlv;
        sys_tlv_t *pt_zb_tlv;
    } param;
} multi_app_queue_t;

typedef enum
{
    QUEUE_TYPE_APP_REQ,
    QUEUE_TYPE_BLE,
    QUEUE_TYPE_ZIGBEE,
    QUEUE_TYPE_OTHERS,
} app_queue_param_type;



void app_init(void);
extern uint32_t gu32_join_success;
#ifdef __cplusplus
};
#endif
#endif /* __MULTI_APP_H__ */
