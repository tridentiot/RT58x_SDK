/************************************************************************
 *
 * File Name  : ble_profile_app.c
 * Description: This file contains the definitions and functions of BLE profiles for application.
 *
 ************************************************************************/
#include "stdio.h"
#include "ble_profile.h"
#include "project_config.h"

/**************************************************************************
 * Profile Application Public Definitions and Variables
 **************************************************************************/

/* BLE application TRSP profile (server) information. */
ble_info_link0_t   ble_server_profile_info;


/* BLE application links information which is order by host id. */
ble_app_link_info_t ble_app_link_info[BLE_SUPPORT_NUM_CONN_MAX] =
{
    // Link 0, host id = 0
    {
        .gap_role = BLE_GAP_ROLE_PERIPHERAL,
        .state = 0x00,
        .profile_info = (void *) &ble_server_profile_info,
    },
};
/**************************************************************************
 * Profile Application GENERAL Public Functions
 **************************************************************************/


