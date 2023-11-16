/**
 * @file zigbee_gateway_cmd_app_service.c
 * @author Rex Huang (rex.huang@rafaelmicro.com)
 * @brief
 * @version 0.1
 * @date 2022-05-03
 *
 * @copyright Copyright (c) 2022
 *
 */

//=============================================================================
//                Include
//=============================================================================
/* OS Wrapper APIs*/
#include "sys_arch.h"

/* Utility Library APIs */
#include "util_printf.h"
#include "util_log.h"

/* ZigBee Stack Library APIs */
#include "zigbee_stack_api.h"
#include "zigbee_app.h"

#include "zigbee_evt_handler.h"
#include "zigbee_zcl_msg_handler.h"
#include "zigbee_lib_api.h"

#include "zigbee_gateway.h"


//=============================================================================
//                Private ENUM
//=============================================================================
typedef enum
{
    DEVICE_BIND_TABLE = 1,
    GROUP_TABLE,
    SCENE_TABLE,
} backup_table_idx;

//=============================================================================
//                Private Definitions of const value
//=============================================================================
#define INVALID_FLASH_SEQ                   (~0)
#define FLASH_DEFAULT_VALUE                 (0xFF)
#define RECOVER_DONE_SEQ                    (0x7a)

#define FLASH_SECTOR_SIZE                   (4096)
#define FLASH_PAGE_SIZE                     (256)     /**< Programming 256 bytes at once */

#define BIND_TABLE_OFFSET                   (0xC00)

#define DEVICE_TABLE_FLASH_ADDRESS          (0x000F0000)

#define BIND_TABLE_FLASH_ADDRESS            (DEVICE_TABLE_FLASH_ADDRESS + BIND_TABLE_OFFSET)

#define GROUP_TABLE_FLASH_ADDRESS           (DEVICE_TABLE_FLASH_ADDRESS + FLASH_SECTOR_SIZE)

#define SCENE_TABLE_FLASH_ADDRESS           (GROUP_TABLE_FLASH_ADDRESS + FLASH_SECTOR_SIZE)


#define BACKUP_TABLES_ADDRESS               (SCENE_TABLE_FLASH_ADDRESS + FLASH_SECTOR_SIZE)
#define BACKUP_RECOVER_TABLE_ADDRESS        (BACKUP_TABLES_ADDRESS + FLASH_SECTOR_SIZE - 2)
#define BACKUP_RECOVER_DONE_ADDRESS         (BACKUP_TABLES_ADDRESS + FLASH_SECTOR_SIZE - 1)

#define MAX_ADDR_MAPPING_SIZE               (15)

//=============================================================================
//                Private Struct
//=============================================================================

typedef struct __attribute__((packed))
{
    uint8_t  status;
    uint16_t nwk_Addr;
    uint8_t  length;
    uint8_t  end_point;
    uint16_t profile_id;
    uint16_t device_id;
}
zb_zdo_simple_desc_t;

typedef struct __attribute__((packed))
{
    uint8_t                 used;             /* */
    device_info_param_t     device_info;
    device_name_param_t     device_name;  /* */
    end_point_name_param_t  ep_name;
}
device_table_param_t;

typedef struct __attribute__((packed))
{
    device_table_param_t device_table[DEVICE_TABLE_NUM];
}
device_table_info_t;

typedef struct __attribute__((packed))
{
    uint8_t              ieee_addr[8];
    uint8_t              end_point;
}
extended_address_t;

typedef struct __attribute__((packed))
{
    uint8_t               used;             /* */
    bind_info_param_t     bind_info;
    bind_address_param_t  dst_addr;
}
bind_table_param_t;

typedef struct __attribute__((packed))
{
    bind_table_param_t   bind_table[BIND_TABLE_NUM];
}
bind_table_info_t;


typedef struct __attribute__((packed))
{
    uint8_t             used;             /* */
    uint8_t             end_point;        /* */
    uint16_t            nwk_addr;         /* */
}
group_param_t;

typedef struct __attribute__((packed))
{
    uint8_t             used;             /* */
    uint8_t             group_name_len;   /* */
    uint8_t             group_name[GROUP_NAME_LEN];   /* */
    uint16_t            group_id;         /* */
    group_param_t       group_member[GROUP_TABLE_MEMBER_NUM];
}
group_table_param_t;

typedef struct __attribute__((packed))
{
    group_table_param_t  group_table[GROUP_TABLE_NUM];
}
group_table_info_t;

typedef struct __attribute__((packed))
{
    uint8_t             used;             /* */
    uint8_t             end_point;        /* */
    uint16_t            nwk_addr;         /* */
}
scene_param_t;

typedef struct __attribute__((packed))
{
    uint8_t             used;             /* */
    uint8_t             scene_name_len;   /* */
    uint8_t             scene_name[SCENE_NAME_LEN];   /* */
    uint8_t             scene_id;         /* */
    uint16_t            group_id;         /* */
    scene_param_t       scene_member[SCENE_TABLE_MEMBER_NUM];
}
scene_table_param_t;

typedef struct __attribute__((packed))
{
    scene_table_param_t  scene_table[SCENE_TABLE_NUM];
}
scene_table_info_t;

typedef struct __attribute__((packed))
{
    uint8_t              occupied;             /* */
    uint16_t             nwk_addr;             /* */
    uint8_t              ieee_addr[8];
}
leave_req_mapping_t;

typedef struct __attribute__((packed))
{
    uint8_t              occupied;             /* */
    uint16_t             mapping_addr;
    bind_info_param_t    bind_info;
    bind_address_param_t dst_addr;
}
bind_unbind_req_mapping_t;

//=============================================================================
//                Private Function Declaration
//=============================================================================

void bind_table_copy_to_backup_page(void);


//=============================================================================
//                Private Global Variables
//=============================================================================

device_table_info_t *p_device_table_info = (device_table_info_t *)(DEVICE_TABLE_FLASH_ADDRESS);
group_table_info_t *p_group_table_info   = (group_table_info_t *)(GROUP_TABLE_FLASH_ADDRESS);
scene_table_info_t *p_scene_table_info   = (scene_table_info_t *)(SCENE_TABLE_FLASH_ADDRESS);
bind_table_info_t *p_bind_table_info     = (bind_table_info_t *)(BIND_TABLE_FLASH_ADDRESS);


static bind_unbind_req_mapping_t bind_mapping[MAX_ADDR_MAPPING_SIZE];
static bind_unbind_req_mapping_t unbind_mapping[MAX_ADDR_MAPPING_SIZE];

//=============================================================================
//                Global Variables
//=============================================================================
uint16_t g_pan_id = 0xFFFF;
uint8_t g_channel = 0xFF;

//=============================================================================
//                Functions
//=============================================================================

static void flash_program(uint32_t addr, uint8_t *p_data, uint32_t data_len)
{
    uint16_t program_idx;

    for (program_idx = 0; program_idx < data_len; program_idx++)
    {
        while (flash_check_busy());
        flash_write_byte((addr + program_idx), p_data[program_idx]);
        while (flash_check_busy());
    }

}

static uint32_t flash_sector_erase(uint32_t addr)
{
    uint32_t erase_status;

    while (flash_check_busy());
    erase_status = flash_erase(FLASH_ERASE_SECTOR, addr);
    while (flash_check_busy());

    if (erase_status != STATUS_SUCCESS)
    {
        info_color(LOG_RED, "flash erase fail %d\n", erase_status);
    }
    return erase_status;
}


uint8_t group_id_valid(uint16_t group_id)
{
    return (group_id > GROUP_TABLE_NUM) ? false : true;
}

uint8_t gw_cmd_forward(uint32_t command_id, uint8_t command_len, uint8_t *p_command_payload)
{
    //+-------------+-----------+---------------+------------+-----------------+--------------+--------+
    //|  Header(4)  | Length(1) | Command ID(4) | Address(2) | Address Mode(1) | Parameter(N) | CS (1) |
    //+-------------+-----------+---------------+------------+-----------------+--------------+--------+
    //| FF FC FC FF |           |               |            |                 |              |        |
    //+-------------+-----------+---------------+------------+-----------------+--------------+--------+
    sys_tlv_t *pt_tlv;
    gateway_cmd *p_cmd;
    uint8_t cmd_forward = false;

    pt_tlv = sys_malloc(sizeof(sys_tlv_t) + GW_CMD_HEADER_LEN + command_len);
    if (pt_tlv)
    {
        pt_tlv->length = GW_CMD_HEADER_LEN + command_len;
        p_cmd = (gateway_cmd *)pt_tlv->value;
        p_cmd->hdr.header[0] = 0xff;
        p_cmd->hdr.header[1] = 0xfc;
        p_cmd->hdr.header[2] = 0xfc;
        p_cmd->hdr.header[3] = 0xff;
        p_cmd->hdr.len = sizeof(gateway_cmd_pd) + command_len;
        p_cmd->pd.command_id = command_id;
        p_cmd->pd.address = 0x0000;
        p_cmd->pd.address_mode = 0x00;

        memcpy(p_cmd->pd.parameter, p_command_payload, command_len);

        zigbee_gateway_cmd_recv(pt_tlv);
        cmd_forward = true;
    }
    else
    {
        info_color(LOG_RED, "gw cmd out of mem\n");
    }
    return cmd_forward;
}
void gw_default_rsp_check(uint16_t cluster_id, uint8_t cmd, uint16_t src_addr, uint8_t src_ep, uint8_t status)
{
    info_color(LOG_RED, "gw_default_rsp_check cluster id 0x%04x, cmd %d, srcAddr 0x%04x srcEP %d, status %d\n",
               cluster_id, cmd, src_addr, src_ep, status);
    switch (cluster_id)
    {
    case ZB_ZCL_CLUSTER_ID_GROUPS:
        switch (cmd)
        {
        case GW_CMD_APP_SRV_GROUP_REMOVE_ALL:
        {
            if (status == 0x00)
            {
                group_all_groups_remove_cmd_forward(src_addr, src_ep);
            }
        }
        break;
        }
        break;
    }

}

