/** @file cli_cmd_sys.c
 *
 * @author Rex
 * @version 0.1
 * @date 2021/08/04
 * @license
 * @description
 */


#include "project_config.h"
#if (MODULE_ENABLE(SUPPORT_DEBUG_CONSOLE_CLI))
#include "types.h"
#include "util_string.h"
#include "bsp_console.h"

#include "shell.h"
//#include "sys_arch.h"
//#include "FreeRTOS.h"
//#include "task.h"

#include "mem_mgmt.h"
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
extern void Sys_Software_Reset(void);
static int
_cli_cmd_sys_reset(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{
    Sys_Software_Reset();

    return 0;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
const sh_cmd_t  g_cli_cmd_sys_reset __cli_cmd_pool =
{
    .pCmd_name      = "reset",
    .cmd_exec       = _cli_cmd_sys_reset,
    .pDescription   = "reset device\n"
    "  usage: reset\n"
    "    e.g. reset",
};

#endif
