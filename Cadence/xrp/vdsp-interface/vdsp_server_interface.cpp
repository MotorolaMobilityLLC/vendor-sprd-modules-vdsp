/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vendor/sprd/hardware/vdsp/1.0/IVdspService.h>
#include "vdsp_interface.h"
#include <cutils/properties.h>
#include "xrp_interface.h"
#include "VdspService.h"
#include "vdsp_interface_internal.h"
#include "vdsp_server_interface.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG  "VdspServerClient"
#endif

#define USE_FD_MAX_NUM  32
#define CHECK_SUM_PROPERTY   "persist.vendor.vdsp.checksum"

using namespace android;
using IVdspService = ::vendor::sprd::hardware::vdsp::V1_0::IVdspService;
using IBase = ::android::hidl::base::V1_0::IBase;
using namespace vendor::sprd::hardware::vdsp::V1_0::implementation;

struct fd_info
{
	Mutex fd_mutex;
	int32_t working;
};
static int32_t gInitFlag = 0;
static uint32_t gGeneration = 0;
static Mutex gLock;
static sp<IVdspService> gVdspService;
static sp<IBase> gfakeBinder;
static int32_t gOpenedCount = 0;
static enum sprd_vdsp_worktype gWorkType = SPRD_VDSP_WORK_MAXTYPE;
static int32_t gWorking = 0;
static struct fd_info g_FdInfo[USE_FD_MAX_NUM];
static uint32_t g_fdtable = 0;

static int32_t create_hidl_params(native_handle_t **handle_input , struct sprd_vdsp_client_inout *in , uint32_t num)
{
	uint32_t i;
	if(num == 0) {
		return -1;
	}
	*handle_input = native_handle_create(num , num*SPRD_VDSP_HANDLE_INT_NUM);
	if(*handle_input == NULL) {
		return -1;
	}
	for(i = 0; i < num; i++) {
		(*handle_input)->data[i] = in[i].fd;
		(*handle_input)->data[num + i*SPRD_VDSP_HANDLE_INT_NUM + SPRD_VDSP_HANDLE_PHYADDR_OFFSET -1] = in[i].phy_addr;
		(*handle_input)->data[num + i*SPRD_VDSP_HANDLE_INT_NUM + SPRD_VDSP_HANDLE_SIZE_OFFSET -1] = in[i].size;
		(*handle_input)->data[num + i*SPRD_VDSP_HANDLE_INT_NUM + SPRD_VDSP_HANDLE_FLAG_OFFSET -1] = in[i].flag;
		ALOGD("------fuun:%s i:%d ,fd:%d , phyaddr:%x, size:%x, flag:%x" , __func__ , i , (*handle_input)->data[i],
		(*handle_input)->data[num + i*SPRD_VDSP_HANDLE_INT_NUM + SPRD_VDSP_HANDLE_PHYADDR_OFFSET -1] ,  (*handle_input)->data[num + i*SPRD_VDSP_HANDLE_INT_NUM + SPRD_VDSP_HANDLE_SIZE_OFFSET -1] , (*handle_input)->data[num + i*SPRD_VDSP_HANDLE_INT_NUM + SPRD_VDSP_HANDLE_FLAG_OFFSET -1]);
	}
	return 0;
	
}