void gw_cmd_check(uint32_t command_id, uint8_t *p_command_payload)
{
    uint8_t i, j;
    uint16_t dst_addr;
    memcpy((uint8_t *)&dst_addr, p_command_payload + 9, 2);

    switch (command_id)
    {
    case RAF_CMD_BIND_REQUEST:
    case RAF_CMD_UNBIND_REQUEST:
    {
        uint8_t idx = GW_CMD_HEADER_LEN;
        bind_info_param_t *p_bind_info;
        bind_unbind_req_mapping_t *p_bind_unbind_mapping;

        p_bind_unbind_mapping = (command_id == RAF_CMD_BIND_REQUEST) ?
                                bind_mapping : unbind_mapping;
        p_bind_info = (bind_info_param_t *)(p_command_payload + idx);
        idx += sizeof(bind_info_param_t);

        //confirm bind/unbind information already exist in mapping table or not
        for (i = 0; i < MAX_ADDR_MAPPING_SIZE; i++)
        {
            if ((p_bind_unbind_mapping[i].occupied == 1) &&
                    (memcmp((uint8_t *)&p_bind_unbind_mapping[i].bind_info, (uint8_t *)p_bind_info, sizeof(bind_info_param_t)) == 0) &&
                    (((p_bind_info->dst_addr_mode == ADDR_MODE_GROUP) && (memcmp((uint8_t *)&p_bind_unbind_mapping[i].dst_addr.group_id, (p_command_payload + idx), 2) == 0)) ||
                     ((p_bind_info->dst_addr_mode == ADDR_MODE_EXTENDED_ADDR) && (memcmp((uint8_t *)&p_bind_unbind_mapping[i].dst_addr.unicast.ieee_addr, (p_command_payload + idx), 8) == 0)
                      && (p_bind_unbind_mapping[i].dst_addr.unicast.end_point == (p_command_payload[idx + 8])))))
            {
                p_bind_unbind_mapping[i].mapping_addr = dst_addr;
                break;
            }
        }

        //not found, get a new one mapping table
        if (i == MAX_ADDR_MAPPING_SIZE)
        {
            for (i = 0; i < MAX_ADDR_MAPPING_SIZE; i++)
            {
                if (p_bind_unbind_mapping[i].occupied == 0)
                {
                    p_bind_unbind_mapping[i].occupied = 1;
                    p_bind_unbind_mapping[i].mapping_addr = dst_addr;
                    memcpy((uint8_t *)&p_bind_unbind_mapping[i].bind_info, (uint8_t *)p_bind_info, sizeof(bind_info_param_t));
                    if (p_bind_unbind_mapping[i].bind_info.dst_addr_mode == ADDR_MODE_GROUP)
                    {
                        memcpy((uint8_t *)&p_bind_unbind_mapping[i].dst_addr.group_id, (p_command_payload + idx), 2) ;
                    }
                    else
                    {
                        memcpy((uint8_t *)&p_bind_unbind_mapping[i].dst_addr.unicast.ieee_addr, (p_command_payload + idx), 8);
                        idx += 8;
                        p_bind_unbind_mapping[i].dst_addr.unicast.end_point = p_command_payload[idx];
                        idx += 1;
                    }
                    info_color(LOG_RED, "ieee addr 0x%02x%02x%02x%02x%02x%02x%02x%02x, src_ep %d clusterID 0x%04x\n",
                               p_bind_unbind_mapping[i].bind_info.src_ieee_addr[7], p_bind_unbind_mapping[i].bind_info.src_ieee_addr[6],
                               p_bind_unbind_mapping[i].bind_info.src_ieee_addr[5], p_bind_unbind_mapping[i].bind_info.src_ieee_addr[4],
                               p_bind_unbind_mapping[i].bind_info.src_ieee_addr[3], p_bind_unbind_mapping[i].bind_info.src_ieee_addr[2],
                               p_bind_unbind_mapping[i].bind_info.src_ieee_addr[1], p_bind_unbind_mapping[i].bind_info.src_ieee_addr[0],
                               p_bind_unbind_mapping[i].bind_info.src_ep, p_bind_unbind_mapping[i].bind_info.cluster_id);
                    break;
                }
            }
        }

        //not found cause by mapping table full
        if (i == MAX_ADDR_MAPPING_SIZE)
        {
            info_color(LOG_RED, "bind/unbind mapping table_full, clear all\n");
            for (j = 0; j < MAX_ADDR_MAPPING_SIZE; j++)
            {
                info_color(LOG_RED, "ieee addr 0x%02x%02x%02x%02x%02x%02x%02x%02x, src_ep %d clusterID 0x%04x\n",
                           p_bind_unbind_mapping[j].bind_info.src_ieee_addr[7], p_bind_unbind_mapping[j].bind_info.src_ieee_addr[6],
                           p_bind_unbind_mapping[j].bind_info.src_ieee_addr[5], p_bind_unbind_mapping[j].bind_info.src_ieee_addr[4],
                           p_bind_unbind_mapping[j].bind_info.src_ieee_addr[3], p_bind_unbind_mapping[j].bind_info.src_ieee_addr[2],
                           p_bind_unbind_mapping[j].bind_info.src_ieee_addr[1], p_bind_unbind_mapping[j].bind_info.src_ieee_addr[0],
                           p_bind_unbind_mapping[j].bind_info.src_ep, p_bind_unbind_mapping[j].bind_info.cluster_id);
                p_bind_unbind_mapping[j].occupied = 0;
            }
            p_bind_unbind_mapping[0].occupied = 1;
            p_bind_unbind_mapping[0].mapping_addr = dst_addr;
            memcpy((uint8_t *)&p_bind_unbind_mapping[0].bind_info, (uint8_t *)p_bind_info, sizeof(bind_info_param_t));
            (p_bind_unbind_mapping[0].bind_info.dst_addr_mode == ADDR_MODE_GROUP) ?
            (memcpy((uint8_t *)&p_bind_unbind_mapping[i].dst_addr.group_id, (p_command_payload + idx), 2)) :
            (memcpy((uint8_t *)&p_bind_unbind_mapping[i].dst_addr.unicast, (p_command_payload + idx), 9));

        }
    }
    break;

    case RAF_CMD_NETWORK_START_REQUEST:
    {
        uint16_t pan_id;
        uint8_t channel, reset_flag;

        channel = p_command_payload[GW_CMD_HEADER_LEN];
        memcpy((uint8_t *)&pan_id, (p_command_payload + GW_CMD_HEADER_LEN + 1), 2);
        reset_flag = p_command_payload[GW_CMD_HEADER_LEN + 1 + 2];
        if (reset_flag)
        {
            channel |= 0x80;
            group_table_remove_all();
            scene_table_remove_all();
            device_table_remove_all(true, true);
            zc_info_create_cmd_forward(channel, pan_id, true);
        }
    }
    break;
    }
}

void gw_event_check(uint32_t command_id, uint16_t src_addr, uint8_t command_len, uint8_t *p_command_payload)
{

    switch (command_id)
    {
    case (GW_CMD_APP_CMD_RSP_GEN(RAF_CMD_SIMPLE_DESCRIPTOR_REQUEST)): /*Simple descriptor response*/
    {
        zb_zdo_simple_desc_t *p_simple_desc_rsp = (zb_zdo_simple_desc_t *)p_command_payload;
        if (p_simple_desc_rsp->status == 0x00)
        {
            info_color(LOG_RED, "nwk addr 0x%04x, device id addr 0x%04x\n", p_simple_desc_rsp->nwk_Addr, p_simple_desc_rsp->device_id);
            device_table_update_cmd_forward(p_simple_desc_rsp->nwk_Addr, p_simple_desc_rsp->end_point, p_simple_desc_rsp->device_id);
        }
    }
    break;

    case (GW_CMD_APP_CMD_RSP_GEN(RAF_CMD_BIND_REQUEST)):
    {
        uint8_t i, status = p_command_payload[0];

        if (status == 0x00)
        {
            for (i = 0; i < MAX_ADDR_MAPPING_SIZE; i++)
            {
                if ((bind_mapping[i].occupied == 1) && (bind_mapping[i].mapping_addr == src_addr))
                {
                    bind_mapping[i].occupied = 0;
                    break;
                }
            }

            if (i != MAX_ADDR_MAPPING_SIZE)
            {
                info_color(LOG_RED, "device bind OK\nsrc ieee addr 0x%02x%02x%02x%02x%02x%02x%02x%02x, src_ep %d\ncluster id 0x%04x, dest addr mode %d\n",
                           bind_mapping[i].bind_info.src_ieee_addr[7], bind_mapping[i].bind_info.src_ieee_addr[6],
                           bind_mapping[i].bind_info.src_ieee_addr[5], bind_mapping[i].bind_info.src_ieee_addr[4],
                           bind_mapping[i].bind_info.src_ieee_addr[3], bind_mapping[i].bind_info.src_ieee_addr[2],
                           bind_mapping[i].bind_info.src_ieee_addr[1], bind_mapping[i].bind_info.src_ieee_addr[0],
                           bind_mapping[i].bind_info.src_ep, bind_mapping[i].bind_info.cluster_id, bind_mapping[i].bind_info.dst_addr_mode);

                bind_member_add_cmd_forward(&bind_mapping[i].bind_info, &bind_mapping[i].dst_addr);
            }
            else
            {
                info_color(LOG_RED, "addr not found 0x%04x\n", src_addr);
            }
        }
        else
        {
            info_color(LOG_RED, "bind fail, status 0x%02x\n", status);
        }
    }
    break;

    case (GW_CMD_APP_CMD_RSP_GEN(RAF_CMD_UNBIND_REQUEST)):
    {
        uint8_t i, status = p_command_payload[0];

        if (status == 0x00)
        {
            for (i = 0; i < MAX_ADDR_MAPPING_SIZE; i++)
            {
                if ((unbind_mapping[i].occupied == 1) && (unbind_mapping[i].mapping_addr == src_addr))
                {
                    unbind_mapping[i].occupied = 0;
                    break;
                }
            }

            if (i != MAX_ADDR_MAPPING_SIZE)
            {
                info_color(LOG_RED, "device unbind OK\nsrc ieee addr 0x%02x%02x%02x%02x%02x%02x%02x%02x, src_ep %d\ncluster id 0x%04x, dest addr mode %d\n",
                           unbind_mapping[i].bind_info.src_ieee_addr[7], unbind_mapping[i].bind_info.src_ieee_addr[6],
                           unbind_mapping[i].bind_info.src_ieee_addr[5], unbind_mapping[i].bind_info.src_ieee_addr[4],
                           unbind_mapping[i].bind_info.src_ieee_addr[3], unbind_mapping[i].bind_info.src_ieee_addr[2],
                           unbind_mapping[i].bind_info.src_ieee_addr[1], unbind_mapping[i].bind_info.src_ieee_addr[0],
                           unbind_mapping[i].bind_info.src_ep, unbind_mapping[i].bind_info.cluster_id, unbind_mapping[i].bind_info.dst_addr_mode);

                bind_member_remove_cmd_forward(&unbind_mapping[i].bind_info, &unbind_mapping[i].dst_addr);
            }
            else
            {
                info_color(LOG_RED, "addr not found 0x%04x\n", src_addr);
            }
        }
        else
        {
            info_color(LOG_RED, "bind fail, status 0x%02x\n", status);
        }
    }
    break;
    }
}

void gw_table_recover_start(uint8_t table)
{
    uint8_t recover_table = table;

    flash_program(BACKUP_RECOVER_TABLE_ADDRESS, &recover_table, 1);
}

void gw_table_recover_done(void)
{
    uint8_t recover_done = RECOVER_DONE_SEQ;

    flash_program(BACKUP_RECOVER_DONE_ADDRESS, &recover_done, 1);
}

void gw_table_recover(uint32_t recover_address)
{
    uint32_t read_buf[FLASH_PAGE_SIZE >> 2], page_idx;
    uint32_t status;

    flash_sector_erase(recover_address);
    for (page_idx = 0 ; page_idx < FLASH_SECTOR_SIZE; page_idx += FLASH_PAGE_SIZE)
    {
        status = flash_read_page((uint32_t)read_buf, (BACKUP_TABLES_ADDRESS + page_idx));
        while (flash_check_busy());
        if (status != STATUS_SUCCESS)
        {
            info_color(LOG_RED, "flash read failed %d!\n", status);
        }

        status = flash_write_page((uint32_t)read_buf, recover_address + page_idx);
        while (flash_check_busy());
        if (status != STATUS_SUCCESS)
        {
            info_color(LOG_RED, "flash write failed %d!\n", status);
        }
    }


}
void device_table_create_cmd_forward(uint8_t *p_ieee_address, uint16_t nwk_address)
{
    uint32_t command_id = 0;
    device_table_create_param_t device_table_create;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_TABLE_CREATE;
    memcpy(device_table_create.ieee_addr, p_ieee_address, 8);
    device_table_create.nwk_addr = nwk_address;
    gw_cmd_forward(command_id, sizeof(device_table_create_param_t), (uint8_t *)&device_table_create);
}


void device_table_update_cmd_forward(uint16_t address, uint8_t end_point, uint16_t device_id)
{
    uint32_t command_id = 0;
    device_table_update_param_t device_update;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_TABLE_UPDATE;

    device_update.device_id = device_id;
    device_update.nwk_addr = address;
    device_update.end_point = end_point;
    gw_cmd_forward(command_id, sizeof(device_table_update_param_t), (uint8_t *)&device_update);
}

void device_table_delete_cmd_forward(uint8_t *p_ieee_addr)
{
    uint32_t command_id = 0;
    device_table_delete_param_t device_delete;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_TABLE_REMOVE;

    memcpy(device_delete.ieee_addr, p_ieee_addr, 8);
    gw_cmd_forward(command_id, sizeof(device_table_delete_param_t), (uint8_t *)&device_delete);
}

