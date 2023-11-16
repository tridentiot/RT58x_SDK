// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#ifndef __EXAMPLE_API_H__
#define __EXAMPLE_API_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                Include (Better to prevent)
//=============================================================================

//=============================================================================
//                Public Definitions of const value
//=============================================================================
typedef void    (pf_evt_indication)(uint32_t data_len);
//=============================================================================
//                Public ENUM
//=============================================================================

//=============================================================================
//                Public Struct
//=============================================================================
typedef struct EXAMPLE_CONFIG_T
{
    pf_evt_indication   *pf_evt_indication;
} example_cfg_t;

typedef struct EXAMPLE_TLV_FORMAT
{
    uint16_t    type;       // Payload Identifier : defined by each module
    uint16_t    length;     // Payload Length : the length of the payload data
    uint8_t     value[];    // Payload Data
} example_tlv_t;

//=============================================================================
//                Public Function Declaration
//=============================================================================
/** stack interface init
 * @return 0 is success, other is fail
 */
int example_stack_init(example_cfg_t *pt_cfg);

/** Receive TLV packet from stack module via stack interface.
 *  When stack module call evt_indication of register function, APP
 *  can use this function to receive TLV packet from stack module.
 * @param pu8_buf the packet buffer pointer
 * @param pu32_buf_length the packet buffer length
 * @return 0 is success, other is fail
 */
int example_event_msg_recvfrom(uint8_t *pu8_buf, uint32_t *pu32_buf_len);

/** Send TLV packet to stack module via communication interface.
 *  This function will call memory_alloc of register function, and
 *  copy packet, then send to stack module.
 * @param pt_tlv Communication interface TLV packet pointer
 * @return 0 is success, other is fail
 */
int example_event_msg_sendto(example_tlv_t *pt_tlv);

#ifdef __cplusplus
};
#endif
#endif /* __EXAMPLE_API_H__ */
