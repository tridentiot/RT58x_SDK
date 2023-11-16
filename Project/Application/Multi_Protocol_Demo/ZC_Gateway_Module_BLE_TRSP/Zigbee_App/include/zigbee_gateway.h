// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
#ifndef __ZIGBEE_GATEWAY_DB_H__
#define __ZIGBEE_GATEWAY_DB_H__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                Public ENUM
//=============================================================================
#define GW_CMD_DB_DEVICE_TABLE_BASE           (0x100)
#define GW_CMD_APP_CMD_RSP_GEN(id)            (id | 0x8000)

typedef enum
{
    GW_CMD_DEVICE_ANN_IND = 0x0013,

} zb_gw_cmd_id_table;

typedef enum
{
    RENAME_SUCCESS = 0,
    RENAME_INVALID_LEN,
    RENAME_NOT_FOUND,
    RENAME_FAIL,
} rename_status;


typedef enum
{
    GW_CMD_DB_DEVICE_TABLE_GET = 0,
    GW_CMD_DB_DEVICE_RENAME,
    GW_CMD_DB_END_POINT_RENAME,
    GW_CMD_DB_DEVICE_TABLE_REMOVE_ALL,
    GW_CMD_DB_DEVICE_TABLE_REMOVE_SPECIFIC_DEVICE,
    GW_CMD_DB_DEVICE_TABLE_CREATE,
    GW_CMD_DB_DEVICE_TABLE_UPDATE,
    GW_CMD_DB_DEVICE_TABLE_GET_BY_IDX,
    GW_CMD_DB_DEVICE_TABLE_REMOVE,

} e_device_table;

#define GW_CMD_DB_GROUP_TABLE_BASE            (0x200)
typedef enum
{
    GW_CMD_DB_GROUP_TABLE_GET = 0,
    GW_CMD_DB_GROUP_RENAME,
    GW_CMD_DB_GROUP_TABLE_REMOVE_ALL,
    GW_CMD_DB_GROUP_TABLE_CREATE,
    GW_CMD_DB_GROUP_TABLE_MEMBER_ADD,
    GW_CMD_DB_GROUP_TABLE_MEMBER_REMOVE,
    GW_CMD_DB_GROUP_TABLE_DEVICE_ALL_GROUPS_REMOVE,
    GW_CMD_DB_GROUP_TABLE_GET_BY_IDX,
} e_group_table;

#define GW_CMD_DB_SCENE_TABLE_BASE            (0x300)
typedef enum
{
    GW_CMD_DB_SCENE_TABLE_GET = 0,
    GW_CMD_DB_SCENE_RENAME,
    GW_CMD_DB_SCENE_TABLE_REMOVE_ALL,
    GW_CMD_DB_SCENE_TABLE_CREATE,
    GW_CMD_DB_SCENE_TABLE_MEMBER_ADD,
    GW_CMD_DB_SCENE_TABLE_MEMBER_REMOVE,
    GW_CMD_DB_SCENE_TABLE_GET_BY_IDX,
} e_scene_table;

#define GW_CMD_DB_BIND_TABLE_BASE             (0x400)
typedef enum
{
    GW_CMD_DB_BIND_TABLE_GET = 0,
    GW_CMD_DB_BIND_TABLE_REMOVE_ALL,
    GW_CMD_DB_BIND_TABLE_CRATE,
    GW_CMD_DB_BIND_TABLE_REMOVE,
    GW_CMD_DB_BIND_TABLE_GET_BY_IDX,
} e_bind_table;

#define GW_CMD_DB_ZC_INFO_BASE                (0xA00)
typedef enum
{
    GW_CMD_DB_ZC_INFO_GET = 0,
    GW_CMD_DB_ZC_INFO_CHANGE_NOTIFY,
    GW_CMD_DB_ZC_INFO_INTERNAL_UPDATE,
    GW_CMD_DB_ZC_INFO_CREATE,
    GW_CMD_DB_ZC_INFO_REMOVE,
} e_zc_info;

//=============================================================================
//                Public Definitions of const value
//=============================================================================

#define GW_CMD_DB_CMD_OFFSET                  (0x100)
#define GW_CMD_DB_BASE                        (0x10000000)

#define GW_CMD_HEADER_LEN                     (sizeof(gateway_cmd))

#define DEVICE_TABLE_NUM                      (51) /*50 device + 1 zc*/
#define SCENE_TABLE_NUM                       (16)
#define SCENE_TABLE_MEMBER_NUM                (32)
#define GROUP_TABLE_NUM                       (16)
#define GROUP_TABLE_MEMBER_NUM                (32)
#define BIND_TABLE_NUM                        (16)


#define ALL_GROUP                             (0xFFFF)
#define ALL_SCENE                             (0xFF)

#define ADDR_MODE_GROUP                       (0x1)
#define ADDR_MODE_EXTENDED_ADDR               (0x3)