void device_table_get_by_idx_cmd_forward(uint16_t start_idx)
{
    uint32_t command_id = 0;
    device_table_get_by_idx_param_t device_get;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_DEVICE_TABLE_BASE | GW_CMD_DB_DEVICE_TABLE_GET_BY_IDX;

    device_get.start_idx = start_idx;
    gw_cmd_forward(command_id, sizeof(device_table_get_by_idx_param_t), (uint8_t *)&device_get);
}

uint8_t device_table_create(uint8_t *p_ieee_addr, uint16_t nwk_addr)
{
    uint8_t create_success = false;
    uint16_t i;

    for (i = 0; i < DEVICE_TABLE_NUM ; i++)
    {
        if ((p_device_table_info->device_table[i].used == true) &&
                (memcmp(p_device_table_info->device_table[i].device_info.ieee_addr, p_ieee_addr, 8) == 0))
        {
            if (p_device_table_info->device_table[i].device_info.nwk_addr == nwk_addr)
            {
                create_success = true;
                break;
            }
            else /*nwk address different with data base*/
            {
                device_table_delete(8, p_device_table_info->device_table[i].device_info.ieee_addr, NULL);
            }
        }
    }
    if (create_success == false)
    {
        for (i = 0; i < DEVICE_TABLE_NUM ; i++)
        {
            if (p_device_table_info->device_table[i].used != true)
            {
                uint8_t used = true;

                flash_program((uint32_t)&p_device_table_info->device_table[i].used, &used, 1);
                flash_program((uint32_t)&p_device_table_info->device_table[i].device_info.ieee_addr[0], p_ieee_addr, 8);
                flash_program((uint32_t)&p_device_table_info->device_table[i].device_info.nwk_addr, (uint8_t *)&nwk_addr, 2);
                create_success = true;
                break;
            }
        }
    }

    if (create_success == true)
    {
        flush_cache();
    }
    return create_success;
}

uint8_t device_table_update(uint16_t nwk_addr, uint16_t device_id, uint8_t end_point)
{
    uint8_t update_success = false, ieee_addr[8], invalid_addr[8];
    uint16_t i;

    memset(ieee_addr, 0, 8);
    memset(invalid_addr, 0, 8);

    for (i = 0; i < DEVICE_TABLE_NUM ; i++)
    {
        if (p_device_table_info->device_table[i].used == true)
        {
            if (p_device_table_info->device_table[i].device_info.nwk_addr == nwk_addr)
            {
                if ((p_device_table_info->device_table[i].device_info.end_point == end_point) &&
                        (p_device_table_info->device_table[i].device_info.device_id == device_id))
                {
                    /*already saving into DB*/
                    update_success = true;
                    break;
                }
                else if (p_device_table_info->device_table[i].device_info.end_point > 0 &&
                         p_device_table_info->device_table[i].device_info.end_point < 0xFF)
                {
                    /*multi-endpoint: use next empty index to update another endpoint*/
                    if ((memcmp(ieee_addr, invalid_addr, 8) != 0) &&
                            (memcmp(ieee_addr, p_device_table_info->device_table[i].device_info.ieee_addr, 8) != 0))
                    {
                        info_color(LOG_RED, "nwk address conflict! 0x%02x%02x%02x%02x%02x%02x%02x%02x, 0x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                                   ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4], ieee_addr[3], ieee_addr[2], ieee_addr[1], ieee_addr[0],
                                   p_device_table_info->device_table[i].device_info.ieee_addr[7],
                                   p_device_table_info->device_table[i].device_info.ieee_addr[6],
                                   p_device_table_info->device_table[i].device_info.ieee_addr[5],
                                   p_device_table_info->device_table[i].device_info.ieee_addr[4],
                                   p_device_table_info->device_table[i].device_info.ieee_addr[3],
                                   p_device_table_info->device_table[i].device_info.ieee_addr[2],
                                   p_device_table_info->device_table[i].device_info.ieee_addr[1],
                                   p_device_table_info->device_table[i].device_info.ieee_addr[0]);
                    }
                    memcpy(ieee_addr, p_device_table_info->device_table[i].device_info.ieee_addr, 8);
                }
                else
                {
                    /*endpoint was not update yet*/

                    flash_program((uint32_t)&p_device_table_info->device_table[i].device_info.device_id, (uint8_t *)&device_id, 2);
                    flash_program((uint32_t)&p_device_table_info->device_table[i].device_info.end_point, &end_point, 1);
                    flash_program((uint32_t)&p_device_table_info->device_table[i].device_info.nwk_addr, (uint8_t *)&nwk_addr, 2);
                    update_success = true;
                    break;
                }
            }
        }

    }

    if ((update_success == false) && (memcmp(ieee_addr, invalid_addr, 8) != 0/*for multi-endpoint:*/))
    {
        for (i = 0; i < DEVICE_TABLE_NUM ; i++)
        {
            if (p_device_table_info->device_table[i].used != true)
            {
                uint8_t used = true;
                flash_program((uint32_t)&p_device_table_info->device_table[i].used, &used, 1);
                flash_program((uint32_t)&p_device_table_info->device_table[i].device_info.ieee_addr[0], ieee_addr, 8);
                flash_program((uint32_t)&p_device_table_info->device_table[i].device_info.nwk_addr, (uint8_t *)&nwk_addr, 2);
                flash_program((uint32_t)&p_device_table_info->device_table[i].device_info.device_id, (uint8_t *)&device_id, 2);
                flash_program((uint32_t)&p_device_table_info->device_table[i].device_info.end_point, &end_point, 1);
                update_success = true;
                break;
            }
        }
    }
    if (update_success == true)
    {
        flush_cache();
    }

    return update_success;

}

void device_table_copy_to_backup_page(void)
{
    device_table_info_t *p_new_device_table_info = (device_table_info_t *)BACKUP_TABLES_ADDRESS;
    uint16_t i, j = 0;


    for (i = 0; i < DEVICE_TABLE_NUM; i++)
    {
        if (p_device_table_info->device_table[i].used == true)
        {
            uint8_t used = true;
            flash_program((uint32_t)&p_new_device_table_info->device_table[j].used, &used, 1);
            flash_program((uint32_t)&p_new_device_table_info->device_table[j].device_info,
                          (uint8_t *)&p_device_table_info->device_table[i].device_info, sizeof(device_info_param_t));

            if ((p_device_table_info->device_table[i].device_name.name_len != 0) &&
                    (p_device_table_info->device_table[i].device_name.name_len != 0xff))
            {
                flash_program((uint32_t)&p_new_device_table_info->device_table[j].device_name.name_len, &p_device_table_info->device_table[i].device_name.name_len, 1);
                flash_program((uint32_t)&p_new_device_table_info->device_table[j].device_name.name[0], &p_device_table_info->device_table[i].device_name.name[0], p_device_table_info->device_table[i].device_name.name_len);
            }

            if ((p_device_table_info->device_table[i].ep_name.name_len != 0) &&
                    (p_device_table_info->device_table[i].ep_name.name_len != 0xff))
            {
                flash_program((uint32_t)&p_new_device_table_info->device_table[j].ep_name.name_len, &p_device_table_info->device_table[i].ep_name.name_len, 1);
                flash_program((uint32_t)&p_new_device_table_info->device_table[j].ep_name.name[0], &p_device_table_info->device_table[i].ep_name.name[0], p_device_table_info->device_table[i].ep_name.name_len);
            }
            j++;
        }
    }
    flush_cache();
}

uint8_t device_table_delete(uint8_t addr_len, uint8_t *p_addr, uint8_t end_point)
{
    device_table_info_t *p_new_info = (device_table_info_t *)BACKUP_TABLES_ADDRESS;
    uint16_t i, j = 0, nwk_addr;
    uint8_t target_found = false;

    if (addr_len == 2)
    {
        memcpy((uint8_t *)&nwk_addr, p_addr, sizeof(uint8_t) * 2);
    }

    flash_sector_erase(BACKUP_TABLES_ADDRESS);

    for (i = 0; i < DEVICE_TABLE_NUM; i++)
    {
        if (p_device_table_info->device_table[i].used == true)
        {
            if (((addr_len == 2) &&
                    ((p_device_table_info->device_table[i].device_info.nwk_addr == nwk_addr) &&
                     (p_device_table_info->device_table[i].device_info.end_point == end_point))) ||
                    ((addr_len == 8) &&
                     (memcmp(p_addr, p_device_table_info->device_table[i].device_info.ieee_addr, 8) == 0)))
            {
                target_found = true;
            }
            else
            {
                uint8_t used = true;
                flash_program((uint32_t)&p_new_info->device_table[j].used, &used, 1);


                flash_program((uint32_t)&p_new_info->device_table[j].device_info,
                              (uint8_t *)&p_device_table_info->device_table[i].device_info, sizeof(device_info_param_t));
                if ((p_device_table_info->device_table[i].device_name.name_len != 0) &&
                        (p_device_table_info->device_table[i].device_name.name_len != 0xff))
                {
                    flash_program((uint32_t)&p_new_info->device_table[j].device_name.name_len, &p_device_table_info->device_table[i].device_name.name_len, 1);
                    flash_program((uint32_t)&p_new_info->device_table[j].device_name.name[0], &p_device_table_info->device_table[i].device_name.name[0], p_device_table_info->device_table[i].device_name.name_len);
                }

                if ((p_device_table_info->device_table[i].ep_name.name_len != 0) &&
                        (p_device_table_info->device_table[i].ep_name.name_len != 0xff))
                {
                    flash_program((uint32_t)&p_new_info->device_table[j].ep_name.name_len, &p_device_table_info->device_table[i].ep_name.name_len, 1);
                    flash_program((uint32_t)&p_new_info->device_table[j].ep_name.name[0], &p_device_table_info->device_table[i].ep_name.name[0], p_device_table_info->device_table[i].ep_name.name_len);
                }
                j++;
            }
        }
    }

    if (target_found == true)
    {
        bind_table_copy_to_backup_page();
        gw_table_recover_start(DEVICE_BIND_TABLE);
        gw_table_recover((uint32_t)p_device_table_info);
        gw_table_recover_done();
        flush_cache();
    }
    return target_found;
}

uint8_t device_table_get(device_table_get_rsp_t *p_device_data, uint8_t start_idx)
{
    uint16_t i;

    p_device_data->status = 0x01; /*fail*/
    for (i = start_idx; i < DEVICE_TABLE_NUM; i++)
    {
        if (p_device_table_info->device_table[i].used == true)
        {
            p_device_data->status = 0x00;

            memcpy((uint8_t *)&p_device_data->device_info, (uint8_t *)&p_device_table_info->device_table[i].device_info, sizeof(device_info_param_t));

            if (p_device_table_info->device_table[i].device_name.name_len != 0xFF &&
                    p_device_table_info->device_table[i].device_name.name_len != 0)
            {
                p_device_data->device_name.name_len = p_device_table_info->device_table[i].device_name.name_len;
                memcpy(&p_device_data->device_name.name, (uint8_t *)&p_device_table_info->device_table[i].device_name.name, p_device_table_info->device_table[i].device_name.name_len);
            }
            else
            {
                p_device_data->device_name.name_len = 0;
            }

            if (p_device_table_info->device_table[i].ep_name.name_len != 0xFF &&
                    p_device_table_info->device_table[i].ep_name.name_len != 0)
            {
                p_device_data->ep_name.name_len = p_device_table_info->device_table[i].ep_name.name_len;
                memcpy(&p_device_data->ep_name.name, (uint8_t *)&p_device_table_info->device_table[i].ep_name.name, p_device_table_info->device_table[i].ep_name.name_len);
            }
            else
            {
                p_device_data->ep_name.name_len = 0;
            }

            break;
        }
    }

    return i;
}

