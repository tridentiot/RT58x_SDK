// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#ifndef __ZIGBEE_APP_H__
#define __ZIGBEE_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zigbee_stack_api.h"

#define SCENE_TABLE_SIZE 16
#define GROUP_TABLE_SIZE 16
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

typedef struct SCENE_ENTRY
{
    uint8_t occupied;
    uint8_t scene_id;
    uint8_t system_mode;
    int16_t occupied_cooling_setpoint;
    int16_t occupied_heating_setpoint;
    uint16_t group_id;
    uint16_t scene_trans_time;
} scene_entry_t;

typedef struct SCENE_DB
{
    scene_entry_t scene_table[SCENE_TABLE_SIZE];
    uint16_t group_table[GROUP_TABLE_SIZE];
} scene_db_t;
extern zb_af_device_ctx_t simple_desc_tstat_ctx;

void zigbee_app_init(void);
void zigbee_app_evt_change(uint32_t evt);


extern scene_db_t scene_table_db;
void scene_db_check(void);
void scene_db_update(void);
uint32_t get_identify_time(void);
void set_scene_count(uint8_t scene_count);
int16_t get_cooling_setpoint(void);
int16_t get_heating_setpoint(void);
uint8_t get_system_mode (void);
void set_system_mode(uint8_t mode);
void set_cooling_setpoint(int16_t setpoint);
void set_heating_setpoint(int16_t setpoint);
void reset_attr(void);
#ifdef __cplusplus
};
#endif
#endif /* __ZIGBEE_APP_H__ */
