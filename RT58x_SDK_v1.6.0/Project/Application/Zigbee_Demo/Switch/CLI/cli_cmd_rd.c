/** @file cli_cmd_rd.c
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

#include "shell.h"
#include "bsp_console.h"

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
static int
_cli_cmd_rd(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{
    do
    {
        uint32_t    addr = 0, cnt = 0;
        uint32_t    *pCur = 0;

        if ( argc != 3 )
        {
            log_out("wrong parameters\n");
            break;
        }

        addr = (*(argv[1] + 1) == 'x')
               ? utility_strtox(argv[1] + 2, 0)
               : utility_strtol(argv[1], 0);

        cnt  = (*(argv[2] + 1) == 'x')
               ? utility_strtox(argv[2] + 2, 0)
               : utility_strtol(argv[2], 0);

        pCur = (uint32_t *)addr;
        cnt  = (cnt + 0x3) >> 2;

        for (int i = 0; i < cnt; ++i)
        {
            if ( !(i & 0x3) )
            {
                log_out("\n%08X |", addr);
                addr += 16;
            }

            log_out(" %08X", pCur[i]);
        }
        log_out("\n\n");
    } while (0);
    return 0;
}

static int _cli_cmd_dw(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{
    do
    {
        uint32_t    addr = 0, cnt = 0;
        uint32_t    *pCur = 0;

        if ((argc != 3) && (argc != 2))
        {
            log_out("wrong parameters\n");
            break;
        }

        addr = (*(argv[1] + 1) == 'x')
               ? utility_strtox(argv[1] + 2, 0)
               : utility_strtox(argv[1], 0);

        if (argc == 2)
        {
            cnt = 64;
        }
        else
        {
            cnt  = (*(argv[2] + 1) == 'x')
                   ? utility_strtox(argv[2] + 2, 0)
                   : utility_strtox(argv[2], 0);
        }

        pCur = (uint32_t *)addr;
        cnt  = (cnt + 0x3) >> 2;

        for (int i = 0; i < cnt; ++i)
        {
            if ( !(i & 0x3) )
            {
                log_out("\n%08X |", addr);
                addr += 16;
            }

            log_out(" %08X", pCur[i]);
        }
        log_out("\n\n");
    } while (0);
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
const sh_cmd_t  g_cli_cmd_rd __cli_cmd_pool =
{
    .pCmd_name      = "rd",
    .cmd_exec       = _cli_cmd_rd,
    .pDescription   = "read memory data\n"
    "  usage: rd [address] [length]\n"
    "    e.g. rd 0x30000000 32",
};

const sh_cmd_t  g_cli_cmd_dw __cli_cmd_pool =
{
    .pCmd_name      = "dw",
    .cmd_exec       = _cli_cmd_dw,
    .pDescription   = "read memory data\n"
    "  usage: rd [address] [length]\n"
    "    e.g. rd 0x30000000 32",
};

#endif