uint8_t device_table_device_rename(uint16_t nwk_addr, uint8_t len, uint8_t *p_name)
{
    uint16_t i;
    uint8_t rename_success = false;

    for (i = 0; i < DEVICE_TABLE_NUM; i++)
    {
        if (p_device_table_info->device_table[i].used == true)
        {
            if (p_device_table_info->device_table[i].device_info.nwk_addr == nwk_addr)
            {
                if (p_device_table_info->device_table[i].device_name.name_len == FLASH_DEFAULT_VALUE)
                {
                    if ((len < DEVICE_NAME_LEN) && (rename_success == false))
                    {
                        flash_program((uint32_t)&p_device_table_info->device_table[i].device_name.name_len, &len, 1);
                        flash_program((uint32_t)&p_device_table_info->device_table[i].device_name.name[0], p_name, len);
                        rename_success = true;
                    }
                    else
                    {
                        info_color(LOG_RED, "len of device name %d, rename_success %d\n", len, rename_success);
                    }
                }
                else
                {
                    uint8_t ieee_addr[8], end_point, ep_name[END_POINT_NAME_LEN], ep_name_len = 0;
                    uint16_t device_id;


                    info_color(LOG_RED, "device name already program, update it after flash erase\n");
                    memcpy(ieee_addr, p_device_table_info->device_table[i].device_info.ieee_addr, 8);
                    device_id = p_device_table_info->device_table[i].device_info.device_id;
                    end_point = p_device_table_info->device_table[i].device_info.end_point;

                    if (p_device_table_info->device_table[i].ep_name.name_len > 0)
                    {
                        ep_name_len = p_device_table_info->device_table[i].ep_name.name_len;

                        ep_name_len = (ep_name_len == FLASH_DEFAULT_VALUE) ? 0 : ep_name_len;
                        memcpy(ep_name, p_device_table_info->device_table[i].ep_name.name, ep_name_len);
                    }
                    device_table_delete(2, (uint8_t *)&nwk_addr, end_point);
                    device_table_create(ieee_addr, nwk_addr);
                    device_table_update(nwk_addr, device_id, end_point);
                    info_color(LOG_RED, "ep_name_len %d\n", ep_name_len);

                    if (ep_name_len > 0)
                    {
                        device_table_end_point_rename(nwk_addr, end_point, ep_name_len, ep_name);
                    }

                    if (rename_success == false)
                    {
                        if (device_table_device_rename(nwk_addr, len, p_name) == true)
                        {
                            rename_success = true;
                        }
                    }
                }
            }
        }
    }

    if (rename_success)
    {
        flush_cache();
    }
    return rename_success;
}

uint8_t device_table_end_point_rename(uint16_t nwk_addr, uint8_t end_point, uint8_t len, uint8_t *p_name)
{
    uint16_t i;
    uint8_t rename_success = false;

    for (i = 0; i < DEVICE_TABLE_NUM; i++)
    {
        if (p_device_table_info->device_table[i].used == true)
        {
            if ((p_device_table_info->device_table[i].device_info.nwk_addr == nwk_addr) &&
                    (p_device_table_info->device_table[i].device_info.end_point == end_point))
            {
                if (p_device_table_info->device_table[i].ep_name.name_len == FLASH_DEFAULT_VALUE)
                {
                    if (len < DEVICE_NAME_LEN)
                    {
                        flash_program((uint32_t)&p_device_table_info->device_table[i].ep_name.name_len, &len, 1);
                        flash_program((uint32_t)&p_device_table_info->device_table[i].ep_name.name[0], p_name, len);
                        rename_success = true;
                        break;
                    }
                    else
                    {
                        info_color(LOG_RED, "len of device name invalid: %d\n", len);
                    }
                }
                else
                {
                    uint8_t ieee_addr[8], device_name[DEVICE_NAME_LEN], device_name_len = 0;
                    uint16_t device_id;

                    info_color(LOG_RED, "end point name already program, update it after flash erase\n");
                    memcpy(ieee_addr, p_device_table_info->device_table[i].device_info.ieee_addr, 8);
                    device_id = p_device_table_info->device_table[i].device_info.device_id;

                    if (p_device_table_info->device_table[i].device_name.name_len != FLASH_DEFAULT_VALUE)
                    {
                        device_name_len = p_device_table_info->device_table[i].device_name.name_len;
                        device_name_len = (device_name_len == FLASH_DEFAULT_VALUE) ? 0 : device_name_len;

                        memcpy(device_name, p_device_table_info->device_table[i].device_name.name, device_name_len);
                    }
                    device_table_delete(2, (uint8_t *)&nwk_addr, end_point);
                    device_table_create(ieee_addr, nwk_addr);
                    device_table_update(nwk_addr, device_id, end_point);
                    if (device_name_len > 0)
                    {
                        device_table_device_rename(nwk_addr, device_name_len, device_name);
                    }
                    if (device_table_end_point_rename(nwk_addr, end_point, len, p_name) == true)
                    {
                        rename_success = true;
                    }
                    break;
                }
            }
        }
    }

    if (rename_success)
    {
        flush_cache();
    }
    return rename_success;
}

void device_table_remove_all(uint8_t bind_remove, uint8_t zc_info_remove)
{
    device_table_info_t *p_new_device_table_info = (device_table_info_t *)BACKUP_TABLES_ADDRESS;
    uint16_t i, j = 0;

    flash_sector_erase(BACKUP_TABLES_ADDRESS);

    if (bind_remove == false)
    {
        bind_table_copy_to_backup_page();
    }

    if (zc_info_remove == false)
    {
        for (i = 0; i < DEVICE_TABLE_NUM; i++)
        {
            if (p_device_table_info->device_table[i].used == true)
            {
                if (p_device_table_info->device_table[i].device_info.nwk_addr == 0x0000)
                {
                    uint8_t used = true;
                    flash_program((uint32_t)&p_new_device_table_info->device_table[j].used, &used, 1);
                    flash_program((uint32_t)&p_new_device_table_info->device_table[j].device_info,
                                  (uint8_t *)&p_device_table_info->device_table[i].device_info, sizeof(device_info_param_t));
                    j++;
                    break;
                }
            }
        }
    }

    if ((bind_remove == false) || (zc_info_remove == false))
    {
        gw_table_recover_start(DEVICE_BIND_TABLE);
        gw_table_recover((uint32_t)p_device_table_info);
        gw_table_recover_done();
        flush_cache();
    }
    else
    {
        flash_sector_erase(DEVICE_TABLE_FLASH_ADDRESS);
    }
}


void group_member_add_cmd_forward(uint16_t address, uint8_t end_point, uint16_t group_id)
{
    uint32_t command_id = 0;
    group_member_add_remove_param_t group_member_add;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_GROUP_TABLE_BASE | GW_CMD_DB_GROUP_TABLE_MEMBER_ADD;

    group_member_add.group_id = group_id;
    group_member_add.nwk_addr = address;
    group_member_add.end_point = end_point;
    gw_cmd_forward(command_id, sizeof(group_member_add_remove_param_t), (uint8_t *)&group_member_add);
}

void group_member_remove_cmd_forward(uint16_t address, uint8_t end_point, uint16_t group_id)
{
    uint32_t command_id = 0;
    group_member_add_remove_param_t group_member_remove;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_GROUP_TABLE_BASE | GW_CMD_DB_GROUP_TABLE_MEMBER_REMOVE;

    group_member_remove.group_id = group_id;
    group_member_remove.nwk_addr = address;
    group_member_remove.end_point = end_point;

    gw_cmd_forward(command_id, sizeof(group_member_add_remove_param_t), (uint8_t *)&group_member_remove);
}

void group_all_groups_remove_cmd_forward(uint16_t address, uint8_t end_point)
{
    uint32_t command_id = 0;
    group_all_groups_remove_param_t device_all_groups_remove;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_GROUP_TABLE_BASE | GW_CMD_DB_GROUP_TABLE_DEVICE_ALL_GROUPS_REMOVE;

    device_all_groups_remove.nwk_addr = address;
    device_all_groups_remove.end_point = end_point;

    gw_cmd_forward(command_id, sizeof(group_all_groups_remove_param_t), (uint8_t *)&device_all_groups_remove);
}

void group_table_get_by_idx_cmd_forward(uint16_t start_idx)
{
    uint32_t command_id = 0;
    group_table_get_by_idx_param_t group_get;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_GROUP_TABLE_BASE | GW_CMD_DB_GROUP_TABLE_GET_BY_IDX;

    group_get.start_idx = start_idx;
    gw_cmd_forward(command_id, sizeof(device_table_get_by_idx_param_t), (uint8_t *)&group_get);
}


uint8_t group_table_create(uint16_t group_id)
{
    uint8_t create_success = false;
    uint16_t i;

    for (i = 0; i < GROUP_TABLE_NUM ; i++)
    {
        if ((p_group_table_info->group_table[i].used == true) &&
                (p_group_table_info->group_table[i].group_id == group_id))
        {
            create_success = true;
            break;
        }
    }

    if (create_success == false)
    {
        for (i = 0; i < GROUP_TABLE_NUM ; i++)
        {
            if (p_group_table_info->group_table[i].used != true)
            {
                uint8_t used = true;
                flash_program((uint32_t)&p_group_table_info->group_table[i].used, &used, 1);
                flash_program((uint32_t)&p_group_table_info->group_table[i].group_id, (uint8_t *)&group_id, 2);
                create_success = true;
                break;
            }
        }
    }

    if (create_success == true)
    {
        flush_cache();
    }
    return create_success;
}

uint8_t group_table_update(uint16_t group_id, uint16_t nwk_addr, uint8_t end_point)
{
    uint8_t update_success = false;
    uint16_t i, j;

    for (i = 0; i < GROUP_TABLE_NUM ; i++)
    {
        if ((p_group_table_info->group_table[i].used == true) &&
                (p_group_table_info->group_table[i].group_id == group_id))
        {
            //group table found, check member list
            for (j = 0; j < GROUP_TABLE_MEMBER_NUM ; j++)
            {
                if (p_group_table_info->group_table[i].group_member[j].used)
                {
                    if (p_group_table_info->group_table[i].group_member[j].end_point == end_point &&
                            p_group_table_info->group_table[i].group_member[j].nwk_addr == nwk_addr)
                    {
                        //already exist
                        update_success = true;
                        break;
                    }
                }
            }

            if (j == GROUP_TABLE_MEMBER_NUM)
            {
                //device not found in member list, update it in empty list.
                for (j = 0; j < GROUP_TABLE_MEMBER_NUM ; j++)
                {
                    if (p_group_table_info->group_table[i].group_member[j].used != 1)
                    {
                        uint8_t used = true;
                        flash_program((uint32_t)&p_group_table_info->group_table[i].group_member[j].used, &used, 1);
                        flash_program((uint32_t)&p_group_table_info->group_table[i].group_member[j].end_point, &end_point, 1);
                        flash_program((uint32_t)&p_group_table_info->group_table[i].group_member[j].nwk_addr, (uint8_t *)&nwk_addr, 2);
                        update_success = true;
                        break;
                    }
                }
            }
            break;
        }
    }

    if (i == GROUP_TABLE_NUM) /*group table not found*/
    {
        //create group table
        if (group_table_create(group_id) == true)
        {
            //update again
            group_member_add_cmd_forward(nwk_addr, end_point, group_id);
        }
        else
        {
            info_color(LOG_RED, "group table full\n");
        }
    }

    if (update_success == true)
    {
        flush_cache();
    }

    return update_success;

}

