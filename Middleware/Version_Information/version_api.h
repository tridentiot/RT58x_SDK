
typedef struct __version_entry_s
{
    char  *version_tag;
    char  *version_info;
    char  *build_date;
} version_entry_t;



void ble_lib_ver_get(volatile const version_entry_t *pt_ver_info);
void ble_mesh_lib_ver_get(volatile const version_entry_t *pt_ver_info);
void zigbee_lib_ver_get(volatile const version_entry_t *pt_ver_info);
void thread_lib_ver_get(volatile const version_entry_t *pt_ver_info);
void matter_lib_ver_get(volatile const version_entry_t *pt_ver_info);
void bootloader_ver_get(volatile const version_entry_t *pt_ver_info);

