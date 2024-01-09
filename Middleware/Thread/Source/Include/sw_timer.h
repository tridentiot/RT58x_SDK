/**
 * @file sw_timer.h
 * @author Rex Huang (rex.huang@rafaelmicro.com)
 * @brief
 * @version 0.1
 * @date 2022-08-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SW_TIMER_H__
#define __SW_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Include (Better to prevent)
//=============================================================================
#include <stdint.h>
#include "util_list.h"
//=============================================================================
//                Public Definitions of const value
//=============================================================================
#define SW_MAX_TIMER                0xFFFFFFFF
#define SW_TIMER_MAX_PERIOD         0x0A4CB800  // 172800000 ms = 48 hour
#define SW_TIMER_SHIFT_THRESHOLD    0x80000000  // About 24.8 day

//############################################################
// SW Timer Tick = 0.1 ms
// MAX Time = 48 Hour = 2880 min = 172800 sec =  172800000 ms
//############################################################
#define SW_TIMER_HOUR_TO_TICK(h)            ((h) * 3600000)
#define SW_TIMER_MINUTE_TO_TICK(m)          ((m) * 60000)
#define SW_TIMER_SECOND_TO_TICK(s)          ((s) * 1000)
#define SW_TIMER_MILLISECOND_TO_TICK(ms)    (ms)

//=============================================================================
//                Public ENUM
//=============================================================================

typedef enum SW_TIMER_EXECUTE_MODE
{
    SW_TIMER_EXECUTE_ONCE_FOR_EACH_TIMEOUT      = 0,
    SW_TIMER_EXECUTE_ONCE_FOR_DUPLICATE_TIMEOUT = 1,
} sw_timer_mode_t;

typedef enum SW_TIMER_ERRNO
{
    SW_TIMER_EXECUTE_MODE_FAIL  = -5,
    SW_TIMER_PERIOD_FAIL        = -4,
    SW_TIMER_PRIORITY_FAIL      = -3,
    SW_TIMER_CMD_SEND_FAIL      = -2,
    SW_TIMER_NULL               = -1,
    SW_TIMER_PASS               = 0,
} sw_timer_err_t;

//=============================================================================
//                Public Struct
//=============================================================================
typedef void (*sw_timer_cb)(void *p_param);

typedef struct SW_TIMER
{
    uint32_t            period;
    const char          *name;

    uint32_t            auto_reload             : 1;
    uint32_t            execute_mode            : 1;    // follow enum sw_timer_execute_mode_e
    uint32_t            receive_delete_cmd      : 1;
    uint32_t            wating_for_execution    : 8;
    uint32_t            running                 : 1;
    uint32_t            reserved                : 20;

    void                *cb_param;
    sw_timer_cb         cb_function;

    uint32_t            timeout;
    link_list_t         list;
} sw_timer_t;

//=============================================================================
//                Public Function Declaration
//=============================================================================
/**
 * @brief Create a software timer
 *
 * @param name A human readable text name that is assigned to the timer.
 * @param period The period of the timer.
 * @param auto_reload If auto_reload is set to TRUE, then the timer will
 * expire repeatedly with a frequency set by the period parameter.
 * If auto_reload is set to FALSE, then the timer will
 * be a one-shot and enter the dormant state after it expires.
 * @param execute_mode The execute mode of the timer.
 * @param cb_param The input parameter of the call back function.
 * @param cb_function The function to call when the timer expires.
 * @return sw_timer_t* If the timer is created successfully then a handle to the newly created timer is returned.
 * If the timer cannot be created the NULL is returned.
 */
sw_timer_t *sw_timer_create(const char *name, uint32_t period, uint32_t auto_reload, uint32_t execute_mode,
                            void *cb_param, sw_timer_cb cb_function);