#define END_POINT_NAME_LEN                  (20)
#define DEVICE_NAME_LEN                     (20)
#define GROUP_NAME_LEN                      (20)
#define SCENE_NAME_LEN                      (20)

//=============================================================================
//                Public Struct
//=============================================================================

typedef struct __attribute__((packed))
{
    uint8_t header[4];
    uint8_t len;
}
gateway_cmd_hdr;

typedef struct __attribute__((packed))
{
    uint32_t command_id;
    uint16_t address;
    uint8_t address_mode;
    uint8_t parameter[];
}
gateway_cmd_pd;

typedef struct __attribute__((packed))
{
    gateway_cmd_hdr hdr;
    gateway_cmd_pd pd;
}
gateway_cmd;
typedef struct __attribute__((packed))
{
    uint8_t cs;
}
gateway_cmd_end;

typedef struct
{
    uint8_t             ieee_addr[8];     /* */
    uint16_t            nwk_addr;         /* */
} device_table_create_param_t;

typedef struct
{
    uint16_t            nwk_addr;         /* */
    uint16_t            device_id;        /* */
    uint8_t             end_point;        /* */
} device_table_update_param_t;

typedef struct
{
    uint8_t             ieee_addr[8];     /* */
} device_table_delete_param_t;

typedef struct
{
    uint16_t            nwk_addr;         /* */
    uint8_t             end_point;        /* */
    uint8_t             len;        /* */
    uint8_t             name[20];        /* */
} device_table_rename_ep_param_t;

typedef struct
{
    uint16_t            nwk_addr;         /* */
    uint8_t             len;        /* */
    uint8_t             name[20];        /* */
} device_table_rename_device_param_t;

typedef struct
{
    uint16_t            start_idx;         /* */
} device_table_get_by_idx_param_t;

typedef struct __attribute__((packed))
{
    uint8_t             ieee_addr[8];     /* */
    uint16_t            nwk_addr;         /* */
    uint16_t            device_id;        /* */
    uint8_t             end_point;        /* */
}
device_info_param_t;

typedef struct __attribute__((packed))
{
    uint8_t             name_len;  /* */
    uint8_t             name[DEVICE_NAME_LEN];
}
device_name_param_t;

typedef struct __attribute__((packed))
{
    uint8_t             name_len;  /* */
    uint8_t             name[END_POINT_NAME_LEN];
}
end_point_name_param_t;

typedef struct
{
    uint8_t                 status;         /* */
    device_info_param_t     device_info;
    device_name_param_t     device_name;
    end_point_name_param_t  ep_name;
} device_table_get_rsp_t;

typedef struct
{
    uint16_t            group_id;         /* */
} group_table_create_param_t;


typedef struct
{
    uint16_t            group_id;        /* */
    uint16_t            nwk_addr;         /* */
    uint8_t             end_point;        /* */
} group_member_add_remove_param_t;

typedef struct
{
    uint16_t            nwk_addr;         /* */
    uint8_t             end_point;        /* */
} group_all_groups_remove_param_t;

typedef struct
{
    uint16_t            group_id;         /* */
    uint8_t             len;        /* */
    uint8_t             name[20];        /* */
} group_table_rename_param_t;

typedef struct
{
    uint16_t            start_idx;         /* */
} group_table_get_by_idx_param_t;

typedef struct
{
    uint16_t            group_id;         /* */
    uint8_t             secne_id;         /* */
} scene_table_create_param_t;

typedef struct
{
    uint16_t            group_id;        /* */
    uint8_t             scene_id;        /* */
    uint16_t            nwk_addr;         /* */
    uint8_t             end_point;        /* */
} scene_member_add_remove_param_t;

typedef struct
{
    uint16_t            group_id;         /* */
    uint16_t            nwk_addr;         /* */
    uint8_t             end_point;        /* */
} scene_all_scenes_remove_param_t;

typedef struct
{
    uint16_t            group_id;         /* */
    uint8_t             scene_id;        /* */
    uint8_t             len;        /* */
    uint8_t             name[20];        /* */
} scene_table_rename_param_t;

typedef struct
{
    uint16_t            start_idx;         /* */
} scene_table_get_by_idx_param_t;

typedef struct
{
    uint16_t            start_idx;         /* */
} bind_table_get_by_idx_param_t;

typedef struct __attribute__((packed))
{
    uint8_t             src_ieee_addr[8];  /* */
    uint8_t             src_ep;           /* */
    uint16_t            cluster_id;       /* */
    uint8_t             dst_addr_mode;     /* */
}
bind_info_param_t;

typedef struct __attribute__((packed))
{
    uint8_t             ieee_addr[8];         /* */
    uint8_t             end_point;         /* */
}
bind_unicast_t;

typedef union __attribute__((packed))
{
    uint16_t            group_id;       /* */
    bind_unicast_t      unicast;
}
bind_address_param_t;

typedef struct __attribute__((packed))
{
    bind_info_param_t bind_info;
    bind_address_param_t dst_addr;
}
bind_table_bind_unbind_param_t;