class VdspClientDeathRecipient: public hardware::hidl_death_recipient
{
public:
        VdspClientDeathRecipient(){}
        // IBinder::DeathRecipient
        virtual void serviceDied(__unused uint64_t cookie , __unused const ::android::wp<::android::hidl::base::V1_0::IBase>& who)
	{
		AutoMutex _l(gLock);
		gInitFlag = 0;
		gGeneration ++;
		ALOGD("binderDied func:%s , gGeneration:%d\n" , __func__ , gGeneration);
	}
};
static sp<VdspClientDeathRecipient> gDeathRecipient = NULL;
static int32_t check_sum_flag()
{
	char value[128];
	ALOGD("func:%s" , __func__);
	property_get(CHECK_SUM_PROPERTY , value , "");
	if(!strcmp(value ,  "1"))
		return 1;
	else
		return 0;
}
static void print_checksum_result(const char *nsid , const char *info , struct VdspInputOutput* buffer , uint32_t num)
{
	uint32_t i , j;
	uint64_t *result;
	uint8_t *temp;
	uint64_t tempresult = 0;
	if(buffer == NULL)
		return;
	result = (uint64_t*)malloc(num*sizeof(uint64_t));
	for(i = 0; i < num; i++){
		for(j =0; j < buffer[i].size; j++) {
			temp = (uint8_t*)buffer[i].viraddr;
			if(j == 0)
				tempresult = ((uint64_t)temp[j] + (uint64_t)temp[j+1]);
			else
				tempresult = ((uint64_t)tempresult + (uint64_t)temp[j+1]);
		}
		result[i] = tempresult;
	}
	for(i = 0; i < num; i++) {
		ALOGD("func:%s nsid:%s %s index:%d , sum:%llx" , __func__ , nsid , info , i , (unsigned long long)result[i]);
	}
	free(result);
}
static int32_t Alloc_Fd()
{
        uint32_t valid_bits = (g_fdtable ^ 0xffffffff);
        for(int i =0; i < USE_FD_MAX_NUM; i++) {
                if(((valid_bits >> i) & 0x1)) {
			g_fdtable |= (1<<i);
                        return i;
                }
        }
        return -1;
}
static int32_t Free_Fd(int32_t fd)
{
        if(fd < 0)
                return -1;
        /*check whether fd is valid*/
        if((g_fdtable & (1<<fd)) == 0) {
                ALOGE("func:%s , free fd:%d , is not valid, g_fdtable:%x\n" , __func__ , fd , g_fdtable);
                return -1;
        }
        g_fdtable &= ~(1<<fd);
        return 0;
}
static int32_t Check_FdValid(int32_t fd) {
        if(fd < 0)
                return -1;
        if((g_fdtable & (1<<fd)) == 0) {
                ALOGE("func:%s , fd:%d , is not valid, g_fdtable:%x\n" , __func__ , fd , g_fdtable);
                return -1;
        }
        return 0;
}
static int32_t Check_GenrationValid(uint32_t generation)
{
	if(generation == gGeneration)
	{
		return 0;
	}
	return -1;
}
static sp<IVdspService> getVdspService(int32_t realget)
{
	AutoMutex _l(gLock);
	int ret;
	ALOGD("func:%s  , gInitFlag:%d\n" , __func__ , gInitFlag);
	if((0 == gInitFlag) && (realget != 0))
	{
		sp <IVdspService> binder = IVdspService::getService();
		if(binder != NULL) {
			g_fdtable = 0;
			gWorkType = SPRD_VDSP_WORK_MAXTYPE;
			for(int i = 0; i< USE_FD_MAX_NUM; i++)
			{
				g_FdInfo[i].working = 0;
			}
			if(gDeathRecipient == NULL) {
				gDeathRecipient = new VdspClientDeathRecipient();
			}
			ret = binder->linkToDeath(gDeathRecipient , 0);
			gVdspService = binder;
			gfakeBinder =  new IBase();
			ALOGD("func:%s  , gDeathRecipient:%p , linkTodeath:%d , gfakeBinder:%p , errno:%d\n" , __func__ , gDeathRecipient.get() ,ret ,gfakeBinder.get() , errno);
			gInitFlag = 1;
			gOpenedCount = 0;
		}
	}
	ALOGD("func:%s  , gInitFlag:%d , xrpService:%p\n" , __func__ , gInitFlag , gVdspService.get());
	return gVdspService;
}
static int32_t putVdspService()
{
	if(gInitFlag != 0) {
		gVdspService->unlinkToDeath(gDeathRecipient);
		gVdspService.clear();
		gfakeBinder.clear();
		gInitFlag = 0;
		gOpenedCount = 0;
	}
	return 0;
}

