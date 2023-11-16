/** @file cli_cmd_rf_subg.c
 *
 * @author Jiawei
 * @version 0.1
 * @date 2022/10/20
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
#include "util_log.h"

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
uint8_t  g_start_flag = 0;
uint32_t g_data_rate;  //unit: bps
uint32_t g_frequency;  //unit: kHz
uint16_t g_pkt_number; //unit: k pkts
uint8_t g_band_type;

//=============================================================================
//                  Private Function Definition
//=============================================================================
static int
_cli_cmd_start(int argc, char **argv, cb_shell_out_t log_out, void *pExtra)
{
    bool_t start_permit = 1;
    do
    {
        if (argc < 4)
        {
            log_out("Invalid parameter number\n");
            start_permit = 0;
            break;
        }

        if (g_start_flag == 1)
        {
            info_color(LOG_RED, "Test already started\n");
            start_permit = 0;
            break;
        }
        g_data_rate = (*(argv[1] + 1) == 'x')
                      ? utility_strtox(argv[1] + 2, 0)
                      : utility_strtol(argv[1], 0);

        g_frequency = (*(argv[2] + 1) == 'x')
                      ? utility_strtox(argv[2] + 2, 0)
                      : utility_strtol(argv[2], 0);

        g_pkt_number = (*(argv[3] + 1) == 'x')
                       ? utility_strtox(argv[3] + 2, 0)
                       : utility_strtol(argv[3], 0);


        if (g_data_rate != 6250 && g_data_rate != 12500 && g_data_rate != 25000 && g_data_rate != 50000 &&
                g_data_rate != 100000 && g_data_rate != 150000 && g_data_rate != 200000 && g_data_rate != 300000)
        {
            log_out("Unsupported data rate %d bps\n", g_data_rate);
            start_permit = 0;
            break;
        }

        if (g_frequency < 100000 || g_frequency > 1200000)
        {
            log_out("Invalid frequency %d kHz\n", g_frequency);
            start_permit = 0;
            break;
        }
        else if (g_frequency > 900000)
        {
            g_band_type = 0;
        }
        else if (g_frequency > 800000)
        {
            g_band_type = 1;
        }
        else if (g_frequency > 400000)
        {
            g_band_type = 2;
        }
        else
        {
            g_band_type = 3;
        }

        if (g_data_rate >= 50000 && g_band_type > 0)
        {
            g_band_type++;
        }
    } while (0);

    if (start_permit)
    {
        log_out("Test start!\n");
        g_start_flag = 1;
    }

    return 0;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================

const sh_cmd_t  g_cli_cmd_start __cli_cmd_pool =
{
    .pCmd_name      = "start",
    .cmd_exec       = _cli_cmd_start,
    .pDescription   = "start\n"
    "  usage: start [data rate](bps) [frequency](kHz) [packet number](k)\n"
    "    e.g. start 50 915000 30\n",
};

#endif
