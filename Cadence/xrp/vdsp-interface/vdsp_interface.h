#ifndef _SPRD_VDSP_INTERFACE__H
#define _SPRD_VDSP_INTERFACE__H
#include <sys/types.h>


enum sprd_vdsp_getinfo_cmd
{
	SPRD_VDSP_GET_VERSION,
	SPRD_VDSP_GET_MAX,
};
enum sprd_vdsp_ctrl_cmd
{
	SPRD_VDSP_CTRL_CMD_MAX,
};
enum sprd_vdsp_status
{
        SPRD_XRP_STATUS_SUCCESS = 0,
        SPRD_XRP_STATUS_FAILURE,
        SPRD_XRP_STATUS_PENDING,
};
enum sprd_vdsp_result
{
	SPRD_VDSP_RESULT_SUCCESS = 0,
	SPRD_VDSP_RESULT_FAIL,
	SPRD_VDSP_RESULT_OLD_GENERATION,
	SPRD_VDSP_RESULT_NOT_SUPPORT,
	SPRD_VDSP_RESULT_MAX,
};
enum sprd_vdsp_power_level
{
	SPRD_VDSP_POWERHINT_RESTORE_DVFS = -1,
	SPRD_VDSP_POWERHINT_LEVEL_0,
	SPRD_VDSP_POWERHINT_LEVEL_1,
	SPRD_VDSP_POWERHINT_LEVEL_2,
	SPRD_VDSP_POWERHINT_LEVEL_3,
	SPRD_VDSP_POWERHINT_LEVEL_4,
	SPRD_VDSP_POWERHINT_LEVEL_5,
	SPRD_VDSP_POWERHINT_LEVEL_MAX,
};
enum sprd_vdsp_bufflag
{
	SPRD_VDSP_XRP_READ,
	SPRD_VDSP_XRP_WRITE,
	SPRD_VDSP_XRP_READ_WRITE,
	SPRD_VDSP_XRP_ACCESS_MAX,
};
struct sprd_vdsp_client_inout
{
	int32_t fd;
	void *viraddr;
	uint32_t phy_addr;
	uint32_t size;
	enum sprd_vdsp_bufflag flag;
};


enum sprd_vdsp_worktype
{
	SPRD_VDSP_WORK_NORMAL,
	SPRD_VDSP_WORK_FACEID,
	SPRD_VDSP_WORK_MAXTYPE,
};
enum sprd_xrp_queue_priority
{
        SPRD_XRP_PRIORITY_0 = 0,
        SPRD_XRP_PRIORITY_1,
        SPRD_XRP_PRIORITY_2,
        SPRD_XRP_PRIORITY_MAX
};
enum sprd_vdsp_powerhint_acquire_release
{
	SPRD_VDSP_POWERHINT_ACQUIRE = 0,
	SPRD_VDSP_POWERHINT_RELEASE,
	SPRD_VDSP_POWERHINT_MAX,
};
enum sprd_vdsp_interface_type
{
	SPRD_VDSP_INTERFACE_DIRECTLY,
	SPRD_VDSP_INTERFACE_BYSERVER,
	SPRD_VDSP_INTERFACe_MAX,
};