enum sprd_vdsp_result sprd_vdsp_open_device_serv(enum sprd_vdsp_worktype type , struct vdsp_handle *handle)
{
	sp<IVdspService> cs = NULL;
	enum sprd_vdsp_result result;
	int32_t ret;
	int32_t fd = -1;
	cs = getVdspService(1);
	if((handle == NULL) || (cs == NULL))
	{
		 ALOGE("func:%s get resource failed cs:%p , handle:%p\n" , __func__ ,cs.get() , handle);
		return SPRD_VDSP_RESULT_FAIL;
	}
	ALOGD("func:%s enter cs:%p\n" , __func__ ,cs.get());
	{
		AutoMutex _l(gLock);
		handle->fd = -1;
		handle->generation = 0xffffffff;
		fd = Alloc_Fd();
		if(fd < 0)
		{
			 ALOGE("func:%s Alloc_Fd faild fd:%d\n" , __func__ , fd);
			goto __openerror;;
		}
		handle->fd = fd;
		handle->generation = gGeneration;
		if(0 == gOpenedCount)
		{
			ALOGD("func:%s , gfakeBinder:%p" , __func__ , gfakeBinder.get());
			ret = cs->openXrpDevice(gfakeBinder , type);
			result = (enum sprd_vdsp_result) ret;
			if(result == SPRD_VDSP_RESULT_SUCCESS)
			{
				gOpenedCount ++;
				gWorkType = type;
				ALOGD("func:%s gOpenedCount 0 fd:%d\n" , __func__ , fd);
				goto __openok;
			}
			else {
				ALOGE("func:%s openXrpDevice failed fd:%d\n" , __func__ , fd);
				goto __openerror;
			}
		} else {
			if(type == gWorkType) {
				gOpenedCount++;
			} else {
				ALOGE("func:%s openXrpDevice fail type:%d, gWorkType:%d\n" , __func__ , type , gWorkType);
				Free_Fd(fd);
				goto __openerror;
			}
		}
		ALOGD("func:%s fd:%d\n" , __func__ , fd);

		goto __openok;
		//return (enum sprd_vdsp_result) cs->openXrpDevice(gfakeBinder);
	}
__openerror:
	if(fd > 0)
		Free_Fd(fd);
	return SPRD_VDSP_RESULT_FAIL;
__openok:
	return SPRD_VDSP_RESULT_SUCCESS;
}

enum sprd_vdsp_result sprd_vdsp_close_device_serv(void *handle)
{
	sp<IVdspService> cs = NULL;
	enum sprd_vdsp_result result;
	int32_t ret;
	uint32_t generation;
	int32_t fd;
	struct vdsp_handle *hnd = (struct vdsp_handle*)handle;
	if(handle == NULL)
	{
		/*fd is abnormal value*/
		 ALOGE("func:%s close invalid input hadle is NULL\n" , __func__);
		return SPRD_VDSP_RESULT_FAIL;
	}
	generation = hnd->generation;
	fd = hnd->fd;
	cs = getVdspService(0);
	if(cs != NULL)
	{
		AutoMutex _l(gLock);
		if(0 != Check_GenrationValid(generation))
		{
			ALOGE("func:%s Check_GenrationValid failed generation:%d , gGeneration:%d\n" , __func__ ,
				generation , gGeneration);
			gLock.unlock();
			return SPRD_VDSP_RESULT_OLD_GENERATION;
		}
		if(0 != Check_FdValid(fd))
		{
			ALOGE("func:%s Check_FdValid failed fd:%d\n" , __func__ , fd);
			return SPRD_VDSP_RESULT_FAIL;
		}
		gOpenedCount--;
		if(gOpenedCount == 0)
		{
			if(gWorking != 0)
			{
				ALOGE("func:%s gWorking is not zerofd:%d\n" , __func__ , fd);
				return SPRD_VDSP_RESULT_FAIL;
			}
			ret = cs->closeXrpDevice(gfakeBinder);
			result = (enum sprd_vdsp_result) ret;
			if(result == SPRD_VDSP_RESULT_SUCCESS)
			{
				Free_Fd(fd);
				putVdspService();
				ALOGD("func:%s closeXrpDevice ok fd:%d\n" , __func__ , fd);
				gWorkType = SPRD_VDSP_WORK_MAXTYPE;
				return SPRD_VDSP_RESULT_SUCCESS;
			}
			else {
				ALOGE("func:%s closeXrpDevice failed fd:%d\n" , __func__ , fd);
				return SPRD_VDSP_RESULT_FAIL;
			}
		}
		if(g_FdInfo[fd].working == 0){
			Free_Fd(fd);
			ALOGD("func:%s freefd ok fd:%d\n" , __func__ , fd);
			return SPRD_VDSP_RESULT_SUCCESS;
		}
		else {
			ALOGE("func:%s freefd faild working fd:%d\n" , __func__ , fd);
			return SPRD_VDSP_RESULT_FAIL;
		}
	}
	return SPRD_VDSP_RESULT_FAIL;
}

