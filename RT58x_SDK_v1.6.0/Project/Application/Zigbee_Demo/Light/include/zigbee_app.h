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
#define LIGHT_EP 2
#define ZB_ZCL_SUPPORT_CLUSTER_OTA_UPGRADE
#define ZB_HA_ENABLE_OTA_UPGRADE_CLIENT
#define DEVICE_DB_START_ADDRESS 0xF0000
#define DEVICE_DB_END_ADDRESS 0xF2000
/*
0xF0000 - 0xF2000 device db(scene entry, group entry,global scene, start up entry)
0xF2000 - 0xF3000 check reset
0xF3000 - 0xF4000 reserved
*/
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
    DS_TYPE_SCENE = 0x01,
    DS_TYPE_STARTUP_ENTRY = 0x02,
} dataset_type_t;
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
    uint8_t onoff_stat;
    uint8_t level;
    uint8_t scene_trans_time_100ms;
    uint16_t group_id;
    uint16_t scene_trans_time;
} scene_entry_t;
typedef struct STARTUP_ENTRY
{
    uint8_t last_onoff_stat;
    uint8_t last_level;
    uint8_t startup_onoff;
    uint8_t start_up_current_level;
} startup_entry_t;
typedef struct DEVICE_DB
{
    scene_entry_t global_scene;
    scene_entry_t scene_table[SCENE_TABLE_SIZE];
    uint16_t group_table[GROUP_TABLE_SIZE];
} scene_db_t;
extern zb_af_device_ctx_t simple_desc_light_ctx;
extern pwm_seq_para_head_t pwm_para_config[2];

void zigbee_app_init(void);
void zigbee_app_evt_change(uint32_t evt);
void set_duty_cycle(pwm_seq_para_head_t *pwm_para_config, uint8_t current_lv);


extern scene_db_t scene_table_db;
extern startup_entry_t startup_db;
extern uint16_t short_addr;

uint32_t get_on_off_status(void);
void set_on_off_status(uint32_t status);
uint32_t get_current_level(void);
void set_current_level(uint32_t level);
uint32_t get_identify_time(void);
uint32_t get_on_off_on_time(void);
void set_on_off_on_time(uint16_t time);
uint32_t get_on_off_off_wait_time(void);
void set_on_off_off_wait_time(uint16_t time);
uint8_t get_level_option(void);
uint32_t get_startup_onoff(void);
void set_startup_onoff(uint32_t status);
uint32_t get_start_up_current_level(void);
void set_start_up_current_level(uint32_t status);

void reset_attr(void);
void set_scene_count(uint8_t scene_count);
void scene_db_check(void);
void scene_db_update(void);
void startup_db_check(void);
void startup_db_update(void);

void z_get_upgrade_server_addr(uint8_t *in);
uint32_t z_get_file_version(void);
void z_set_file_version(uint32_t ver);
void z_set_OTA_status(uint8_t ver);
void z_set_OTA_downloaded_file_version(uint32_t ver);
void z_set_OTA_downloaded_file_offset(uint32_t ver);

#ifdef __cplusplus
};
#endif
#endif /* __ZIGBEE_APP_H__ */