/**
 * @brief Start a software timer that was previously created using the sw_timer_create()API function.
 *
 * @param timer The handle of the timer being started/restarted.
 * @return sw_timer_err_t SW_TIMER_PASS will be returned if the command was successfully sent to the command queue.
 * SW_TIMER_CMD_SEND_FAIL will be returned if the start command could not be sent to the timer command queue.
 */
sw_timer_err_t sw_timer_start(sw_timer_t *timer);

/**
 * @brief Stop a software timer that was previously created using the sw_timer_create()API function.
 * Stopping a timer ensures the timer is not in the active state.
 *
 * @param timer The handle of the timer being stopped.
 * @return sw_timer_err_t SW_TIMER_PASS will be returned if the stop command was successfully sent to the command queue.
 * SW_TIMER_CMD_SEND_FAIL will be returned if the stop command could not be sent to the timer command queue.
 */
sw_timer_err_t sw_timer_stop(sw_timer_t *timer);

/**
 * @brief Re-start a software timer that was previously created using the sw_timer_create()API function.
 * Restting a timer ensures the timer is in the active state.
 *
 * @param timer The handle of the timer being reset/started/restarted.
 * @return sw_timer_err_t SW_TIMER_PASS will be returned if the reset command was successfully sent to the command queue.
 * SW_TIMER_CMD_SEND_FAIL will be returned if the reset command could not be sent to the timer command queue.
 */
sw_timer_err_t sw_timer_reset(sw_timer_t *timer);

/**
 * @brief Changes the period of a software timer that was previously created using the sw_timer_create()API function.
 *
 * @param timer The handle of the timer that is having its period chanded.
 * @param period The new period for timer.
 * @return sw_timer_err_t SW_TIMER_PASS will be returned if the change period command was successfully sent to the command queue.
 * SW_TIMER_CMD_SEND_FAIL will be returned if the change period command could not be sent to the timer command queue.
 */
sw_timer_err_t sw_timer_change_period(sw_timer_t *timer, uint32_t period);

/**
 * @brief Changes the execute mode of a software timer that was previously created using the sw_timer_create()API function.
 *
 * @param timer The handle of the timer that is having its execute mode.
 * @param execute_mode The new execute mode for timer.
 * @return sw_timer_err_t SW_TIMER_PASS will be returned if the change execute mode command was successfully sent to the command queue.
 * SW_TIMER_CMD_SEND_FAIL will be returned if the change execute mode command could not be sent to the timer command queue.
 */
sw_timer_err_t sw_timer_change_execute_mode(sw_timer_t *timer, uint32_t execute_mode);

/**
 * @brief Delete a software timer that was previously created using the sw_timer_create()API function.
 *
 * @param timer The handle of the timer being deleted.
 * @return sw_timer_err_t SW_TIMER_PASS will be returned if the delete command was successfully sent to the command queue.
 * SW_TIMER_CMD_SEND_FAIL will be returned if the delete command could not be sent to the timer command queue.
 */
sw_timer_err_t sw_timer_delete(sw_timer_t *timer);


/**
 * @brief Initial the software tasks.
 *
 * @param u32_hw_timer_id Hardware timer id for the software timer task.
 * @return int SW_ERR_OK will be returned if the initial was successfully.
 */
// int sw_timer_init(uint32_t u32_hw_timer_id);
void sw_timer_init();

/**
 * @brief Queries a software timer to see if it is active or dormant.
 *
 * @param timer The timer being queried.
 * @return true The timer is active.
 * @return false The timer is dormant.
 */
bool sw_timer_get_running(sw_timer_t *timer);

/**
 * @brief Get the tick of the software timer.
 *
 * @return uint32_t The tick ot the time.
 */
uint32_t sw_timer_get_tick(void);
/**
 * @brief Set the tick of the software timer.
 *
 * @return
 */
void sw_timer_set_tick(uint32_t timer);


void sw_timer_tick_handler();

void sw_timer_proc(void);

/** @} */

#ifdef __cplusplus
};
#endif
#endif /* __SW_TIMER_H__ */