enum sprd_vdsp_result sprd_vdsp_send_cmd_serv(void *handle , const char *nsid , struct sprd_vdsp_client_inout *in, struct sprd_vdsp_client_inout *out ,
                                                struct sprd_vdsp_client_inout *buffer , uint32_t bufnum , uint32_t priority)
{
	sp<IVdspService> cs = NULL;
	enum sprd_vdsp_result result;
	int32_t ret;
	uint32_t generation;
	uint32_t fd;
	hidl_handle input;
	hidl_handle output;
	hidl_handle buffers;
	native_handle_t *handle_input = NULL;
	native_handle_t *handle_output = NULL;
	native_handle_t *handle_buffers = NULL;
	struct vdsp_handle *hnd = (struct vdsp_handle*) handle;
	cs = getVdspService(0);
	if((cs == NULL) || (NULL == handle))
	{
		/*fd is abnormal value*/
                 ALOGE("func:%s err param cs:%p , handle:%p\n" , __func__ , cs.get() , handle);
                return SPRD_VDSP_RESULT_FAIL;
	}
	generation = hnd->generation;
	fd = hnd->fd;
	//if(cs != NULL)
	{
		gLock.lock();
		if(0 != Check_GenrationValid(generation))
		{
			ALOGE("func:%s Check_GenrationValid failed generation:%d , gGeneration:%d\n" , __func__ ,
				generation , gGeneration);
			ret = SPRD_VDSP_RESULT_OLD_GENERATION;
			goto __err_return_unlock;
		}
		if(0 != Check_FdValid(fd))
                {
			ALOGE("func:%s Check_FdValid failed fd:%d\n" , __func__ , fd);
			ret = SPRD_VDSP_RESULT_FAIL;
			goto __err_return_unlock;
                }
		if(gOpenedCount == 0)
		{
			ALOGE("func:%s gOpenedCount 0 failed fd:%d\n" , __func__ , fd);
			ret = SPRD_VDSP_RESULT_FAIL;
			goto __err_return_unlock;
		}
		/*input*/
		if(in != NULL) {
			ret = create_hidl_params(&handle_input , in , 1);
			if(ret != 0) {
				ret = SPRD_VDSP_RESULT_FAIL;
				goto __err_return_unlock;
			}
		}
		/*output*/
		if(out != NULL) {
			ret = create_hidl_params(&handle_output , out , 1);
			if(ret != 0) {
				native_handle_delete(handle_input);
				ret = SPRD_VDSP_RESULT_FAIL;
				goto __err_return_unlock;
			}
		}
		input = handle_input;
		output = handle_output;
		/*buffer*/
		if(bufnum != 0) {
			ret = create_hidl_params(&handle_buffers , buffer , bufnum);
			if(ret != 0) {
                        	native_handle_delete(handle_input);
				native_handle_delete(handle_output);
				ret = SPRD_VDSP_RESULT_FAIL;
				goto __err_return_unlock;
			}
		}
		buffers = handle_buffers;
		if(check_sum_flag()) {
                        print_checksum_result(nsid , "buffer" , (struct VdspInputOutput*)buffer , bufnum);
                        print_checksum_result(nsid , "input" , (struct VdspInputOutput*)in , 1);
                        print_checksum_result(nsid , "output" , (struct VdspInputOutput*)out , 1);
                }
		g_FdInfo[fd].working++;
		gWorking++;
		gLock.unlock();

		ret = cs->sendXrpCommand(gfakeBinder , nsid , input , output , buffers,
									bufnum , priority);
		gLock.lock();
		gWorking --;
		g_FdInfo[fd].working --;
		gLock.unlock();
		native_handle_delete(handle_input);
		native_handle_delete(handle_output);
		native_handle_delete(handle_buffers);

		result = static_cast <enum sprd_vdsp_result> (ret);
		if(0 == result) {
			ALOGD("func:%s  ok fd:%d result:%d\n" , __func__ , fd , result);
			return SPRD_VDSP_RESULT_SUCCESS;
		} else {
			ALOGE("func:%s  err fd:%d result:%d\n" , __func__ , fd , result);
			return SPRD_VDSP_RESULT_FAIL;
		}
	}
__err_return_unlock:
	gLock.unlock();
	return (enum sprd_vdsp_result)ret;
}

