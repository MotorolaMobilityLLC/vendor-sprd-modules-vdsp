

#include <cutils/properties.h>
#include <cutils/log.h>
#include "vdsp_interface.h"
#include "xrp_interface.h"
#include "vdsp_interface_internal.h"
#include "vdsp_dvfs.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG  "vdsp_interface"
#endif

#define USE_FD_MAX_NUM  32
#define CHECK_SUM_PROPERTY   "persist.vendor.vdsp.checksum"




__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_open_device(enum sprd_vdsp_worktype type , struct vdsp_handle *handle)
{
	void *hnddev = NULL;
	hnddev = sprd_vdsp_open_device(0 , type);
	if((hnddev == NULL) || (handle == NULL)) {
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


__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_open_device_direct(enum sprd_vdsp_worktype type , void **handle) {
	*handle = sprd_vdsp_open_device(0 , type);
	if(*handle == NULL) {
		ALOGD("func:%s failed type:%d\n" , __func__ , type);
		return SPRD_VDSP_RESULT_FAIL;
	}
	ALOGD("func:%s ok type:%d\n" , __func__ , type);
	return SPRD_VDSP_RESULT_SUCCESS;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_close_device_direct(void *handle) {
	sprd_vdsp_release_device(handle);
	ALOGD("func:%s release device handle:%p\n" , __func__ , handle);
	return SPRD_VDSP_RESULT_SUCCESS;
}

__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_send_cmd_direct(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out ,
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
__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_loadlibrary_direct(void *handle , const char *libname , struct sprd_vdsp_client_inout *buffer)
{
	int ret;
	ret = sprd_vdsp_load_library(handle , (struct sprd_vdsp_inout*)buffer , libname , SPRD_XRP_PRIORITY_2);
	if(ret == SPRD_XRP_STATUS_SUCCESS) {
		return SPRD_VDSP_RESULT_SUCCESS;
	} else {
		return SPRD_VDSP_RESULT_FAIL;
	}
}
__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_unloadlibrary_direct(void *handle , const char *libname)
{
	int ret;
	ret = sprd_vdsp_unload_library(handle , libname , SPRD_XRP_PRIORITY_2);
	if(ret == SPRD_XRP_STATUS_SUCCESS) {
		return SPRD_VDSP_RESULT_SUCCESS;
	} else {
		return SPRD_VDSP_RESULT_FAIL;
	}
}
__attribute__ ((visibility("default"))) enum sprd_vdsp_result sprd_cavdsp_power_hint_direct(void *handle , enum sprd_vdsp_power_level level , enum sprd_vdsp_powerhint_acquire_release acquire_release)
{
	int ret;
	ret = set_powerhint_flag(handle , level , acquire_release);
	if(ret == 0)
		return SPRD_VDSP_RESULT_SUCCESS;
	else
		return SPRD_VDSP_RESULT_FAIL;
}
