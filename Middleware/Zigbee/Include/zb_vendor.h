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
/* PURPOSE:
*/
#ifndef ZB_VENDOR_RAFAEL_H
#define ZB_VENDOR_RAFAEL_H 1

#define A015  0U
#define A047  2U
#define A048  1U

#ifndef BOARD
//#define BOARD   A047
#define BOARD   A048
#endif
/* Number of buttons supported */
#if (BOARD==A047)
#define ZB_N_BUTTONS 4
#else
#define ZB_N_BUTTONS 5
#endif




#endif /* ZB_VENDOR_RAFAEL_H */
