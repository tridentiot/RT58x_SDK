#ifndef __BLE_APP_H__
#define __BLE_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *    INCLUDES
 *************************************************************************************************/
#include <stdint.h>
#include "ble_api.h"

/**************************************************************************************************
 *    CONSTANTS AND DEFINES
 *************************************************************************************************/

#define APP_TRSP_P_HOST_ID              0
#define BLE_APP_CB_QUEUE_SIZE           16
#define APP_ISR_QUEUE_SIZE              2
#define APP_REQ_QUEUE_SIZE              6
#define BLE_APP_QUEUE_SIZE             (BLE_APP_CB_QUEUE_SIZE + APP_ISR_QUEUE_SIZE + APP_REQ_QUEUE_SIZE)

extern xQueueHandle g_app_msg_q;
extern SemaphoreHandle_t semaphore_cb;
extern SemaphoreHandle_t semaphore_isr;
extern SemaphoreHandle_t semaphore_app;
extern TimerHandle_t  g_fota_timer;

/**************************************************************************************************
 *    TYPEDEFS
 *************************************************************************************************/

typedef enum
{
    APP_REQUEST_IDLE,             /**< Application request event: idle.*/
    APP_REQUEST_ADV_START,        /**< Application request event: advertising start.*/
    APP_REQUEST_TRSPS_DATA_SEND,  /**< Application request event: TRSP server data send.*/
    APP_REQUEST_ADV_STOP,         /**< Application request event: advertising stop.*/
    APP_REQUEST_FOTA_TIMER_EXPIRY /**< Application request event: FOTA timer expired.*/
} app_request_t;

typedef enum
{
    STATE_STANDBY,        /**< Application state: standby.*/
    STATE_ADVERTISING,    /**< Application state: advertising.*/
    STATE_CONNECTED,      /**< Application state: connected.*/
} ble_state_t;


typedef struct
{
    uint8_t   host_id;    /**< Application request parameter: host id.*/
    uint16_t  app_req;    /**< Application request parameter: @ref app_request_t "application request event".*/
} app_req_param_t;


/**************************************************************************************************
 *    PUBLIC FUNCTIONS
 *************************************************************************************************/

void ble_app_evt_indication_cb(uint32_t data_len);

void ble_event_parse(ble_tlv_t *pt_ble_tlv);

void ble_app_main(app_req_param_t *p_param);

void ble_device_rename(uint8_t *p_new_device_name);

ble_err_t ble_init(void);

bool app_request_set(uint8_t host_id, app_request_t request, bool from_isr);

void fota_timer_handler( TimerHandle_t timer);

#ifdef __cplusplus
};
#endif

#endif /* __BLE_APP_H__*/