typedef struct
{
    uint16_t            pan_id;     /* */
    uint8_t             channel;         /* */
    uint8_t             sys_reboot;         /* */
} zc_info_create_param_t;

//=============================================================================
//                Public Function Declaration
//=============================================================================
extern uint16_t g_pan_id;
extern uint8_t g_channel;

uint8_t group_id_valid(uint16_t group_id);
void gw_default_rsp_check(uint16_t cluster_id, uint8_t cmd, uint16_t src_addr, uint8_t src_ep, uint8_t status);
void gw_cmd_check(uint32_t command_id, uint8_t *p_command_payload);
void gw_event_check(uint32_t command_id, uint16_t src_addr, uint8_t command_len, uint8_t *p_command_payload);

void device_table_create_cmd_forward(uint8_t *p_ieee_address, uint16_t nwk_address);
void device_table_update_cmd_forward(uint16_t address, uint8_t end_point, uint16_t device_id);
void device_table_delete_cmd_forward(uint8_t *p_ieee_addr);
void device_table_get_by_idx_cmd_forward(uint16_t start_idx);

uint8_t device_table_create(uint8_t *p_ieee_addr, uint16_t nwk_addr);
uint8_t device_table_update(uint16_t nwk_addr, uint16_t device_id, uint8_t end_point);
uint8_t device_table_delete(uint8_t addr_len, uint8_t *p_addr, uint8_t end_point);
uint8_t device_table_get(device_table_get_rsp_t *p_device_data, uint8_t start_idx);
uint8_t device_table_device_rename(uint16_t nwk_addr, uint8_t len, uint8_t *p_name);
uint8_t device_table_end_point_rename(uint16_t nwk_addr, uint8_t end_point, uint8_t len, uint8_t *p_name);
void device_table_initial(void);
void device_table_remove_all(uint8_t bind_remove, uint8_t zc_info_remove);

void group_member_add_cmd_forward(uint16_t address, uint8_t end_point, uint16_t group_id);
void group_member_remove_cmd_forward(uint16_t address, uint8_t end_point, uint16_t group_id);
void group_all_groups_remove_cmd_forward(uint16_t address, uint8_t end_point);
void group_table_get_by_idx_cmd_forward(uint16_t start_idx);
void group_table_delete_by_nwk_addr(uint16_t nwk_addr);

uint8_t group_table_create(uint16_t group_id);
uint8_t group_table_update(uint16_t group_id, uint16_t nwk_addr, uint8_t end_point);
void group_table_delete(uint16_t group_id, uint16_t nwk_addr, uint8_t end_point);
uint8_t group_table_rename(uint16_t group_id, uint8_t len, uint8_t *p_name);
uint8_t group_table_get(uint8_t *p_group_data, uint8_t *p_device_len, uint8_t start_idx);
uint32_t group_table_remove_all(void);

void scene_member_add_cmd_forward(uint16_t address, uint8_t end_point, uint16_t group_id, uint8_t scene_id);
void scene_member_remove_cmd_forward(uint16_t address, uint8_t end_point, uint16_t group_id, uint8_t scene_id);
void scene_table_get_by_idx_cmd_forward(uint16_t start_idx);

uint8_t scene_table_create(uint16_t group_id, uint8_t scene_id);
uint8_t scene_table_update(uint16_t group_id, uint8_t scene_id, uint16_t nwk_addr, uint8_t end_point);
void scene_table_delete(uint16_t group_id, uint8_t scene_id, uint16_t nwk_addr, uint8_t end_point);
uint8_t scene_table_rename(uint16_t group_id, uint8_t scene_id, uint8_t len, uint8_t *p_name);
uint8_t scene_table_get(uint8_t *p_scene_data, uint8_t *p_scene_data_len, uint8_t start_idx);
uint32_t scene_table_remove_all(void);
void scene_table_delete_by_nwk_addr(uint16_t nwk_addr);

void bind_table_get_by_idx_cmd_forward(uint16_t start_idx);
void bind_member_add_cmd_forward(bind_info_param_t *p_bind_info, bind_address_param_t *p_bind_addr);
void bind_member_remove_cmd_forward(bind_info_param_t *p_bind_info, bind_address_param_t *p_bind_addr);

uint8_t bind_table_create(bind_table_bind_unbind_param_t *p_bind);
void bind_table_delete(bind_table_bind_unbind_param_t *p_bind);
uint8_t bind_table_get(uint8_t *p_bind_data, uint8_t start_idx);
uint32_t bind_table_remove_all(void);
void bind_table_delete_by_ieee_addr(uint8_t *p_src_addr);

void zc_info_create_cmd_forward(uint8_t channel, uint16_t pan_id, uint8_t sys_reboot);
void zc_info_remove_cmd_forward(void);

void gw_table_initial(void);


#ifdef __cplusplus
};
#endif
#endif /* __ZIGBEE_GATEWAY_DB_H__ */
