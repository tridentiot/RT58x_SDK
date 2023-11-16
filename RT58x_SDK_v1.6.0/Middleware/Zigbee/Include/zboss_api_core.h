/* ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2020 DSR Corporation, Denver CO, USA.
 * www.dsr-zboss.com
 * www.dsr-corporation.com
 * All rights reserved.
 *
 * This is unpublished proprietary source code of DSR Corporation
 * The copyright notice does not evidence any actual or intended
 * publication of such source code.
 *
 * ZBOSS is a registered trademark of Data Storage Research LLC d/b/a DSR
 * Corporation
 *
 * Commercial Usage
 * Licensees holding valid DSR Commercial licenses may use
 * this file in accordance with the DSR Commercial License
 * Agreement provided with the Software or, alternatively, in accordance
 * with the terms contained in a written agreement between you and
 * DSR.
 */
/*  PURPOSE: ZBOSS core API header. Time, scheduler and memory buffers API.
*/

#ifndef ZB_ZBOSS_API_CORE_H
#define ZB_ZBOSS_API_CORE_H 1

#include "zb_vendor.h"
#include "zb_config.h"
#include "zb_types.h"
#include "zb_errors.h"

/* zb_callback_t is used in osif if we have serial API */
/*! \addtogroup sched */
/*! @{ */

/**
 *   Callback function typedef.
 *   Callback is function planned to execute by another function.
 *
 *   @param param - callback parameter - usually, but not always, ref to packet buf
 *
 *   See any sample
 */
typedef void (ZB_CODE *zb_callback_t)(zb_uint8_t param);

/*! @} */


/**
   Timer type.
 */

#ifdef ZB_TIMER_32
typedef zb_uint32_t zb_time_t;
/**
 * Maximum timer value, if 32-bit timer is used.
 */
#define ZB_MAX_TIME_VAL ZB_UINT32_MAX
/**
 * Minimum timer value, if 32-bit timer is used.
 */
#define ZB_MIN_TIME_VAL ZB_UINT32_MIN
#else
/* 16 bits for 8051 - it will be hw timer value. */
typedef zb_uint16_t zb_time_t;
/**
 * Maximum timer value, if 16-bit timer is used.
 */
#define ZB_MAX_TIME_VAL ZB_UINT16_MAX
/**
 * Minimum timer value, if 16-bit timer is used.
 */
#define ZB_MIN_TIME_VAL ZB_UINT16_MIN
#endif



/*! @} */

#include "zb_osif.h"
#include "zb_debug.h"
#include "zb_trace.h"




#endif /* ZB_ZBOSS_API_CORE_H */
