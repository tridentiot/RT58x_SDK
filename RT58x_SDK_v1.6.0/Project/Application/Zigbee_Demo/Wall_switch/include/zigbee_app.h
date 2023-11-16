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
#define WALL_SWITCH_EP     0x02

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
    uint8_t onoff_stat;
    uint8_t level;
    uint16_t group_id;
    uint16_t scene_trans_time;
} scene_entry_t;

typedef struct SCENE_DB
{
    scene_entry_t scene_table[SCENE_TABLE_SIZE];
    uint16_t group_table[GROUP_TABLE_SIZE];
} scene_db_t;
extern zb_af_device_ctx_t simple_desc_switch_ctx;
extern pwm_seq_para_head_t pwm_para_config[1];

void zigbee_app_init(void);
void zigbee_app_evt_change(uint32_t evt);
void set_duty_cycle(pwm_seq_para_head_t *pwm_para_config, uint8_t current_lv);


extern scene_db_t scene_table_db;
uint32_t get_on_off_status(void);
void set_on_off_status(uint32_t status);
uint32_t get_current_level(void);
void set_current_level(uint32_t level);
uint32_t get_identify_time(void);
void reset_attr(void);
void set_scene_count(uint8_t scene_count);
void scene_db_check(void);
void scene_db_update(void);
#ifdef __cplusplus
};
#endif
#endif /* __ZIGBEE_APP_H__ */
