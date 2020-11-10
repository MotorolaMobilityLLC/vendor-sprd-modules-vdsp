#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include <cutils/log.h>
#include "vdsp_interface.h"
#include "xrp_interface.h"
#include "vdsp_interface_internal.h"
#include "vdsp_server_interface.h"
#include "vdsp_dvfs.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG  "vdsp_interface"
#endif

//using namespace android;

#define USE_FD_MAX_NUM  32
#define CHECK_SUM_PROPERTY   "persist.vendor.vdsp.checksum"

static char version[] = "ver_1.1";

struct vdsp_wrap_handle
{
	enum sprd_vdsp_interface_type type;
	void *handle;
};

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_get_info(enum sprd_vdsp_getinfo_cmd cmd , void *output)
{
	enum sprd_vdsp_result ret = SPRD_VDSP_RESULT_SUCCESS;
	switch(cmd) {
	case SPRD_VDSP_GET_VERSION:
		strcpy((char*)output , version);
		ALOGD("func:%s ver:%s" , __func__ ,  output);
		break;
	default:
		ret = SPRD_VDSP_RESULT_FAIL;
		ALOGD("func:%s cmd:%d" , __func__ , cmd);
		break;
	}
	return ret;
}



__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_open_device(enum sprd_vdsp_worktype type , struct vdsp_handle *handle)
{
	void *hnddev = NULL;
	hnddev = sprd_vdsp_open_device(0 , type);
	if((hnddev == NULL) || (handle == NULL)) {
		if(hnddev != NULL) {
			sprd_vdsp_release_device(hnddev);
		}
		ALOGD("func:%s failed type:%d\n" , __func__ , type);
		return SPRD_VDSP_RESULT_FAIL;
	}
	handle->fd = (int32_t)((unsigned long long)hnddev & 0xffffffff); /*low 32 bit*/
	handle->generation = (unsigned int)(((unsigned long long)hnddev>>32) & 0xffffffff); /*high 32 bit*/

	ALOGD("func:%s ok type:%d handle:%p , handlefd:%x, genera:%x\n" , __func__ , type , hnddev , handle->fd , handle->generation);
	return SPRD_VDSP_RESULT_SUCCESS;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_close_device(void *handle)
{
	struct vdsp_handle *vdsphnd = (struct vdsp_handle*) handle;
	void *hnd = NULL;
	if(vdsphnd == NULL) {
		ALOGD("func:%s failed handle is NULL\n" , __func__);
		return SPRD_VDSP_RESULT_FAIL;
	}
	hnd = (void*)(((((unsigned long long)vdsphnd->generation) & 0xffffffff)<<32) | (((unsigned long long)vdsphnd->fd) & 0xffffffff));
	if(hnd == NULL) {
		ALOGD("func:%s failed real handle is NULL\n" , __func__);
		return SPRD_VDSP_RESULT_FAIL;
	}
	sprd_vdsp_release_device(hnd);
	return SPRD_VDSP_RESULT_SUCCESS;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_send_cmd(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out ,
                                                struct sprd_vdsp_client_inout *buffer , uint32_t bufnum , uint32_t priority)
{
	int32_t ret1;
	struct vdsp_handle *vdsphnd = (struct vdsp_handle*) handle;
	void *hnd = NULL;
	if(vdsphnd == NULL) {
		ALOGD("func:%s failed handle is NULL\n" , __func__);
		return SPRD_VDSP_RESULT_FAIL;
	}
	hnd = (void*)(((((unsigned long long)vdsphnd->generation) & 0xffffffff)<<32) | (((unsigned long long)vdsphnd->fd) & 0xffffffff));
	if(hnd == NULL) {
		ALOGD("func:%s failed real handle is NULL\n" , __func__);
		return SPRD_VDSP_RESULT_FAIL;
	}
        ret1 = sprd_vdsp_send_command_directly(hnd , nsid ,(struct sprd_vdsp_inout*)in, (struct sprd_vdsp_inout*)out,
                                                        (struct sprd_vdsp_inout*)buffer ,bufnum,(enum sprd_xrp_queue_priority) priority);
        if(ret1 == 0) {
                ALOGD("func:%s result ok\n" , __func__);
                return SPRD_VDSP_RESULT_SUCCESS;
        }
        else {
                ALOGD("func:%s result fail\n" , __func__);
                return SPRD_VDSP_RESULT_FAIL;
        }
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_loadlibrary(void *handle , const char *libname , struct sprd_vdsp_client_inout *buffer)
{
	int ret;
        struct vdsp_handle *vdsphnd = (struct vdsp_handle*) handle;
        void *hnd = NULL;
        if(vdsphnd == NULL) {
                ALOGD("func:%s failed handle is NULL\n" , __func__);
                return SPRD_VDSP_RESULT_FAIL;
        }
	hnd = (void*)(((((unsigned long long)vdsphnd->generation) & 0xffffffff)<<32) | (((unsigned long long)vdsphnd->fd) & 0xffffffff));
        if(hnd == NULL) {
                ALOGD("func:%s failed real handle is NULL\n" , __func__);
                return SPRD_VDSP_RESULT_FAIL;
        }
	ALOGD("func:%s hnd:%p , generation:%x, fd:%x" , __func__ , hnd , vdsphnd->generation , vdsphnd->fd);
	ret = sprd_vdsp_load_library(hnd , (struct sprd_vdsp_inout*)buffer , libname , SPRD_XRP_PRIORITY_2);
        if(ret == SPRD_XRP_STATUS_SUCCESS) {
                return SPRD_VDSP_RESULT_SUCCESS;
        } else {
                return SPRD_VDSP_RESULT_FAIL;
        }
	//return SPRD_VDSP_RESULT_FAIL;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_unloadlibrary(void *handle , const char *libname)
{
	int ret;
	struct vdsp_handle *vdsphnd = (struct vdsp_handle*) handle;
        void *hnd = NULL;
        if(vdsphnd == NULL) {
                ALOGD("func:%s failed handle is NULL\n" , __func__);
                return SPRD_VDSP_RESULT_FAIL;
        }
	hnd = (void*)(((((unsigned long long)vdsphnd->generation) & 0xffffffff)<<32) | (((unsigned long long)vdsphnd->fd) & 0xffffffff));
        if(hnd == NULL) {
                ALOGD("func:%s failed real handle is NULL\n" , __func__);
                return SPRD_VDSP_RESULT_FAIL;
        }
        ret = sprd_vdsp_unload_library(hnd , libname , SPRD_XRP_PRIORITY_2);
        if(ret == SPRD_XRP_STATUS_SUCCESS) {
                return SPRD_VDSP_RESULT_SUCCESS;
        } else {
                return SPRD_VDSP_RESULT_FAIL;
        }
}
__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_power_hint(void *handle , enum sprd_vdsp_power_level level , enum sprd_vdsp_powerhint_acquire_release acquire_release)
{
	int ret;
	struct vdsp_handle *vdsphnd = (struct vdsp_handle*) handle;
        void *hnd = NULL;
        if(vdsphnd == NULL) {
                ALOGD("func:%s failed handle is NULL\n" , __func__);
                return SPRD_VDSP_RESULT_FAIL;
        }
	hnd = (void*)(((((unsigned long long)vdsphnd->generation) & 0xffffffff)<<32) | (((unsigned long long)vdsphnd->fd) & 0xffffffff));
        if(hnd == NULL) {
                ALOGD("func:%s failed real handle is NULL\n" , __func__);
                return SPRD_VDSP_RESULT_FAIL;
        }
        ret = set_powerhint_flag(hnd , level , acquire_release);
        if(ret == 0)
                return SPRD_VDSP_RESULT_SUCCESS;
        else
                return SPRD_VDSP_RESULT_FAIL;
}


enum sprd_vdsp_result sprd_vdsp_open_device_direc(enum sprd_vdsp_worktype type , void **handle) {
	*handle = sprd_vdsp_open_device(0 , type);
	if(*handle == NULL) {
		ALOGD("func:%s failed type:%d\n" , __func__ , type);
		return SPRD_VDSP_RESULT_FAIL;
	}
	ALOGD("func:%s ok type:%d\n" , __func__ , type);
	return SPRD_VDSP_RESULT_SUCCESS;
}

enum sprd_vdsp_result sprd_vdsp_close_device_direc(void *handle) {
	sprd_vdsp_release_device(handle);
	ALOGD("func:%s release device handle:%p\n" , __func__ , handle);
	return SPRD_VDSP_RESULT_SUCCESS;
}

enum sprd_vdsp_result sprd_vdsp_send_cmd_direc(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out ,
                                                struct sprd_vdsp_client_inout *buffer , uint32_t bufnum , uint32_t priority) {

	int32_t ret1;
	ret1 = sprd_vdsp_send_command_directly(handle , nsid ,(struct sprd_vdsp_inout*)in, (struct sprd_vdsp_inout*)out,
							(struct sprd_vdsp_inout*)buffer ,bufnum,(enum sprd_xrp_queue_priority) priority);
	if(ret1 == 0) {
		ALOGD("func:%s result ok\n" , __func__);
		return SPRD_VDSP_RESULT_SUCCESS;
	}
	else {
		ALOGD("func:%s result fail\n" , __func__);
		return SPRD_VDSP_RESULT_FAIL;
	}
}
enum sprd_vdsp_result sprd_vdsp_loadlibrary_direc(void *handle , const char *libname , struct sprd_vdsp_client_inout *buffer)
{
	int ret;
	ret = sprd_vdsp_load_library(handle , (struct sprd_vdsp_inout*)buffer , libname , SPRD_XRP_PRIORITY_2);
	if(ret == SPRD_XRP_STATUS_SUCCESS) {
		return SPRD_VDSP_RESULT_SUCCESS;
	} else {
		return SPRD_VDSP_RESULT_FAIL;
	}
}
enum sprd_vdsp_result sprd_vdsp_unloadlibrary_direc(void *handle , const char *libname)
{
	int ret;
	ret = sprd_vdsp_unload_library(handle , libname , SPRD_XRP_PRIORITY_2);
	if(ret == SPRD_XRP_STATUS_SUCCESS) {
		return SPRD_VDSP_RESULT_SUCCESS;
	} else {
		return SPRD_VDSP_RESULT_FAIL;
	}
}
enum sprd_vdsp_result sprd_vdsp_power_hint_direc(void *handle , enum sprd_vdsp_power_level level , enum sprd_vdsp_powerhint_acquire_release acquire_release)
{
	int ret;
	ret = set_powerhint_flag(handle , level , acquire_release);
	if(ret == 0)
		return SPRD_VDSP_RESULT_SUCCESS;
	else
		return SPRD_VDSP_RESULT_FAIL;
}



__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_open_device_compat(struct vdsp_open_param *type , void **handle)
{
        enum sprd_vdsp_result ret = SPRD_VDSP_RESULT_FAIL;
	void *directhandle = NULL;
	struct vdsp_wrap_handle *wrap_handle = (struct vdsp_wrap_handle *) malloc(sizeof(struct vdsp_wrap_handle));
	if(wrap_handle == NULL) {
		ALOGE("func:%s alloc wrap handle is NULL" , __func__);
		return ret;
	}
	*handle = NULL;
        switch(type->int_type)
        {
        case SPRD_VDSP_INTERFACE_DIRECTLY:
		ret = sprd_vdsp_open_device_direc(type->work_type , &directhandle);
		if(ret == SPRD_VDSP_RESULT_SUCCESS) {
			wrap_handle->type = SPRD_VDSP_INTERFACE_DIRECTLY;
			wrap_handle->handle = directhandle;
			*handle = wrap_handle;
		}
		else {
			free(wrap_handle);
		}
                break;
        case SPRD_VDSP_INTERFACE_BYSERVER:
	{
		struct vdsp_handle *vdsphandle = (struct vdsp_handle*) malloc(sizeof(struct vdsp_handle));
		if(vdsphandle == NULL)
		{
			free(wrap_handle);
			return ret;
		}
		ret = sprd_vdsp_open_device_serv(type->work_type , vdsphandle);
		if(ret == SPRD_VDSP_RESULT_SUCCESS) {
			wrap_handle->type = SPRD_VDSP_INTERFACE_BYSERVER;
			wrap_handle->handle = vdsphandle;
			*handle = wrap_handle;
		}
		else
		{
			free(wrap_handle);
			free(vdsphandle);
		}
                break;
        }
	default:
		free(wrap_handle);
		ALOGE("func:%s , error int type:%d" , __func__ , type->int_type);
                break;
        }
	ALOGD("func:%s result:%d" , __func__ , ret);
        return ret;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_close_device_compat(void *handle)
{
	enum sprd_vdsp_result ret = SPRD_VDSP_RESULT_FAIL;
	struct vdsp_wrap_handle *wrap_handle = (struct vdsp_wrap_handle *) handle;
	if((wrap_handle == NULL) || (wrap_handle->handle == NULL))
	{
		ALOGE("func:%s ret:%d" , __func__ ,ret);
		return ret;
	}
	switch(wrap_handle->type)
	{
	case SPRD_VDSP_INTERFACE_DIRECTLY:
		ret = sprd_vdsp_close_device_direc(wrap_handle->handle);
		if(ret == SPRD_VDSP_RESULT_SUCCESS)
		{
			free(wrap_handle);
		}
		break;
	case SPRD_VDSP_INTERFACE_BYSERVER:
		ret = sprd_vdsp_close_device_serv(wrap_handle->handle);
		if((ret == SPRD_VDSP_RESULT_SUCCESS) || (ret == SPRD_VDSP_RESULT_OLD_GENERATION))
		{
			free(wrap_handle->handle);
			free(wrap_handle);
		}
		break;
	default:
		break;
	}
	return ret;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_send_cmd_compat(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out ,
                                                struct sprd_vdsp_client_inout *buffer , uint32_t bufnum , uint32_t priority)
{
	struct vdsp_wrap_handle *wrap_handle = (struct vdsp_wrap_handle *) handle;
	enum sprd_vdsp_result ret = SPRD_VDSP_RESULT_FAIL;
	if((wrap_handle == NULL) || (wrap_handle->handle == NULL)) {
		ALOGE("func:%s some handle is null wrap_handle:%p" , __func__ , wrap_handle);
		return ret;
	}
	switch(wrap_handle->type) {
	case SPRD_VDSP_INTERFACE_DIRECTLY:
		ret = sprd_vdsp_send_cmd_direc(wrap_handle->handle , nsid , in ,out , buffer ,bufnum , priority);
		break;
	case SPRD_VDSP_INTERFACE_BYSERVER:
		ret = sprd_vdsp_send_cmd_serv(wrap_handle->handle , nsid , in ,out , buffer ,bufnum , priority);
		break;
	default:
		break;
	}
	return ret;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_loadlibrary_compat(void *handle , const char *libname , struct sprd_vdsp_client_inout *buffer)
{
	struct vdsp_wrap_handle *wrap_handle = (struct vdsp_wrap_handle *) handle;
	enum sprd_vdsp_result ret = SPRD_VDSP_RESULT_FAIL;
	if((wrap_handle == NULL) || (wrap_handle->handle == NULL)) {
		ALOGE("func:%s some handle is null wrap_handle:%p" , __func__ , wrap_handle);
		return ret;
	}
	switch(wrap_handle->type) {
	case SPRD_VDSP_INTERFACE_DIRECTLY:
		ret = sprd_vdsp_loadlibrary_direc(wrap_handle->handle , libname , buffer);
		break;
	case SPRD_VDSP_INTERFACE_BYSERVER:
		ret = sprd_vdsp_loadlibrary_serv(wrap_handle->handle , libname , buffer);
		break;
	default:
		break;
	}
	return ret;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_unloadlibrary_compat(void *handle , const char *libname)
{
	struct vdsp_wrap_handle *wrap_handle = (struct vdsp_wrap_handle *) handle;
	enum sprd_vdsp_result ret = SPRD_VDSP_RESULT_FAIL;
	if((wrap_handle == NULL) || (wrap_handle->handle == NULL)) {
		ALOGE("func:%s some handle is null wrap_handle:%p" , __func__ , wrap_handle);
		return ret;
	}
	switch(wrap_handle->type) {
	case SPRD_VDSP_INTERFACE_DIRECTLY:
		ret = sprd_vdsp_unloadlibrary_direc(wrap_handle->handle , libname);
		break;
	case SPRD_VDSP_INTERFACE_BYSERVER:
		ret = sprd_vdsp_unloadlibrary_serv(wrap_handle->handle , libname);
		break;
	default:
		break;
	}
	return ret;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_power_hint_compat(void *handle , enum sprd_vdsp_power_level level , enum sprd_vdsp_powerhint_acquire_release acquire_release)
{
	struct vdsp_wrap_handle *wrap_handle = (struct vdsp_wrap_handle *) handle;
	enum sprd_vdsp_result ret = SPRD_VDSP_RESULT_FAIL;
	if((wrap_handle == NULL) || (wrap_handle->handle == NULL)) {
		ALOGE("func:%s some handle is null wrap_handle:%p" , __func__ , wrap_handle);
		return ret;
	}
	switch(wrap_handle->type) {
	case SPRD_VDSP_INTERFACE_DIRECTLY:
		ret = sprd_vdsp_power_hint_direc(wrap_handle->handle , level , acquire_release);
		break;
	case SPRD_VDSP_INTERFACE_BYSERVER:
		ret = sprd_vdsp_power_hint_serv(wrap_handle->handle , level , acquire_release);
		break;
	default:
		break;
	}
	return ret;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_open_device_direct(enum sprd_vdsp_worktype type , void **handle)
{
	return sprd_vdsp_open_device_direc(type , handle);
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_close_device_direct(void *handle)
{
	return sprd_vdsp_close_device_direc(handle);
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_send_cmd_direct(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out , struct sprd_vdsp_client_inout *buffer , uint32_t bufnum , uint32_t priority)
{
	return sprd_vdsp_send_cmd_direc(handle , nsid ,in, out , buffer , bufnum ,priority);
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_ctrl(enum sprd_vdsp_ctrl_cmd cmd, __unused void* input , __unused void* output)
{
	enum sprd_vdsp_result ret = SPRD_VDSP_RESULT_NOT_SUPPORT;
	switch(cmd)
	{
	default:
		break;
	}
	return ret;
}

__attribute__ ((visibility("default"))) int sprd_vdsp_check_supported()
{
        char l5ptype[PROP_VALUE_MAX];
        property_get("ro.boot.auto.efuse", l5ptype , "-1");
        if(0 == strcmp(l5ptype , "T618")) {
                return 1;
        } else if(0 == strcmp(l5ptype , "T610")) {
                return 0;
        }
        // other type support as default.
        return 1;
}
