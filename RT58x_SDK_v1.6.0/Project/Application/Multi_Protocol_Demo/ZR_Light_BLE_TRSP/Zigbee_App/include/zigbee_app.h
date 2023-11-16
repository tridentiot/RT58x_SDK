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
#define ZB_APP_QUEUE_SIZE 8
#define LIGHT_EP 2
#define ZB_ZCL_SUPPORT_CLUSTER_OTA_UPGRADE
#define ZB_HA_ENABLE_OTA_UPGRADE_CLIENT

typedef enum
{
    APP_INIT_EVT    = 0,
    APP_NOT_JOINED_EVT,
    APP_IDLE_EVT,
    APP_ZB_START_EVT,
    APP_ZB_JOINED_EVT,
    APP_ZB_RESET_EVT,
    APP_ZB_LEAVE_EVT,
    APP_ZB_MAC_ADDR_GET_EVT,
    APP_ZB_IS_FACTORY_NEW_EVT,
} app_main_evt_t;


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
    uint16_t startup_onoff;
    uint16_t start_up_current_level;
} startup_entry_t;

typedef struct DEVICE_DB
{
    startup_entry_t startup_table;
    scene_entry_t global_scene;
    scene_entry_t scene_table[SCENE_TABLE_SIZE];
    uint8_t group_table[GROUP_TABLE_SIZE];
} device_db_t;

extern zb_af_device_ctx_t simple_desc_light_ctx;
extern SemaphoreHandle_t semaphore_zb;

extern pwm_seq_para_head_t pwm_para_config[2];

extern device_db_t device_table_db;
extern uint16_t short_addr;
extern uint32_t gu32_timer_100ms_cnt;
extern uint32_t gu32_join_state;

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

uint8_t zigbee_app_evt_change(uint32_t evt, uint8_t from_isr);
void set_duty_cycle(pwm_seq_para_head_t *pwm_para_config, uint8_t current_lv);

void zb_app_main(uint32_t event);
void zb_event_parse(sys_tlv_t *pt_zb_tlv);;
void zb_app_evt_indication_cb(uint32_t data_len);

uint32_t get_on_off_status(void);
void set_on_off_status(uint32_t status);
uint32_t get_current_level(void);
void set_current_level(uint32_t level);
uint32_t get_identify_time(void);
void reset_attr(void);
void set_scene_count(uint8_t scene_count);

void device_db_check(void);
void device_db_update(void);
#ifdef __cplusplus
};
#endif
#endif /* __ZIGBEE_APP_H__ */