enum sprd_vdsp_result sprd_vdsp_loadlibrary_serv(void *handle , const char *libname , struct sprd_vdsp_client_inout *buffer)
{
	sp<IVdspService> cs = NULL;
	enum sprd_vdsp_result result;
	int32_t ret;
	uint32_t generation;
	int32_t fd;
	hidl_handle buff;
	native_handle_t *handle_buffer = NULL;
	struct vdsp_handle *hnd = (struct vdsp_handle*) handle;
	cs = getVdspService(0);
	if((cs == NULL) || (NULL == handle))
	{
		/*fd is abnormal value*/
		ALOGE("func:%s err param cs:%p , handle:%p\n" , __func__ , cs.get() , handle);
		return SPRD_VDSP_RESULT_FAIL;
	}
	fd = hnd->fd;
	generation = hnd->generation;
	//if(cs != NULL)
	{
		gLock.lock();
		if(0 != Check_GenrationValid(generation))
		{
			ALOGE("func:%s Check_GenrationValid failed generation:%d , gGeneration:%d\n" , __func__ ,
				generation , gGeneration);
			gLock.unlock();
			return SPRD_VDSP_RESULT_OLD_GENERATION;
		}
		if(0 != Check_FdValid(fd))
                {
			ALOGE("func:%s Check_FdValid failed fd:%d\n" , __func__ , fd);
			gLock.unlock();
                        return SPRD_VDSP_RESULT_FAIL;
                }
		if(gOpenedCount == 0)
		{
			gLock.unlock();
			ALOGE("func:%s gOpenedCount 0 failed fd:%d\n" , __func__ , fd);
			return SPRD_VDSP_RESULT_FAIL;
		}
		ret = create_hidl_params(&handle_buffer , buffer , 1);
                if(ret != 0) {
			gLock.unlock();
			ALOGE("func:%s create_hidl_params failed\n" , __func__);
                        return SPRD_VDSP_RESULT_FAIL;
                }
                buff = handle_buffer;
		gWorking++;
		g_FdInfo[fd].working++;
		gLock.unlock();
		ALOGD("func:%s gfakeBinder:%p" , __func__  , gfakeBinder.get());
		ret = cs->loadXrpLibrary(gfakeBinder , libname , buff);
		result = (enum sprd_vdsp_result) ret;
		native_handle_delete(handle_buffer);
		gLock.lock();
		gWorking --;
		g_FdInfo[fd].working --;
		gLock.unlock();
		if( 0 == result) {
			ALOGD("func:%s ok fd:%d result:%d\n" , __func__ , fd , result);
			return SPRD_VDSP_RESULT_SUCCESS;
		} else {
			ALOGE("func:%s err fd:%d result:%d\n" , __func__ , fd , result);
			return SPRD_VDSP_RESULT_FAIL;
		}
	}
}

