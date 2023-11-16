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

//=============================================================================
//                Public Definitions of const value
//=============================================================================
#define APP_STACK_SIZE                  512
//=============================================================================
//                Public ENUM
//=============================================================================
typedef enum
{
    APP_QUEUE_ZIGBEE_EVT    = 0,
    APP_QUEUE_BLE_EVT,
    APP_QUEUE_ISR_BLE_EVT,
    APP_QUEUE_GW_CMD_PROC_EVT,
    APP_QUEUE_ZB_MSG_EVT,
} app_queue_evt_t;


typedef enum
{
    QUEUE_TYPE_APP_REQ,
    QUEUE_TYPE_BLE,
    QUEUE_TYPE_ZIGBEE,
    QUEUE_TYPE_OTHERS,
} app_queue_param_type;

//=============================================================================
//                Public Struct
//=============================================================================

typedef struct
{
    uint8_t event;
    uint8_t param_type;
    uint8_t data_len;
    uint8_t reserved;
    union
    {
        app_req_param_t  ble_app_req;
        uint32_t  zb_app_req;
        uint8_t *pt_data;
    } param;
} multi_app_queue_t;


//=============================================================================
//                Public Function Declaration
//=============================================================================

void app_init(void);

#ifdef __cplusplus
};
#endif
#endif /* __MULTI_APP_H__ */
