#ifndef _SPRD_VDSP_SERVER_INTERFACE__H
#define _SPRD_VDSP_SERVER_INTERFACE__H
#include "vdsp_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

enum sprd_vdsp_result sprd_vdsp_open_device_serv(enum sprd_vdsp_worktype type , struct vdsp_handle *handle);
enum sprd_vdsp_result sprd_vdsp_close_device_serv(void *handle);
enum sprd_vdsp_result sprd_vdsp_send_cmd_serv(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out ,
                                                struct sprd_vdsp_client_inout *buffer , uint32_t bufnum , uint32_t priority);
enum sprd_vdsp_result sprd_vdsp_loadlibrary_serv(void *handle , const char *libname , struct sprd_vdsp_client_inout *buffer);
enum sprd_vdsp_result sprd_vdsp_unloadlibrary_serv(void *handle , const char *libname);
enum sprd_vdsp_result sprd_vdsp_power_hint_serv(void *handle , enum sprd_vdsp_power_level level , enum sprd_vdsp_powerhint_acquire_release acquire_release);


#ifdef __cplusplus
}
#endif

#endif