void group_table_delete_by_nwk_addr(uint16_t nwk_addr)
{
    group_table_info_t *p_new_group_table_info = (group_table_info_t *)BACKUP_TABLES_ADDRESS;
    uint16_t i, j = 0, k;
    uint8_t used = 1, group_update = 0, target_found = false;

    flash_sector_erase(BACKUP_TABLES_ADDRESS);

    for (i = 0; i < GROUP_TABLE_NUM; i++)
    {
        if (p_group_table_info->group_table[i].used == true)
        {
            /* move the members of group table which has different address & endpoint
               with target address & endpoint into new flash page */
            group_update = 0;
            for (k = 0; k < GROUP_TABLE_MEMBER_NUM; k++)
            {
                if (p_group_table_info->group_table[i].group_member[k].used == 1)
                {
                    if (p_group_table_info->group_table[i].group_member[k].nwk_addr == nwk_addr)
                    {
                        //target found, do nothing
                        target_found = true;
                    }
                    else
                    {
                        flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_member[k].used, &used, 1);
                        flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_member[k].end_point,
                                      &p_group_table_info->group_table[i].group_member[k].end_point, 1);
                        flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_member[k].nwk_addr,
                                      (uint8_t *)&p_group_table_info->group_table[i].group_member[k].nwk_addr, 2);
                        group_update = 1;
                    }
                }
            }

            if (group_update == 1)
            {
                flash_program((uint32_t)&p_new_group_table_info->group_table[j].used, &used, 1);
                flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_id, (uint8_t *)&p_group_table_info->group_table[i].group_id, 2);

                if ((p_group_table_info->group_table[i].group_name_len != 0) &&
                        (p_group_table_info->group_table[i].group_name_len != 0xff))
                {
                    flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_name_len, &p_group_table_info->group_table[i].group_name_len, 1);
                    flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_name[0], &p_group_table_info->group_table[i].group_name[0], p_group_table_info->group_table[i].group_name_len);
                }
                j++;
            }
        }
    }

    if (target_found == true)
    {
        gw_table_recover_start(GROUP_TABLE);
        gw_table_recover((uint32_t)p_group_table_info);
        gw_table_recover_done();
        flush_cache();
    }
}

void group_table_delete(uint16_t group_id, uint16_t nwk_addr, uint8_t end_point)
{
    group_table_info_t *p_new_info = (group_table_info_t *)BACKUP_TABLES_ADDRESS;
    uint16_t i, j = 0, k;
    uint8_t used = 1, group_update = false, target_found = false;

    flash_sector_erase(BACKUP_TABLES_ADDRESS);

    for (i = 0; i < GROUP_TABLE_NUM; i++)
    {
        if (p_group_table_info->group_table[i].used == true)
        {
            if ((group_id == ALL_GROUP) ||
                    (p_group_table_info->group_table[i].group_id == group_id))
            {
                /* move the members of group table which has different address & endpoint
                   with target address & endpoint into new flash page */
                group_update = false;
                for (k = 0; k < GROUP_TABLE_MEMBER_NUM; k++)
                {
                    if (p_group_table_info->group_table[i].group_member[k].used == 1)
                    {
                        if ((p_group_table_info->group_table[i].group_member[k].end_point == end_point) &&
                                (p_group_table_info->group_table[i].group_member[k].nwk_addr == nwk_addr))
                        {
                            //target found
                            target_found = true;
                        }
                        else
                        {
                            flash_program((uint32_t)&p_new_info->group_table[j].group_member[k].used, &used, 1);
                            flash_program((uint32_t)&p_new_info->group_table[j].group_member[k].end_point,
                                          &p_group_table_info->group_table[i].group_member[k].end_point, 1);
                            flash_program((uint32_t)&p_new_info->group_table[j].group_member[k].nwk_addr,
                                          (uint8_t *)&p_group_table_info->group_table[i].group_member[k].nwk_addr, 2);
                            group_update = true;
                        }
                    }
                }

                if (group_update == true)
                {
                    flash_program((uint32_t)&p_new_info->group_table[j].used, &used, 1);
                    flash_program((uint32_t)&p_new_info->group_table[j].group_id, (uint8_t *)&p_group_table_info->group_table[i].group_id, 2);

                    if ((p_group_table_info->group_table[i].group_name_len != 0) &&
                            (p_group_table_info->group_table[i].group_name_len != 0xff))
                    {
                        flash_program((uint32_t)&p_new_info->group_table[j].group_name_len, &p_group_table_info->group_table[i].group_name_len, 1);
                        flash_program((uint32_t)&p_new_info->group_table[j].group_name[0], &p_group_table_info->group_table[i].group_name[0], p_group_table_info->group_table[i].group_name_len);
                    }
                    j++;
                }
            }
            else
            {
                /* because group id of this table is different with target group id,
                   program all the members into new flash page*/
                flash_program((uint32_t)&p_new_info->group_table[j].used, &used, 1);
                flash_program((uint32_t)&p_new_info->group_table[j].group_id,
                              (uint8_t *)&p_group_table_info->group_table[i].group_id, 2);

                if ((p_group_table_info->group_table[i].group_name_len != 0) &&
                        (p_group_table_info->group_table[i].group_name_len != 0xff))
                {
                    flash_program((uint32_t)&p_new_info->group_table[j].group_name_len, &p_group_table_info->group_table[i].group_name_len, 1);
                    flash_program((uint32_t)&p_new_info->group_table[j].group_name[0], &p_group_table_info->group_table[i].group_name[0], p_group_table_info->group_table[i].group_name_len);
                }

                for (k = 0; k < GROUP_TABLE_MEMBER_NUM; k++)
                {
                    if (p_group_table_info->group_table[i].group_member[k].used == 1)
                    {
                        flash_program((uint32_t)&p_new_info->group_table[j].group_member[k].used, &used, 1);
                        flash_program((uint32_t)&p_new_info->group_table[j].group_member[k].end_point,
                                      &p_group_table_info->group_table[i].group_member[k].end_point, 1);
                        flash_program((uint32_t)&p_new_info->group_table[j].group_member[k].nwk_addr,
                                      (uint8_t *)&p_group_table_info->group_table[i].group_member[k].nwk_addr, 2);
                    }
                }
                j++;
            }

        }
    }

    if (target_found == true)
    {
        gw_table_recover_start(GROUP_TABLE);
        gw_table_recover((uint32_t)p_group_table_info);
        gw_table_recover_done();
        flush_cache();
    }
}

uint8_t group_table_rename(uint16_t group_id, uint8_t len, uint8_t *p_name)
{
    group_table_info_t *p_new_group_table_info = (group_table_info_t *)BACKUP_TABLES_ADDRESS;;
    uint16_t i, j = 0, k;
    uint8_t rename_status = RENAME_FAIL, used = 1, already_program = false;

    for (i = 0; i < GROUP_TABLE_NUM; i++)
    {
        if (p_group_table_info->group_table[i].used == true)
        {
            if (p_group_table_info->group_table[i].group_id == group_id)
            {
                if (p_group_table_info->group_table[i].group_name_len == FLASH_DEFAULT_VALUE)
                {
                    if (len < GROUP_NAME_LEN)
                    {
                        flash_program((uint32_t)&p_group_table_info->group_table[i].group_name_len, &len, 1);
                        flash_program((uint32_t)&p_group_table_info->group_table[i].group_name[0], p_name, len);
                        rename_status = RENAME_SUCCESS;
                    }
                    else
                    {
                        info_color(LOG_RED, "len of group name invalid: %d\n", len);
                        rename_status = RENAME_INVALID_LEN;
                    }
                }
                else
                {
                    already_program = true;
                }
                break;
            }
        }
    }

    if (i == GROUP_TABLE_NUM)
    {
        rename_status = RENAME_NOT_FOUND;
    }
    else if (already_program == true)
    {
        /*erase group table and rename the group into group table*/

        flash_sector_erase(BACKUP_TABLES_ADDRESS);
        /*find target group*/
        for (i = 0; i < GROUP_TABLE_NUM; i++)
        {
            if (p_group_table_info->group_table[i].used == true)
            {
                flash_program((uint32_t)&p_new_group_table_info->group_table[j].used, &used, 1);
                flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_id,
                              (uint8_t *)&p_group_table_info->group_table[i].group_id, 2);

                if (group_id == p_group_table_info->group_table[i].group_id)
                {
                    /*target group found, rename it*/
                    flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_name_len, &len, 1);
                    flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_name[0], &p_name[0], len);
                    rename_status = RENAME_SUCCESS;
                }
                else if ((p_group_table_info->group_table[i].group_name_len != 0) &&
                         (p_group_table_info->group_table[i].group_name_len != 0xff))
                {
                    /*copy the original name of group into new group table*/
                    flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_name_len, &p_group_table_info->group_table[i].group_name_len, 1);
                    flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_name[0], &p_group_table_info->group_table[i].group_name[0], p_group_table_info->group_table[i].group_name_len);
                }

                for (k = 0; k < GROUP_TABLE_MEMBER_NUM; k++)
                {
                    /*copy member of group into new group table*/
                    if (p_group_table_info->group_table[i].group_member[k].used == 1)
                    {
                        flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_member[k].used, &used, 1);
                        flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_member[k].end_point,
                                      &p_group_table_info->group_table[i].group_member[k].end_point, 1);
                        flash_program((uint32_t)&p_new_group_table_info->group_table[j].group_member[k].nwk_addr,
                                      (uint8_t *)&p_group_table_info->group_table[i].group_member[k].nwk_addr, 2);
                    }
                }
                j++;
            }
        }

        if (rename_status == RENAME_SUCCESS)
        {
            gw_table_recover_start(GROUP_TABLE);
            gw_table_recover((uint32_t)p_group_table_info);
            gw_table_recover_done();
        }

    }

    if (rename_status == RENAME_SUCCESS)
    {
        flush_cache();
    }

    return rename_status;
}

uint8_t group_table_get(uint8_t *p_group_data, uint8_t *p_group_data_len, uint8_t start_idx)
{
    uint16_t i, j, data_idx = 0;

    for (i = start_idx; i < GROUP_TABLE_NUM; i++)
    {
        if (p_group_table_info->group_table[i].used == 0x1)
        {
            memcpy(&p_group_data[data_idx], (uint8_t *)&p_group_table_info->group_table[i].group_id, sizeof(p_group_table_info->group_table[i].group_id));
            data_idx += sizeof(p_group_table_info->group_table[i].group_id);
            if (p_group_table_info->group_table[i].group_name_len != 0xFF &&
                    p_group_table_info->group_table[i].group_name_len != 0)
            {
                memcpy(&p_group_data[data_idx], (uint8_t *)&p_group_table_info->group_table[i].group_name_len, sizeof(p_group_table_info->group_table[i].group_name_len));
                data_idx += sizeof(p_group_table_info->group_table[i].group_name_len);

                memcpy(&p_group_data[data_idx], p_group_table_info->group_table[i].group_name, p_group_table_info->group_table[i].group_name_len);
                data_idx += p_group_table_info->group_table[i].group_name_len;
            }
            else
            {
                p_group_data[data_idx] = 0;
                data_idx += 1;
            }

            for (j = 0; j < GROUP_TABLE_MEMBER_NUM; j++)
            {
                if (p_group_table_info->group_table[i].group_member[j].used == 0x01)
                {
                    memcpy(&p_group_data[data_idx], (uint8_t *)&p_group_table_info->group_table[i].group_member[j].nwk_addr, sizeof(p_group_table_info->group_table[i].group_member[j].nwk_addr));
                    data_idx += sizeof(p_group_table_info->group_table[i].group_member[j].nwk_addr);
                    memcpy(&p_group_data[data_idx], (uint8_t *)&p_group_table_info->group_table[i].group_member[j].end_point, sizeof(p_group_table_info->group_table[i].group_member[j].end_point));
                    data_idx += sizeof(p_group_table_info->group_table[i].group_member[j].end_point);
                }
            }

            break;
        }
    }
    *p_group_data_len = data_idx;
    return i;
}

