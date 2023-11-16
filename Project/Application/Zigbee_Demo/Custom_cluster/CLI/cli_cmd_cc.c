/** @file cli_cmd_gateway.c
 *
 * @author Rex
 * @version 0.1
 * @date 2022/01/17
 * @license
 * @description
 */


#include "project_config.h"
#include "types.h"
#include "util_string.h"
#include "bsp_console.h"

#include "shell.h"
#include "sys_arch.h"
#include "FreeRTOS.h"
#include "task.h"

#include "mem_mgmt.h"

#include "zigbee_stack_api.h"
#include "zigbee_lib_api.h"

#include "zigbee_app.h"

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
extern zb_uint8_t custom_dst_endpoint;
extern zb_uint16_t custom_dst_short_addr;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_cli_cmd_set_custom_target(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{

    do
    {
        if (argc < 3)
        {
            log_out("Invalid parameter\n");
            break;
        }

        custom_dst_short_addr = (*(argv[1] + 1) == 'x')
                                ? utility_strtox(argv[1] + 2, 0)
                                : utility_strtol(argv[1], 0);

        custom_dst_endpoint = (*(argv[2] + 1) == 'x')
                              ? utility_strtox(argv[2] + 2, 0)
                              : utility_strtol(argv[2], 0);

    } while (0);
    return 0;
}

static int
_cli_cmd_zcl_send_custom(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{
    zigbee_zcl_data_req_t *pt_data_req;
    uint8_t len = 0, i, j = 0, k = 0;

    do
    {
        if (argc < 2)
        {
            break;
        }

        for (i = 0; i < argc - 1; i++)
        {
            len += strlen(argv[i + 1]);
        }

        len += (argc - 1 - 1);

        ZIGBEE_ZCL_DATA_REQ(pt_data_req, custom_dst_short_addr, ZB_APS_ADDR_MODE_16_ENDP_PRESENT, custom_dst_endpoint, ZIGBEE_DEFAULT_ENDPOINT,
                            ZB_ZCL_CLUSTER_ID_CUSTOM,
                            UART_TRANSPARENT_CMD,
                            TRUE, TRUE,
                            ZCL_FRAME_CLIENT_SERVER_DIR, 0, len)

        if (pt_data_req)
        {
            for (i = 0; i < argc - 1; i++)
            {
                for (j = 0; j < strlen(argv[i + 1]); j++)
                {
                    pt_data_req->cmdFormat[k] = *(argv[i + 1] + j);
                    k++;
                }
                pt_data_req->cmdFormat[k] = 0x20;
                k++;
            }
            if (k - 1 < 80)
            {
                zigbee_zcl_request(pt_data_req, pt_data_req->cmdFormatLen);
            }
            else
            {
                log_out("The UART Transparent data should less than 80 bytes, abort!\n");
            }
            sys_free(pt_data_req);
        }

    } while (0);
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
const sh_cmd_t  g_cli_cmd_set_custom_target __cli_cmd_pool =
{
    .pCmd_name      = "ct",
    .cmd_exec       = _cli_cmd_set_custom_target,
    .pDescription   = "set target to send custom clster ID packet to\n"
    "  usage: ct [short address] [end point]\n"
    "    e.g. ct 0x1234 02\n",
};

const sh_cmd_t  g_cli_cmd_zcl_send_custom __cli_cmd_pool =
{
    .pCmd_name      = "cs",
    .cmd_exec       = _cli_cmd_zcl_send_custom,
    .pDescription   = "send custom clster ID packet\n"
    "  usage: cs [value]\n"
    "    e.g. cs 0xaa\n",
};
