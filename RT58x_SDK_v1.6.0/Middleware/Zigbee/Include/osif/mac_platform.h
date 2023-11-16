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
/* PURPOSE: MAC platform definitions.
*/

#ifndef __ZB_PLATFORM_MAC_H
#define __ZB_PLATFORM_MAC_H

#include "mac_rt570.h"

/**
   Tail size for mac packet
*/
#define ZB_TAIL_SIZE_FOR_RECEIVED_MAC_FRAME (ZB_MAC_EXTRA_DATA_SIZE) /* 2 */

/**
   Definitions for extra information size in RX fifo packet. Packet format:
   1 byte packet length + N bytes of RX fifo data + 9 bytes of extra information
   (LQI, RSSI, Frame timer, Superframe counter)
   See 3.2.3 RXMAC.
*/
#define ZB_MAC_PACKET_LENGTH_SIZE 1
#define ZB_MAC_EXTRA_DATA_SIZE    2 /* rssi & lqi - see ns_pkt_trailer_t */

#endif /* __ZB_PLATFORM_MAC_H */