uint32_t group_table_remove_all(void)
{
    uint8_t status;
    status = flash_sector_erase(GROUP_TABLE_FLASH_ADDRESS);

    status = (status == STATUS_SUCCESS) ? 0x00/*SUCCESS*/ :
             (status == STATUS_INVALID_PARAM) ? 0x85/*INVALID_FIELD*/ : 0x01;/*FAILURE*/;
    return status;

}

void scene_member_add_cmd_forward(uint16_t address, uint8_t end_point, uint16_t group_id, uint8_t scene_id)
{
    uint32_t command_id = 0;
    scene_member_add_remove_param_t scene_member_add;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_SCENE_TABLE_BASE | GW_CMD_DB_SCENE_TABLE_MEMBER_ADD;

    scene_member_add.group_id = group_id;
    scene_member_add.scene_id = scene_id;
    scene_member_add.nwk_addr = address;
    scene_member_add.end_point = end_point;
    gw_cmd_forward(command_id, sizeof(scene_member_add_remove_param_t), (uint8_t *)&scene_member_add);
}

void scene_member_remove_cmd_forward(uint16_t address, uint8_t end_point, uint16_t group_id, uint8_t scene_id)
{
    uint32_t command_id = 0;
    scene_member_add_remove_param_t scene_member_remove;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_SCENE_TABLE_BASE | GW_CMD_DB_SCENE_TABLE_MEMBER_REMOVE;

    scene_member_remove.group_id = group_id;
    scene_member_remove.scene_id = scene_id;
    scene_member_remove.nwk_addr = address;
    scene_member_remove.end_point = end_point;

    gw_cmd_forward(command_id, sizeof(scene_member_add_remove_param_t), (uint8_t *)&scene_member_remove);
}

void scene_table_get_by_idx_cmd_forward(uint16_t start_idx)
{
    uint32_t command_id = 0;
    scene_table_get_by_idx_param_t scene_get;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_SCENE_TABLE_BASE | GW_CMD_DB_SCENE_TABLE_GET_BY_IDX;

    scene_get.start_idx = start_idx;
    gw_cmd_forward(command_id, sizeof(device_table_get_by_idx_param_t), (uint8_t *)&scene_get);
}

uint8_t scene_table_create(uint16_t group_id, uint8_t scene_id)
{
    uint8_t create_success = false;
    uint16_t i;

    for (i = 0; i < SCENE_TABLE_NUM ; i++)
    {
        if ((p_scene_table_info->scene_table[i].used == true) &&
                (p_scene_table_info->scene_table[i].group_id == group_id) &&
                (p_scene_table_info->scene_table[i].scene_id == scene_id))
        {
            create_success = true;
            break;
        }
    }

    if (create_success == false)
    {
        for (i = 0; i < SCENE_TABLE_NUM ; i++)
        {
            if (p_scene_table_info->scene_table[i].used != true)
            {
                uint8_t used = true;
                flash_program((uint32_t)&p_scene_table_info->scene_table[i].used, &used, 1);
                flash_program((uint32_t)&p_scene_table_info->scene_table[i].group_id, (uint8_t *)&group_id, 2);
                flash_program((uint32_t)&p_scene_table_info->scene_table[i].scene_id, (uint8_t *)&scene_id, 1);
                create_success = true;
                break;
            }
        }
    }


    if (create_success == true)
    {
        flush_cache();
    }

    return create_success;
}

uint8_t scene_table_update(uint16_t group_id, uint8_t scene_id, uint16_t nwk_addr, uint8_t end_point)
{
    uint8_t update_success = false;
    uint16_t i, j;

    for (i = 0; i < SCENE_TABLE_NUM ; i++)
    {
        if ((p_scene_table_info->scene_table[i].used == true) &&
                (p_scene_table_info->scene_table[i].group_id == group_id) &&
                (p_scene_table_info->scene_table[i].scene_id == scene_id))
        {
            //scene table found, check member list
            for (j = 0; j < SCENE_TABLE_MEMBER_NUM ; j++)
            {
                if (p_scene_table_info->scene_table[i].scene_member[j].used)
                {
                    if (p_scene_table_info->scene_table[i].scene_member[j].end_point == end_point &&
                            p_scene_table_info->scene_table[i].scene_member[j].nwk_addr == nwk_addr)
                    {
                        //already exist
                        update_success = true;
                        break;
                    }
                }
            }

            if (j == SCENE_TABLE_MEMBER_NUM)
            {
                //scene not found in member list, update it in empty index.
                for (j = 0; j < SCENE_TABLE_MEMBER_NUM ; j++)
                {
                    if (p_scene_table_info->scene_table[i].scene_member[j].used != 1)
                    {
                        uint8_t used = true;
                        flash_program((uint32_t)&p_scene_table_info->scene_table[i].scene_member[j].used, &used, 1);
                        flash_program((uint32_t)&p_scene_table_info->scene_table[i].scene_member[j].end_point, &end_point, 1);
                        flash_program((uint32_t)&p_scene_table_info->scene_table[i].scene_member[j].nwk_addr, (uint8_t *)&nwk_addr, 2);
                        update_success = true;
                        break;
                    }
                }
            }
            break;
        }
    }

    if (i == SCENE_TABLE_NUM) /*scene table not found*/
    {
        //create scene table
        if (scene_table_create(group_id, scene_id) == true)
        {
            //update again
            scene_member_add_cmd_forward(nwk_addr, end_point, group_id, scene_id);
            update_success = true;
        }
        else
        {
            info_color(LOG_RED, "scene table full\n");
        }
    }

    if (update_success == true)
    {
        flush_cache();
    }

    return update_success;

}

void scene_table_delete_by_nwk_addr(uint16_t nwk_addr)
{
    scene_table_info_t *p_new_scene_table_info = (scene_table_info_t *)BACKUP_TABLES_ADDRESS;
    uint16_t i, j = 0, k;
    uint8_t used = 1, group_update = 0, target_found = false;

    flash_sector_erase(BACKUP_TABLES_ADDRESS);

    for (i = 0; i < SCENE_TABLE_NUM; i++)
    {
        if (p_scene_table_info->scene_table[i].used == true)
        {
            /* move the members of scene table which has different address & endpoint
               with target address & endpoint into new flash page */
            if (p_scene_table_info->scene_table[i].scene_member[k].used == 1)
            {
                group_update = 0;
                for (k = 0; k < SCENE_TABLE_MEMBER_NUM; k++)
                {
                    if  (p_scene_table_info->scene_table[i].scene_member[k].nwk_addr == nwk_addr)
                    {
                        //target found
                        target_found = true;
                    }
                    else
                    {
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].used, &used, 1);
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].end_point,
                                      &p_scene_table_info->scene_table[i].scene_member[k].end_point, 1);
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].nwk_addr,
                                      (uint8_t *)&p_scene_table_info->scene_table[i].scene_member[k].nwk_addr, 2);
                        group_update = 1;
                    }
                }

                if (group_update == 1)
                {
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].used, &used, 1);
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].group_id, (uint8_t *)&p_scene_table_info->scene_table[i].group_id, 2);
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_id, (uint8_t *)&p_scene_table_info->scene_table[i].scene_id, 1);

                    if ((p_scene_table_info->scene_table[i].scene_name_len != 0) &&
                            (p_scene_table_info->scene_table[i].scene_name_len != 0xff))
                    {
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_name_len, &p_scene_table_info->scene_table[i].scene_name_len, 1);
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_name[0], &p_scene_table_info->scene_table[i].scene_name[0], p_scene_table_info->scene_table[i].scene_name_len);
                    }
                    j++;
                }
            }
        }


    }

    if (target_found == true)
    {
        gw_table_recover_start(SCENE_TABLE);
        gw_table_recover((uint32_t)p_scene_table_info);
        gw_table_recover_done();
        flush_cache();
    }
}

void scene_table_delete(uint16_t group_id, uint8_t scene_id, uint16_t nwk_addr, uint8_t end_point)
{
    scene_table_info_t *p_new_scene_table_info = (scene_table_info_t *)BACKUP_TABLES_ADDRESS;
    uint16_t i, j = 0, k;
    uint8_t used = 1, group_update = 0, target_found = false;

    flash_sector_erase(BACKUP_TABLES_ADDRESS);
    for (i = 0; i < SCENE_TABLE_NUM; i++)
    {
        if (p_scene_table_info->scene_table[i].used == true)
        {
            if ((p_scene_table_info->scene_table[i].group_id == group_id) &&
                    ((scene_id == ALL_SCENE) ||
                     (p_scene_table_info->scene_table[i].scene_id == scene_id)))
            {
                /* move the members of scene table which has different address & endpoint
                   with target address & endpoint into new flash page */
                group_update = 0;
                for (k = 0; k < SCENE_TABLE_MEMBER_NUM; k++)
                {
                    if (p_scene_table_info->scene_table[i].scene_member[k].used == 1)
                    {
                        if ((p_scene_table_info->scene_table[i].scene_member[k].end_point == end_point) &&
                                (p_scene_table_info->scene_table[i].scene_member[k].nwk_addr == nwk_addr))
                        {
                            //target found
                            target_found = true;
                        }
                        else
                        {
                            flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].used, &used, 1);
                            flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].end_point,
                                          &p_scene_table_info->scene_table[i].scene_member[k].end_point, 1);
                            flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].nwk_addr,
                                          (uint8_t *)&p_scene_table_info->scene_table[i].scene_member[k].nwk_addr, 2);
                            group_update = 1;
                        }
                    }
                }

                if (group_update == 1)
                {
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].used, &used, 1);
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].group_id, (uint8_t *)&p_scene_table_info->scene_table[i].group_id, 2);
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_id, (uint8_t *)&p_scene_table_info->scene_table[i].scene_id, 1);

                    if ((p_scene_table_info->scene_table[i].scene_name_len != 0) &&
                            (p_scene_table_info->scene_table[i].scene_name_len != 0xff))
                    {
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_name_len, &p_scene_table_info->scene_table[i].scene_name_len, 1);
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_name[0], &p_scene_table_info->scene_table[i].scene_name[0], p_scene_table_info->scene_table[i].scene_name_len);
                    }
                    j++;
                }
            }
            else
            {
                /* because group & scene id of this table is different with target group & scene id,
                   move all the members into new flash page*/
                flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].used, &used, 1);
                flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].group_id,
                              (uint8_t *)&p_scene_table_info->scene_table[i].group_id, 2);
                flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_id,
                              (uint8_t *)&p_scene_table_info->scene_table[i].scene_id, 1);

                if ((p_scene_table_info->scene_table[i].scene_name_len != 0) &&
                        (p_scene_table_info->scene_table[i].scene_name_len != 0xff))
                {
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_name_len, &p_scene_table_info->scene_table[i].scene_name_len, 1);
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_name[0], &p_scene_table_info->scene_table[i].scene_name[0], p_scene_table_info->scene_table[i].scene_name_len);
                }

                for (k = 0; k < SCENE_TABLE_MEMBER_NUM; k++)
                {
                    if (p_scene_table_info->scene_table[i].scene_member[k].used == 1)
                    {
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].used, &used, 1);
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].end_point,
                                      &p_scene_table_info->scene_table[i].scene_member[k].end_point, 1);
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].nwk_addr,
                                      (uint8_t *)&p_scene_table_info->scene_table[i].scene_member[k].nwk_addr, 2);
                    }
                }
                j++;
            }

        }
    }

    if (target_found == true)
    {
        gw_table_recover_start(SCENE_TABLE);
        gw_table_recover((uint32_t)p_scene_table_info);
        gw_table_recover_done();
        flush_cache();
    }
}

