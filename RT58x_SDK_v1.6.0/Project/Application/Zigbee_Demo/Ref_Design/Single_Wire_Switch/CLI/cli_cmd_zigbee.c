/** @file cli_cmd_zigbee.c
 *
 * @author Rex
 * @version 0.1
 * @date 2021/12/23
 * @license
 * @description
 */


#include "project_config.h"
#if (MODULE_ENABLE(SUPPORT_DEBUG_CONSOLE_CLI))
#include "types.h"
#include "util_log.h"
#include "util_string.h"
#include "bsp_console.h"

#include "shell.h"
#include "sys_arch.h"

#include "zigbee_api.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
void _zigbee_cmd_req(uint32_t t_type, uint32_t t_index, uint32_t u32_len, uint8_t *pu8_value)
{
    sys_tlv_t *pt_tlv = sys_malloc(SYS_TLV_HEADER_SIZE + sizeof(raf_cmd_req_t) + u32_len);
    raf_cmd_req_t *pt_cmd_req;

    do
    {
        if (pt_tlv == NULL)
        {
            info_color(LOG_RED, "CMD malloc fail\n");
            break;
        }
        memset(pt_tlv, 0, SYS_TLV_HEADER_SIZE + sizeof(raf_cmd_req_t) + u32_len);

        pt_tlv->type = ZIGBEE_EVT_TYPE_RAF_CMD_REQUEST;
        pt_tlv->length = sizeof(raf_cmd_req_t) + u32_len;

        pt_cmd_req = (raf_cmd_req_t *)pt_tlv->value;
        pt_cmd_req->cmd_type = t_type;
        pt_cmd_req->cmd_index = t_index;
        pt_cmd_req->cmd_length = u32_len;
        memcpy(pt_cmd_req->cmd_value, pu8_value, u32_len);

        util_log_mem(UTIL_LOG_INFO, "  ", (uint8_t *)pt_tlv, SYS_TLV_HEADER_SIZE + sizeof(raf_cmd_req_t) + u32_len, 0);

        zigbee_event_msg_sendto(pt_tlv);

        sys_free(pt_tlv);
    } while (0);
}

static void cli_get_panid(void)
{
    _zigbee_cmd_req(RAF_CMD, GET_PANID, 0, NULL);
}

static int
_cli_cmd_zb(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{
    do
    {
        if (argc < 2)
        {
            log_out("parameter error\n");
            break;
        }

        if (memcmp("panid", argv[1], 5) == 0)
        {
            cli_get_panid();
        }
    } while (0);
    return 0;
}


//=============================================================================
//                  Public Function Definition
//=============================================================================
const sh_cmd_t  g_cli_cmd_zb __cli_cmd_pool =
{
    .pCmd_name      = "zb",
    .cmd_exec       = _cli_cmd_zb,
    .pDescription   = "zigbee cmd\n"
    "  usage: zb [sub cmd] [value]\n"
    "    e.g. zb panid",
};



#endif