struct vdsp_handle
{
        int32_t fd;
        uint32_t generation;
};
struct vdsp_open_param
{
	int32_t idx;
	enum sprd_vdsp_worktype work_type;
	enum sprd_vdsp_interface_type int_type;
};
typedef struct
{
	uint32_t width, height;
	uint32_t phyaddr;		/*image phyaddr*/
	uint32_t workstage;		/*enroll:0,auth:1*/
	uint32_t framecount;
	uint32_t liveness;		/*0:off 1:faceid_single 2:faceid_3D 3:pay_3D*/
	int32_t  help_info[259];		/*AE BV*/
	uint32_t l_ir_phyaddr;	/*Left IR phyaddr*/
	uint32_t r_ir_phyaddr;	/*Right IR phyaddr*/
	uint32_t bgr_phyaddr;	/*bgr phyaddr*/
	uint32_t otp_phyaddr;	/*otp phyaddr*/
}FACEID_IN;
#ifdef __cplusplus
extern "C" {
#endif
/************************************************************
name: sprd_cavdsp_get_info
description: get vdsp library version
param:
cmd ---- now only support GETVERSION
output ---- output info
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
***********************************************************/
enum sprd_vdsp_result sprd_cavdsp_get_info(enum sprd_vdsp_getinfo_cmd cmd , void* output);

/************************************************************
name: sprd_cavdsp_open_device
description: open virtural vdsp device
param:
type ---- SPRD_VDSP_WORK_NORMAL for arithmetic firmwwareand SPRD_VDSP_WORK_FACEID for faceid firmware
handle -------- handle.fd is fd; generation is xrp server generation
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
***********************************************************/
enum sprd_vdsp_result sprd_cavdsp_open_device(enum sprd_vdsp_worktype type , struct vdsp_handle *handle);

/****************************************************************
name: sprd_cavdsp_close_device
description: close the virtual device opened by sprd_cavdsp_open_device
param:
vdsphandle is handl which get from sprd_cavdsp_open_device
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
SPRD_VDSP_RESULT_OLD_GENERATION   -----return failed
					because of xrp server may be restart,
					the generation is not matched
****************************************************************/
enum sprd_vdsp_result sprd_cavdsp_close_device(void *vdsphandle);

/****************************************************************
name: sprd_cavdsp_send_cmd
description: send command to vdsp
param:
handle  ---- handle get from sprd_cavdsp_open_device
nsid    ---- library name or system cmd name, the max size is 32 bytes
in      ---- input param
out     ---- output param
buffer  ---- buffer descriptor pointer which pointer the
	     bufnum of struct sprd_vdsp_client_inout
bufnum  ---- buffer number
priority ---- cmd priority from 0~?
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
SPRD_VDSP_RESULT_OLD_GENERATION   -----return failed
                                        because of xrp server may be restart,
                                        the generation is not matched
****************************************************************/
enum sprd_vdsp_result sprd_cavdsp_send_cmd(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out , 
						struct sprd_vdsp_client_inout *buffer , uint32_t bufnum , uint32_t priority);

/************************************************************
name: sprd_cavdsp_loadlibrary
description: load specified library
param:
handle   ------ handle get from sprd_cavdsp_open_device
libname  ------ library name which need be loaded , which max size is 32 bytes
buffer   ------ library code and data buffer ,which store the library code and data
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
SPRD_VDSP_RESULT_OLD_GENERATION   -----return failed
                                        because of xrp server may be restart,
                                        the generation is not matched
************************************************************/
enum sprd_vdsp_result sprd_cavdsp_loadlibrary(void *handle , const char *libname , struct sprd_vdsp_client_inout *buffer);

/************************************************************
name: sprd_cavdsp_unloadlibrary
description: unload specified library
param:
handle ---- handle get from sprd_cavdsp_open_device
libname ----library name which need be unloaded , which max size is 32 bytes
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
SPRD_VDSP_RESULT_OLD_GENERATION   -----return failed
                                        because of xrp server may be restart,
                                        the generation is not matched
************************************************************/
enum sprd_vdsp_result sprd_cavdsp_unloadlibrary(void *handle , const char *libname);

/***************
name: sprd_cavdsp_power_hint
description: set and cancel vdsp power hint level
param:
handle ---- handle get from sprd_cavdsp_open_device
level ---- power level which defined in enum sprd_vdsp_power_level
acquire_release----  SPRD_VDSP_POWERHINT_ACQUIRE is acqure power hint value, SPRD_VDSP_POWERHINT_RELEASE
is to cancel the power hint level
the acquire/release must be coupled and acqure/release the same level
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
SPRD_VDSP_RESULT_OLD_GENERATION   -----return failed
                                        because of xrp server may be restart,
                                        the generation is not matched
*************/
enum sprd_vdsp_result sprd_cavdsp_power_hint(void *handle , enum sprd_vdsp_power_level level , enum sprd_vdsp_powerhint_acquire_release acquire_release);


/*new interfaces*/
/*
name: sprd_cavdsp_open_device_compat
description: open device
type.work_type is SPRD_VDSP_WORK_NORMAL for arithmetic firmwwareand SPRD_VDSP_WORK_FACEID for faceid firmware
type.int_type DIRECTLY is for call driver directly, BYSERVER is for ipc call to vdsp server and vdsp server call driver
handle ---- output parameter the vdsp virtual device handle.
return value: 0 is ok, other value is failed
*/
enum sprd_vdsp_result sprd_cavdsp_open_device_compat(struct vdsp_open_param *type , void **handle);

/****************************************************************
name: sprd_cavdsp_close_device_compat
description: close vdsp virtual device opened by sprd_cavdsp_open_device_compat
param:
handle is handl which get from sprd_cavdsp_open_device_compat
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
SPRD_VDSP_RESULT_OLD_GENERATION   -----return failed , only BYSERVER mode supported
                                        because of xrp server may be restart,
                                        the generation is not matched
****************************************************************/
enum sprd_vdsp_result sprd_cavdsp_close_device_compat(void *handle);

/****************************************************************
name: sprd_cavdsp_send_cmd_compat
description: send command to vdsp
param:
handle  ---- handle get from sprd_cavdsp_open_device_compat
nsid    ---- library name or system cmd name, max size is 32 bytes
in      ---- input param
out     ---- output param
buffer  ---- buffer descriptor pointer which pointer the
             bufnum of struct sprd_vdsp_client_inout
bufnum  ---- buffer number
priority ---- cmd priority from 0~?
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
SPRD_VDSP_RESULT_OLD_GENERATION   -----return failed , only BYSERVER mode supported
                                        because of xrp server may be restart,
                                        the generation is not matched
****************************************************************/
enum sprd_vdsp_result sprd_cavdsp_send_cmd_compat(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out ,
                                                struct sprd_vdsp_client_inout *buffer , uint32_t bufnum , uint32_t priority);

/************************************************************
name: sprd_cavdsp_loadlibrary_compat
description: load specified library
param:
handle   ------ handle get from sprd_cavdsp_open_device_compat
libname  ------ library name which need be loaded , which max size is 32 bytes
buffer   ------ library code and data buffer ,which store the library code and data
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
SPRD_VDSP_RESULT_OLD_GENERATION   -----return failed , only BYSERVER mode supported
                                        because of xrp server may be restart,
                                        the generation is not matched
************************************************************/
enum sprd_vdsp_result sprd_cavdsp_loadlibrary_compat(void *handle , const char *libname , struct sprd_vdsp_client_inout *buffer);

/************************************************************
name: sprd_cavdsp_unloadlibrary_compat
description: unload specified library
param:
handle ---- handle get from sprd_cavdsp_open_device_compat
libname ----library name which need be unloaded , which max size is 32 bytes
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
SPRD_VDSP_RESULT_OLD_GENERATION   -----return failed  , only BYSERVER mode supported
                                        because of xrp server may be restart,
                                        the generation is not matched
************************************************************/
enum sprd_vdsp_result sprd_cavdsp_unloadlibrary_compat(void *handle , const char *libname);

/***************
name: sprd_cavdsp_power_hint_compat
description: set/cancel power hint level to vdsp system
param:
handle ---- handle get from sprd_cavdsp_open_device
level ---- power level which defined by enum sprd_vdsp_power_level
acquire_release----  SPRD_VDSP_POWERHINT_ACQUIRE is acqure power hint value, SPRD_VDSP_POWERHINT_RELEASE
is cancel the setting
the acquire/release must be coupled and acqure/release the same level
return value:
SPRD_VDSP_RESULT_SUCCESS   ------ ok
SPRD_VDSP_RESULT_FAIL    --------failed
SPRD_VDSP_RESULT_OLD_GENERATION   -----return failed , only BYSERVER mode supported
                                        because of xrp server may be restart,
                                        the generation is not matched
*************/
enum sprd_vdsp_result sprd_cavdsp_power_hint_compat(void *handle , enum sprd_vdsp_power_level level , enum sprd_vdsp_powerhint_acquire_release acquire_release);


/* ion mem operation*/
/***************
name: sprd_alloc_ionmem
description: alloc ion memory
param:
size ---- ion memory size need be allocated
iscache ---- cacheble attribute, 0 is uncacheble , 1 is cacheable
fd ----- output param , file descriptor of ion mem
viraddr ---- virtual address of ion memory allocated
return value:
!NULL is ok
NULL is failed
*************/
void* sprd_alloc_ionmem(uint32_t size, uint8_t iscache, int32_t* fd, void** viraddr);

/***********************************************************************
name: sprd_alloc_ionmem2
description: alloc ion memory with output virtual address and physical address
param:
size ---- ion memory size need be allocated
iscache ---- cacheble attribute, 0 is uncacheble , 1 is cacheable
fd ----- output param , file descriptor of ion mem
viraddr ---- virtual address of ion memory allocated
phyaddr ---- physical address of ion memory allocated
return value:
!NULL is ok
NULL is failed
***********************************************************************/
void* sprd_alloc_ionmem2(uint32_t size, uint8_t iscache, int32_t* fd, void** viraddr, unsigned long* phyaddr);

/***********************************************************************
name: sprd_free_ionmem
description: free ion memory
param:
handle ---- return value of sprd_alloc_ionmem or sprd_alloc_ionmem2
return value:
SPRD_XRP_STATUS_SUCCESS is success
SPRD_XRP_STATUS_FAILURE is failed
***********************************************************************/
enum sprd_vdsp_status sprd_free_ionmem(void* handle);

/***********************************************************************
name: sprd_flush_ionmem
description: flush cacheline of ion memory
param:
handle ---- return value of sprd_alloc_ionmem or sprd_alloc_ionmem2
vaddr ---- virtual address of ion memory
paddr ---- physical address of ion memory
size ---- ion memory size
return value:
SPRD_XRP_STATUS_SUCCESS is success
SPRD_XRP_STATUS_FAILURE is failed
***********************************************************************/
enum sprd_vdsp_status sprd_flush_ionmem(void* handle, void* vaddr, void* paddr, uint32_t size);

/***********************************************************************
name: sprd_invalid_ionmem
description: invalid cacheline of ion memory
param:
handle ---- return value of sprd_alloc_ionmem or sprd_alloc_ionmem2
return value:
SPRD_XRP_STATUS_SUCCESS is success
SPRD_XRP_STATUS_FAILURE is failed
***********************************************************************/
enum sprd_vdsp_status sprd_invalid_ionmem(void* handle);

/*v1.1 only inherit from v1.0 , in v1.1 only return fail*/
enum sprd_vdsp_result sprd_cavdsp_open_device_direct(enum sprd_vdsp_worktype type , void **handle);
enum sprd_vdsp_result sprd_cavdsp_close_device_direct(void *handle);
enum sprd_vdsp_result sprd_cavdsp_send_cmd_direct(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out ,
                                                struct sprd_vdsp_client_inout *buffer , uint32_t bufnum , uint32_t priority);

/*******************************
backup interface for compatiable, not support anything
*********************************/
enum sprd_vdsp_result sprd_cavdsp_ctrl(enum sprd_vdsp_ctrl_cmd cmd, void* input , void* output);

/*internal interfaces*/
enum sprd_vdsp_result sprd_vdsp_open_device_direc(enum sprd_vdsp_worktype type , void **handle);
enum sprd_vdsp_result sprd_vdsp_close_device_direc(void *handle);
enum sprd_vdsp_result sprd_vdsp_send_cmd_direc(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out ,
                                                struct sprd_vdsp_client_inout *buffer , uint32_t bufnum , uint32_t priority);
enum sprd_vdsp_result sprd_vdsp_loadlibrary_direc(void *handle , const char *libname , struct sprd_vdsp_client_inout *buffer);
enum sprd_vdsp_result sprd_vdsp_unloadlibrary_direc(void *handle , const char *libname);
enum sprd_vdsp_result sprd_vdsp_power_hint_direc(void *handle , enum sprd_vdsp_power_level level , enum sprd_vdsp_powerhint_acquire_release acquire_release);



#ifdef __cplusplus
}
#endif

#endif