uint8_t scene_table_rename(uint16_t group_id, uint8_t scene_id, uint8_t len, uint8_t *p_name)
{
    scene_table_info_t *p_new_scene_table_info = (scene_table_info_t *)BACKUP_TABLES_ADDRESS;;
    uint16_t i, j = 0, k;
    uint8_t rename_status = RENAME_FAIL, used = 1, already_program = false;

    for (i = 0; i < SCENE_TABLE_NUM; i++)
    {
        if (p_scene_table_info->scene_table[i].used == true)
        {
            if ((p_scene_table_info->scene_table[i].group_id == group_id) &&
                    (p_scene_table_info->scene_table[i].scene_id == scene_id))
            {
                if (p_scene_table_info->scene_table[i].scene_name_len == FLASH_DEFAULT_VALUE)
                {
                    if (len < SCENE_NAME_LEN)
                    {
                        flash_program((uint32_t)&p_scene_table_info->scene_table[i].scene_name_len, &len, 1);
                        flash_program((uint32_t)&p_scene_table_info->scene_table[i].scene_name[0], p_name, len);
                        rename_status = RENAME_SUCCESS;
                    }
                    else
                    {
                        info_color(LOG_RED, "len of scene name invalid: %d\n", len);
                        rename_status = RENAME_INVALID_LEN;
                    }
                }
                else
                {
                    already_program = true;
                }
                break;
            }
        }
    }

    if (i == SCENE_TABLE_NUM)
    {
        rename_status = RENAME_NOT_FOUND;
    }
    else if (already_program == true)
    {
        /*rename the scene into new scene table*/
        flash_sector_erase(BACKUP_TABLES_ADDRESS);
        for (i = 0; i < SCENE_TABLE_NUM; i++)
        {
            if (p_scene_table_info->scene_table[i].used == true)
            {
                flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].used, &used, 1);
                flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].group_id,
                              (uint8_t *)&p_scene_table_info->scene_table[i].group_id, 2);
                flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_id,
                              (uint8_t *)&p_scene_table_info->scene_table[i].scene_id, 1);

                if ((group_id == p_scene_table_info->scene_table[i].group_id) &&
                        (scene_id == p_scene_table_info->scene_table[i].scene_id))
                {
                    /*target scene found, rename it*/
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_name_len, &len, 1);
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_name[0], &p_name[0], len);
                    rename_status = RENAME_SUCCESS;
                }
                else if ((p_scene_table_info->scene_table[i].scene_name_len != 0) &&
                         (p_scene_table_info->scene_table[i].scene_name_len != 0xff))
                {
                    /*copy the original name of scene into new scene table*/
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_name_len, &p_scene_table_info->scene_table[i].scene_name_len, 1);
                    flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_name[0], &p_scene_table_info->scene_table[i].scene_name[0], p_scene_table_info->scene_table[i].scene_name_len);
                }

                for (k = 0; k < SCENE_TABLE_MEMBER_NUM; k++)
                {
                    /*copy member of scene into new scene table*/
                    if (p_scene_table_info->scene_table[i].scene_member[k].used == 1)
                    {
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].used, &used, 1);
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].end_point,
                                      &p_scene_table_info->scene_table[i].scene_member[k].end_point, 1);
                        flash_program((uint32_t)&p_new_scene_table_info->scene_table[j].scene_member[k].nwk_addr,
                                      (uint8_t *)&p_scene_table_info->scene_table[i].scene_member[k].nwk_addr, 2);
                    }
                }
                j++;
            }
        }

        gw_table_recover_start(SCENE_TABLE);
        gw_table_recover((uint32_t)p_scene_table_info);
        gw_table_recover_done();

    }

    if (rename_status == RENAME_SUCCESS)
    {
        flush_cache();

    }

    return rename_status;
}

uint8_t scene_table_get(uint8_t *p_scene_data, uint8_t *p_scene_data_len, uint8_t start_idx)
{
    uint16_t i, j, data_idx = 0;

    for (i = start_idx; i < SCENE_TABLE_NUM; i++)
    {
        if (p_scene_table_info->scene_table[i].used == 0x1)
        {
            memcpy(&p_scene_data[data_idx], (uint8_t *)&p_scene_table_info->scene_table[i].group_id, sizeof(p_scene_table_info->scene_table[i].group_id));
            data_idx += sizeof(p_scene_table_info->scene_table[i].group_id);

            memcpy(&p_scene_data[data_idx], (uint8_t *)&p_scene_table_info->scene_table[i].scene_id, sizeof(p_scene_table_info->scene_table[i].scene_id));
            data_idx += sizeof(p_scene_table_info->scene_table[i].scene_id);

            if (p_scene_table_info->scene_table[i].scene_name_len != 0xFF &&
                    p_scene_table_info->scene_table[i].scene_name_len != 0)
            {
                memcpy(&p_scene_data[data_idx], (uint8_t *)&p_scene_table_info->scene_table[i].scene_name_len, sizeof(p_scene_table_info->scene_table[i].scene_name_len));
                data_idx += sizeof(p_scene_table_info->scene_table[i].scene_name_len);

                memcpy(&p_scene_data[data_idx], p_scene_table_info->scene_table[i].scene_name, p_scene_table_info->scene_table[i].scene_name_len);
                data_idx += p_scene_table_info->scene_table[i].scene_name_len;
            }
            else
            {
                p_scene_data[data_idx] = 0;
                data_idx += 1;
            }

            for (j = 0; j < GROUP_TABLE_MEMBER_NUM; j++)
            {
                if (p_scene_table_info->scene_table[i].scene_member[j].used == 0x01)
                {
                    memcpy(&p_scene_data[data_idx], (uint8_t *)&p_scene_table_info->scene_table[i].scene_member[j].nwk_addr, sizeof(p_scene_table_info->scene_table[i].scene_member[j].nwk_addr));
                    data_idx += sizeof(p_scene_table_info->scene_table[i].scene_member[j].nwk_addr);

                    memcpy(&p_scene_data[data_idx], (uint8_t *)&p_scene_table_info->scene_table[i].scene_member[j].end_point, sizeof(p_scene_table_info->scene_table[i].scene_member[j].end_point));
                    data_idx += sizeof(p_scene_table_info->scene_table[i].scene_member[j].end_point);
                }
            }

            break;
        }
    }
    *p_scene_data_len = data_idx;
    return i;
}

uint32_t scene_table_remove_all(void)
{
    uint8_t status;
    status = flash_sector_erase(SCENE_TABLE_FLASH_ADDRESS);

    status = (status == STATUS_SUCCESS) ? 0x00/*SUCCESS*/ :
             (status == STATUS_INVALID_PARAM) ? 0x85/*INVALID_FIELD*/ : 0x01;/*FAILURE*/;
    return status;

}

void bind_table_get_by_idx_cmd_forward(uint16_t start_idx)
{
    uint32_t command_id = 0;
    bind_table_get_by_idx_param_t bind_get;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_BIND_TABLE_BASE | GW_CMD_DB_BIND_TABLE_GET_BY_IDX;

    bind_get.start_idx = start_idx;
    gw_cmd_forward(command_id, sizeof(bind_table_get_by_idx_param_t), (uint8_t *)&bind_get);
}

void bind_member_add_cmd_forward(bind_info_param_t *p_bind_info, bind_address_param_t *p_bind_addr)
{
    uint32_t command_id = 0;
    bind_table_bind_unbind_param_t bind_member_add;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_BIND_TABLE_BASE | GW_CMD_DB_BIND_TABLE_CRATE;

    memcpy(&bind_member_add.bind_info, p_bind_info, sizeof(bind_info_param_t));

    if (bind_member_add.bind_info.dst_addr_mode == ADDR_MODE_GROUP)
    {
        bind_member_add.dst_addr.group_id = p_bind_addr->group_id;
        gw_cmd_forward(command_id, sizeof(bind_info_param_t) +2, (uint8_t *)&bind_member_add);
    }
    else if (bind_member_add.bind_info.dst_addr_mode == ADDR_MODE_EXTENDED_ADDR)
    {
        memcpy(bind_member_add.dst_addr.unicast.ieee_addr, p_bind_addr->unicast.ieee_addr, 8);/*dst ieee_addr + dst endpoint*/
        bind_member_add.dst_addr.unicast.end_point = p_bind_addr->unicast.end_point;
        gw_cmd_forward(command_id, sizeof(bind_table_bind_unbind_param_t), (uint8_t *)&bind_member_add);
    }
    else
    {
        info_color(LOG_RED, "dst addr mode %d error\n", bind_member_add.bind_info.dst_addr_mode);
    }
}

void bind_member_remove_cmd_forward(bind_info_param_t *p_bind_info, bind_address_param_t *p_bind_addr)
{
    uint32_t command_id = 0;
    bind_table_bind_unbind_param_t bind_member_remove;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_BIND_TABLE_BASE | GW_CMD_DB_BIND_TABLE_REMOVE;

    memcpy(&bind_member_remove.bind_info, p_bind_info, sizeof(bind_info_param_t));

    if (bind_member_remove.bind_info.dst_addr_mode == ADDR_MODE_GROUP)
    {
        bind_member_remove.dst_addr.group_id = p_bind_addr->group_id;
        gw_cmd_forward(command_id, sizeof(bind_info_param_t) + 2, (uint8_t *)&bind_member_remove);
    }
    else if (bind_member_remove.bind_info.dst_addr_mode == ADDR_MODE_EXTENDED_ADDR)
    {
        memcpy(bind_member_remove.dst_addr.unicast.ieee_addr, p_bind_addr->unicast.ieee_addr, 8);/*dst ieee_addr + dst endpoint*/
        bind_member_remove.dst_addr.unicast.end_point = p_bind_addr->unicast.end_point;
        gw_cmd_forward(command_id, sizeof(bind_info_param_t) + 9, (uint8_t *)&bind_member_remove);
    }
    else
    {
        info_color(LOG_RED, "dst addr mode %d error\n", bind_member_remove.bind_info.dst_addr_mode);
    }
}


uint8_t bind_table_create(bind_table_bind_unbind_param_t *p_bind)
{
    uint8_t create_success = false;
    uint16_t i;

    for (i = 0; i < BIND_TABLE_NUM ; i++)
    {
        if ((p_bind_table_info->bind_table[i].used == true) &&
                (memcmp(&p_bind_table_info->bind_table[i].bind_info, &p_bind->bind_info, sizeof(bind_info_param_t)) == 0))
        {
            if (p_bind->bind_info.dst_addr_mode == ADDR_MODE_GROUP)
            {
                if (p_bind_table_info->bind_table[i].dst_addr.group_id == p_bind->dst_addr.group_id)
                {
                    create_success = true;
                    break;
                }
            }
            else if (p_bind->bind_info.dst_addr_mode == ADDR_MODE_EXTENDED_ADDR)
            {
                if ((memcmp(p_bind_table_info->bind_table[i].dst_addr.unicast.ieee_addr, p_bind->dst_addr.unicast.ieee_addr, 8) == 0) &&
                        (p_bind_table_info->bind_table[i].dst_addr.unicast.end_point == p_bind->dst_addr.unicast.end_point))
                {
                    create_success = true;
                    break;
                }
            }
            else
            {
                info_color(LOG_RED, "bind_table_create invalid addr_mode: %d\n", p_bind->bind_info.dst_addr_mode);
                break;
            }
        }
    }

    if (create_success == false)
    {
        for (i = 0; i < BIND_TABLE_NUM ; i++)
        {
            if (p_bind_table_info->bind_table[i].used != true)
            {
                uint8_t used = true;
                flash_program((uint32_t)&p_bind_table_info->bind_table[i].used, &used, 1);
                flash_program((uint32_t)&p_bind_table_info->bind_table[i].bind_info, (uint8_t *)&p_bind->bind_info, sizeof(bind_info_param_t));

                if (p_bind->bind_info.dst_addr_mode == ADDR_MODE_GROUP)
                {
                    flash_program((uint32_t)&p_bind_table_info->bind_table[i].dst_addr.group_id, (uint8_t *)&p_bind->dst_addr.group_id, 2);
                    create_success = true;
                }
                else if (p_bind->bind_info.dst_addr_mode == ADDR_MODE_EXTENDED_ADDR)
                {
                    flash_program((uint32_t)&p_bind_table_info->bind_table[i].dst_addr.unicast.ieee_addr[0], (uint8_t *)&p_bind->dst_addr.unicast.ieee_addr[0], 8);
                    flash_program((uint32_t)&p_bind_table_info->bind_table[i].dst_addr.unicast.end_point, (uint8_t *)&p_bind->dst_addr.unicast.end_point, 1);
                    create_success = true;
                }
                else
                {
                    info_color(LOG_RED, "bind_table_create invalid addr_mode: %d\n", p_bind->bind_info.dst_addr_mode);
                }
                break;
            }
        }
    }
    if (create_success == true)
    {
        flush_cache();
    }
    return create_success;
}

