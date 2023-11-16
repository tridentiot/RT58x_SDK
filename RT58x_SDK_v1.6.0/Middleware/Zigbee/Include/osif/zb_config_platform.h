/* ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2020 DSR Corporation, Denver CO, USA.
 * http://www.dsr-zboss.com
 * http://www.dsr-corporation.com
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
/* PURPOSE: Configuration file: configuration- and platform-specific definitions,
constants etc.
*/
#ifndef ZB_CONFIG_PLATFORM_H
#define ZB_CONFIG_PLATFORM_H 1

#include "project_config.h"

#define Timer0 0
#define Timer1 1
#define Timer2 2
#define Timer3 3

#if (MODULE_ENABLE(SUPPORT_FREERTOS_PORT))
#define UsedTimer Timer2
#else
#if (defined (ZB_ED_ROLE) && (CHIP_VERSION == RT58X_MPA))
#define UsedTimer Timer3  //RT58X_SHUTTLE not support Timer3, Timer3 will be used for ZED sleepy device
#else
#define UsedTimer Timer0
#endif
#endif


#endif /* ZB_CONFIG_PLATFORM_H */
