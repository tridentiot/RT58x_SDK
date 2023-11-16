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
/*  PURPOSE: Public APS layer API
*/
#ifndef ZB_ZBOSS_API_APS_H
#define ZB_ZBOSS_API_APS_H 1

/**
 * @addtogroup ZigBee
 * @{
 * @defgroup APS
 * @{
 * @details
 * @defgroup aps_api
 * @}
 * @}
**************************************************************************/

/*! \addtogroup aps_api */
/*! @{ */

/** @addtogroup aps_common_constants APS common constants
  * @{
  */

//#include "zb_config.h"
//#include "zb_address.h"
//#include "zboss_api_aps_interpan.h"

/**
 * @name APS status constants
 * @anchor aps_status
 */
/** @{ */


/** @} */



/**
 * @name APS addressing mode constants
 * @anchor aps_addr_mode
 */
/** @{ */
#define ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT  0x00U /*!< DstAddress and DstEndpoint not present  */
#define ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT  0x01U /*!< 16-bit group address for DstAddress; DstEndpoint not present */
#define ZB_APS_ADDR_MODE_16_ENDP_PRESENT            0x02U /*!< 16-bit address for DstAddress and DstEndpoint present */
#define ZB_APS_ADDR_MODE_64_ENDP_PRESENT            0x03U /*!< 64-bit extended address for DstAddress and DstEndpoint present  */
#define ZB_APS_ADDR_MODE_BIND_TBL_ID                0x04U /*!< "destination endpoint" is interpreted as an index in the binding table,
                                                              all other destination address information is ignored */
/** @} */

/** @brief APSME binding structure.
  *
  * This data structure passed to @ref zb_apsme_bind_request()
  * and to @ref zb_apsme_unbind_request().
  */
typedef struct zb_apsme_binding_req_s
{
    zb_ieee_addr_t  src_addr;       /*!< The source IEEE address for the binding entry. */
    zb_uint8_t      src_endpoint;   /*!< The source endpoint for the binding entry. */
    zb_uint16_t     clusterid;      /*!< The identifier of the cluster on the source
                                        device that is to be bound to the destination device.*/
    zb_uint8_t      addr_mode;      /*!< The type of destination address supplied by
                                       the DstAddr parameter - see @ref aps_addr_mode  */
    zb_addr_u       dst_addr;       /*!< The destination address for the binding entry. */
    zb_uint8_t      dst_endpoint;   /*!< This parameter will be present only if
                                       the DstAddrMode parameter has a value of
                                       0x03 and, if present, will be the
                                       destination endpoint for the binding entry.*/
#ifdef SNCP_MODE
    zb_uint8_t       remote_bind;   /*!< Indication if the bind req is local or remote */
    zb_uint8_t       id;             /*!< unique identifier of the entry for NCP, updated only in
                                    zb_apsme_bind_request and zb_apsme_unbind_request and used to
                                    notify NCP */
    /* confirm_cb is not sent in payload by NCP, keep it in the end of this structure */
#endif
    zb_callback_t   confirm_cb;     /*!< The callback to be called when the operation is completed. */
} zb_apsme_binding_req_t;


/**
 * @brief Type for APS addressing mode constants.
 *
 * Holds one of @ref aps_addr_mode. Kept only for backward compatibility as
 * @ref aps_addr_mode were declared previously as enum.
 */
typedef zb_uint8_t zb_aps_addr_mode_t;

/**
  * This data structure passed to @ref zb_aps_check_binding_request().
  */
typedef struct zb_aps_check_binding_req_s
{
    zb_uint8_t src_endpoint; /* a source endpoint, if ZB_ZCL_BROADCAST_ENDPOINT - any endpoint */
    zb_uint16_t cluster_id; /* a source cluster ID */
    zb_callback_t response_cb;
} zb_aps_check_binding_req_t;


/**
  * This data structure passed to callback passed to @ref zb_aps_check_binding_request().
  */
typedef struct zb_aps_check_binding_resp_s
{
    zb_uint8_t src_endpoint; /* a source endpoint from the corresponding request */
    zb_uint16_t cluster_id; /* a source cluster ID from the corresponding request */
    zb_bool_t exists; /* whether the binding with specified parameters exists */
} zb_aps_check_binding_resp_t;




#endif /* #ifndef ZB_ZBOSS_API_APS_H */