enum sprd_vdsp_result sprd_vdsp_unloadlibrary_serv(void *handle , const char *libname)
{
	sp<IVdspService> cs = NULL;
	enum sprd_vdsp_result result;
	int32_t ret;
	uint32_t generation;
	int32_t fd;
	struct vdsp_handle *hnd = (struct vdsp_handle*) handle;
	cs = getVdspService(0);
	if((cs == NULL) || (NULL == handle))
	{
		/*fd is abnormal value*/
		ALOGE("func:%s err param cs:%p , handle:%p\n" , __func__ , cs.get() , handle);
		return SPRD_VDSP_RESULT_FAIL;
	}
	generation = hnd->generation;
	fd = hnd->fd;
	//if(cs != NULL)
	{
		gLock.lock();
		if(0 != Check_GenrationValid(generation))
		{
			ALOGE("func:%s Check_GenrationValid failed generation:%d , gGeneration:%d\n" , __func__ ,
				generation , gGeneration);
			gLock.unlock();
			return SPRD_VDSP_RESULT_OLD_GENERATION;
		}
		if(0 != Check_FdValid(fd))
                {
			ALOGE("func:%s Check_FdValid failed fd:%d\n" , __func__ , fd);
			gLock.unlock();
                        return SPRD_VDSP_RESULT_FAIL;
                }
		if(gOpenedCount == 0)
		{
			gLock.unlock();
			ALOGE("func:%s gOpenedCount 0 failed fd:%d\n" , __func__ , fd);
			return SPRD_VDSP_RESULT_FAIL;
		}
		gWorking ++;
		g_FdInfo[fd].working ++;
		gLock.unlock();
		ret = cs->unloadXrpLibrary(gfakeBinder , libname);
		result = (enum sprd_vdsp_result) ret;
		gLock.lock();
		gWorking --;
		g_FdInfo[fd].working --;
		gLock.unlock();
		if(0 == result) {
			ALOGD("func:%s  fd:%d result:%d , gDeathRecipient:%p\n" , __func__ , fd , result ,gDeathRecipient.get());
			return SPRD_VDSP_RESULT_SUCCESS;
		} else {
			ALOGE("func:%s err fd:%d result:%d , gDeathRecipient:%p\n" , __func__ , fd , result ,gDeathRecipient.get());
			return SPRD_VDSP_RESULT_FAIL;
		}
	}
}
enum sprd_vdsp_result sprd_vdsp_power_hint_serv(void *handle , enum sprd_vdsp_power_level level , enum sprd_vdsp_powerhint_acquire_release acquire_release)
{
	sp<IVdspService> cs = NULL;
        enum sprd_vdsp_result result;
	int32_t ret;
        uint32_t generation;
        int32_t fd;
        struct vdsp_handle *hnd = (struct vdsp_handle*) handle;
        cs = getVdspService(0);
        if((cs == NULL) || (NULL == handle))
        {
                /*fd is abnormal value*/
                ALOGE("func:%s err param cs:%p , handle:%p\n" , __func__ , cs.get() , handle);
                return SPRD_VDSP_RESULT_FAIL;
        }
        generation = hnd->generation;
        fd = hnd->fd;
        //if(cs != NULL)
        {
                gLock.lock();
                if(0 != Check_GenrationValid(generation))
                {
                        ALOGE("func:%s Check_GenrationValid failed generation:%d , gGeneration:%d\n" , __func__ ,
                                generation , gGeneration);
                        gLock.unlock();
                        return SPRD_VDSP_RESULT_OLD_GENERATION;
                }
                if(0 != Check_FdValid(fd))
                {
                        ALOGE("func:%s Check_FdValid failed fd:%d\n" , __func__ , fd);
                        gLock.unlock();
                        return SPRD_VDSP_RESULT_FAIL;
                }
                if(gOpenedCount == 0)
                {
                        gLock.unlock();
                        ALOGE("func:%s gOpenedCount 0 failed fd:%d\n" , __func__ , fd);
                        return SPRD_VDSP_RESULT_FAIL;
                }
                gWorking ++;
		g_FdInfo[fd].working ++;
                gLock.unlock();
		ALOGD("func:%s , before powerHint level:%d, acquire_release:%d" , __func__ , level , acquire_release);
		ret = cs->powerHint(gfakeBinder , level , (uint32_t)acquire_release);
                result = (enum sprd_vdsp_result) ret;
                gLock.lock();
                gWorking --;
                g_FdInfo[fd].working --;
                gLock.unlock();
                if(0 == result) {
			ALOGD("func:%s ok fd:%d result:%d , gDeathRecipient:%p\n" , __func__ , fd , result ,gDeathRecipient.get());
                        return SPRD_VDSP_RESULT_SUCCESS;
                } else {
			ALOGE("func:%s err fd:%d result:%d , gDeathRecipient:%p\n" , __func__ , fd , result ,gDeathRecipient.get());
                        return SPRD_VDSP_RESULT_FAIL;
                }
        }
}