void bind_table_copy_to_backup_page(void)
{
    bind_table_info_t *p_new_bind_table_info = (bind_table_info_t *)(BACKUP_TABLES_ADDRESS + BIND_TABLE_OFFSET);
    uint16_t i, j = 0;

    for (i = 0; i < BIND_TABLE_NUM; i++)
    {
        if (p_bind_table_info->bind_table[i].used == true)
        {
            uint8_t used = true;
            flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].used, &used, 1);
            flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].bind_info, (uint8_t *)&p_bind_table_info->bind_table[i].bind_info, sizeof(bind_info_param_t));

            if (p_bind_table_info->bind_table[i].bind_info.dst_addr_mode == ADDR_MODE_GROUP)
            {
                flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].dst_addr.group_id, (uint8_t *)&p_bind_table_info->bind_table[j].dst_addr.group_id, 2);
            }
            else if (p_bind_table_info->bind_table[i].bind_info.dst_addr_mode == ADDR_MODE_EXTENDED_ADDR)
            {
                flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].dst_addr.unicast.ieee_addr[0], p_bind_table_info->bind_table[j].dst_addr.unicast.ieee_addr, 8);
                flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].dst_addr.unicast.end_point, (uint8_t *)&p_bind_table_info->bind_table[j].dst_addr.unicast.end_point, 1);
            }
            else
            {
                info_color(LOG_RED, "bind_table_copy invalid addr_mode: %d\n", p_bind_table_info->bind_table[i].bind_info.dst_addr_mode);
            }
            j++;
        }
    }

}

void bind_table_delete_by_ieee_addr(uint8_t *p_src_addr)
{
    bind_table_info_t *p_new_bind_table_info = (bind_table_info_t *)(BACKUP_TABLES_ADDRESS + BIND_TABLE_OFFSET);
    uint16_t i, j = 0, target_found = false;

    flash_sector_erase(BACKUP_TABLES_ADDRESS);

    for (i = 0; i < BIND_TABLE_NUM; i++)
    {
        if (p_bind_table_info->bind_table[i].used == true)
        {
            if (memcmp(p_bind_table_info->bind_table[i].bind_info.src_ieee_addr, p_src_addr, 8) == 0)
            {
                //delete target found
                target_found = true;
            }
            else
            {
                uint8_t used = true;
                flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].used, &used, 1);
                flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].bind_info, (uint8_t *)&p_bind_table_info->bind_table[i].bind_info, sizeof(bind_info_param_t));

                if (p_bind_table_info->bind_table[i].bind_info.dst_addr_mode == ADDR_MODE_GROUP)
                {
                    flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].dst_addr.group_id, (uint8_t *)&p_bind_table_info->bind_table[i].dst_addr.group_id, 2);
                }
                else if (p_bind_table_info->bind_table[i].bind_info.dst_addr_mode == ADDR_MODE_EXTENDED_ADDR)
                {
                    flash_program((uint32_t)p_new_bind_table_info->bind_table[j].dst_addr.unicast.ieee_addr, p_bind_table_info->bind_table[i].dst_addr.unicast.ieee_addr, 8);
                    flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].dst_addr.unicast.end_point, (uint8_t *)&p_bind_table_info->bind_table[i].dst_addr.unicast.end_point, 1);
                }
                else
                {
                    info_color(LOG_RED, "bind_table_delete invalid addr_mode: %d\n", p_bind_table_info->bind_table[i].bind_info.dst_addr_mode);
                }
                j++;
            }
        }
    }

    if (target_found == true)
    {
        device_table_copy_to_backup_page();
        gw_table_recover_start(DEVICE_BIND_TABLE);
        gw_table_recover((uint32_t)p_device_table_info);
        gw_table_recover_done();
        flush_cache();
    }
}


void bind_table_delete(bind_table_bind_unbind_param_t *p_bind)
{
    bind_table_info_t *p_new_bind_table_info = (bind_table_info_t *)(BACKUP_TABLES_ADDRESS + BIND_TABLE_OFFSET);
    uint16_t i, j = 0, target_found = false;

    flash_sector_erase(BACKUP_TABLES_ADDRESS);

    for (i = 0; i < BIND_TABLE_NUM; i++)
    {
        if (p_bind_table_info->bind_table[i].used == true)
        {
            if ((memcmp(&p_bind_table_info->bind_table[i].bind_info, &p_bind->bind_info, sizeof(bind_info_param_t)) == 0) &&
                    (((p_bind_table_info->bind_table[i].bind_info.dst_addr_mode == ADDR_MODE_GROUP) &&
                      (memcmp((uint8_t *)&p_bind_table_info->bind_table[i].dst_addr.group_id, (uint8_t *)&p_bind->dst_addr.group_id, 2) == 0)) ||
                     ((p_bind_table_info->bind_table[i].bind_info.dst_addr_mode == ADDR_MODE_EXTENDED_ADDR) &&
                      (memcmp((uint8_t *)&p_bind_table_info->bind_table[i].dst_addr.unicast.ieee_addr[0], p_bind->dst_addr.unicast.ieee_addr, 8) == 0) &&
                      (p_bind_table_info->bind_table[i].dst_addr.unicast.end_point == p_bind->dst_addr.unicast.end_point))))
            {
                //delete target found
                target_found = true;
            }
            else
            {
                uint8_t used = true;
                flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].used, &used, 1);
                flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].bind_info, (uint8_t *)&p_bind_table_info->bind_table[i].bind_info, sizeof(bind_info_param_t));

                if (p_bind_table_info->bind_table[i].bind_info.dst_addr_mode == ADDR_MODE_GROUP)
                {
                    flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].dst_addr.group_id, (uint8_t *)&p_bind_table_info->bind_table[i].dst_addr.group_id, 2);
                }
                else if (p_bind_table_info->bind_table[i].bind_info.dst_addr_mode == ADDR_MODE_EXTENDED_ADDR)
                {
                    flash_program((uint32_t)p_new_bind_table_info->bind_table[j].dst_addr.unicast.ieee_addr, (uint8_t *)&p_bind_table_info->bind_table[i].dst_addr.unicast.ieee_addr, 8);
                    flash_program((uint32_t)&p_new_bind_table_info->bind_table[j].dst_addr.unicast.end_point, (uint8_t *)&p_bind_table_info->bind_table[i].dst_addr.unicast.end_point, 1);
                }
                else
                {
                    info_color(LOG_RED, "bind_table_delete invalid addr_mode: %d\n", p_bind_table_info->bind_table[i].bind_info.dst_addr_mode);
                }
                j++;
            }
        }
    }

    if (target_found == true)
    {
        device_table_copy_to_backup_page();
        gw_table_recover_start(DEVICE_BIND_TABLE);
        gw_table_recover((uint32_t)p_device_table_info);
        gw_table_recover_done();
        flush_cache();
    }
}

uint8_t bind_table_get(uint8_t *p_bind_data, uint8_t start_idx)
{
    uint16_t i, data_idx = 0;

    for (i = start_idx; i < BIND_TABLE_NUM; i++)
    {
        if (p_bind_table_info->bind_table[i].used == true)
        {
            memcpy(&p_bind_data[data_idx], &p_bind_table_info->bind_table[i].bind_info, sizeof(bind_info_param_t));
            data_idx += sizeof(bind_info_param_t);

            if (p_bind_table_info->bind_table[i].bind_info.dst_addr_mode == ADDR_MODE_GROUP)
            {
                memcpy(&p_bind_data[data_idx], (uint8_t *)&p_bind_table_info->bind_table[i].dst_addr.group_id, sizeof(p_bind_table_info->bind_table[i].dst_addr.group_id));
                data_idx += sizeof(p_bind_table_info->bind_table[i].dst_addr.group_id);
            }
            else if (p_bind_table_info->bind_table[i].bind_info.dst_addr_mode == ADDR_MODE_EXTENDED_ADDR)
            {
                memcpy(&p_bind_data[data_idx], (uint8_t *)&p_bind_table_info->bind_table[i].dst_addr.unicast.ieee_addr, 8);
                data_idx += 8;
                memcpy(&p_bind_data[data_idx], (uint8_t *)&p_bind_table_info->bind_table[i].dst_addr.unicast.end_point, 1);
                data_idx += 1;
            }
            else
            {
                info_color(LOG_RED, "invalid addr_mode: %d\n", p_bind_table_info->bind_table[i].bind_info.dst_addr_mode);
            }
            break;
        }
    }

    return i;
}

uint32_t bind_table_remove_all(void)
{
    uint8_t status;

    flash_sector_erase(BACKUP_TABLES_ADDRESS);
    device_table_copy_to_backup_page();
    status = flash_sector_erase(DEVICE_TABLE_FLASH_ADDRESS);

    gw_table_recover_start(DEVICE_BIND_TABLE);
    gw_table_recover((uint32_t)p_device_table_info);
    gw_table_recover_done();

    status = (status == STATUS_SUCCESS) ? 0x00/*SUCCESS*/ :
             (status == STATUS_INVALID_PARAM) ? 0x85/*INVALID_FIELD*/ : 0x01;/*FAILURE*/;
    return status;

}

void zc_info_internal_get_cmd_forward(void)
{
    uint32_t command_id = 0;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_ZC_INFO_BASE | GW_CMD_DB_ZC_INFO_INTERNAL_UPDATE;

    gw_cmd_forward(command_id, 0, NULL);
}

void zc_info_create_cmd_forward(uint8_t channel, uint16_t pan_id, uint8_t sys_reboot)
{
    uint32_t command_id = 0;
    zc_info_create_param_t zc_info_create;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_ZC_INFO_BASE | GW_CMD_DB_ZC_INFO_CREATE;
    zc_info_create.pan_id = pan_id;
    zc_info_create.channel = channel;
    zc_info_create.sys_reboot = sys_reboot;
    gw_cmd_forward(command_id, sizeof(zc_info_create_param_t), (uint8_t *)&zc_info_create);
}

void zc_info_remove_cmd_forward(void)
{
    uint32_t command_id = 0;

    command_id |= GW_CMD_DB_BASE | GW_CMD_DB_ZC_INFO_BASE | GW_CMD_DB_ZC_INFO_REMOVE;
    gw_cmd_forward(command_id, 0, NULL);
}

void gw_table_initial(void)
{
    uint8_t recover_table, recover_done;

    recover_table = flash_read_byte(BACKUP_RECOVER_TABLE_ADDRESS);
    recover_done = flash_read_byte(BACKUP_RECOVER_DONE_ADDRESS);

    if (recover_done != RECOVER_DONE_SEQ)
    {
        if (recover_table == DEVICE_BIND_TABLE)
        {
            gw_table_recover((uint32_t)p_device_table_info);
        }
        else if (recover_table == GROUP_TABLE)
        {
            gw_table_recover((uint32_t)p_group_table_info);
        }
        else if (recover_table == SCENE_TABLE)
        {
            gw_table_recover((uint32_t)p_scene_table_info);
        }
    }
    flash_sector_erase(BACKUP_TABLES_ADDRESS);

    memset(bind_mapping, 0, sizeof(bind_unbind_req_mapping_t));
    memset(unbind_mapping, 0, sizeof(bind_unbind_req_mapping_t));
    zc_info_internal_get_cmd_forward();
}

