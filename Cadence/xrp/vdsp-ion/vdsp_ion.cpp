#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <log/log.h>
#include "sprd_ion.h"
#include "VdspIon.h"
#include "vdsp_ion.h"
#include "vdsp_interface_internal.h"

using namespace android;


void* vdsp_alloc_ionmem(uint32_t size , uint8_t iscache , int32_t* fd , void** viraddr)
{
	VdspIon *pHeapIon = NULL;
	*fd = -1;
	*viraddr = NULL;
	if(iscache != 0)
	{
		//USE ION_HEAP_ID_MASK_MM temp , may be modify to SYSTEM
		pHeapIon = new VdspIon("/dev/ion" , size , 0 ,  (1 << 31) | ION_HEAP_ID_MASK_SYSTEM | ION_FLAG_NO_CLEAR);
	}
	else
	{
		//USE ION_HEAP_ID_MASK_MM temp , may be modify to SYSTEM
		pHeapIon =  new VdspIon("/dev/ion", size, VdspIon::NO_CACHING , ION_HEAP_ID_MASK_SYSTEM | ION_FLAG_NO_CLEAR);
	}
	if(pHeapIon != NULL)
	{
		*viraddr = pHeapIon->getBase();
		*fd = pHeapIon->getHeapID();
	}
	ALOGD("func:%s pHeapIon:%p" , __func__ , pHeapIon);
	return pHeapIon;
}

void* vdsp_alloc_ionmem2(uint32_t size , uint8_t iscache , int32_t* fd , void** viraddr, unsigned long* phyaddr)
{
	VdspIon *pHeapIon = NULL;
	*fd = -1;
	*viraddr = NULL;
	size_t size_ret = 0;

	if(iscache != 0)
	{
		//USE ION_HEAP_ID_MASK_MM temp , may be modify to SYSTEM
		pHeapIon = new VdspIon("/dev/ion" , size , 0 ,  (1 << 31) | ION_HEAP_ID_MASK_CAM | ION_FLAG_NO_CLEAR);
	}
	else
	{
		//USE ION_HEAP_ID_MASK_MM temp , may be modify to SYSTEM
		pHeapIon =  new VdspIon("/dev/ion", size, VdspIon::NO_CACHING , ION_HEAP_ID_MASK_CAM | ION_FLAG_NO_CLEAR);
	}
	if(pHeapIon != NULL)
	{
		*viraddr = pHeapIon->getBase();
		*fd = pHeapIon->getHeapID();
		pHeapIon->get_phy_addr_from_ion(phyaddr ,&size_ret);
	}
	return pHeapIon;
}
enum sprd_vdsp_status vdsp_flush_ionmem(void* handle , void* vaddr, void* paddr, uint32_t size)
{
	VdspIon* pHeapIon = (VdspIon*) handle;
	if(pHeapIon != NULL)
	{
		int32_t fd = pHeapIon->getHeapID();
		int32_t ret = pHeapIon->Flush_ion_buffer(fd , vaddr , paddr, size);
		if(0 == ret)
		{
			return SPRD_XRP_STATUS_SUCCESS;
		}
	}
	return SPRD_XRP_STATUS_FAILURE;
}
enum sprd_vdsp_status vdsp_invalid_ionmem(void* handle)
{
        VdspIon* pHeapIon = (VdspIon*) handle;
        if(pHeapIon != NULL)
        {
                int32_t fd = pHeapIon->getHeapID();
		int32_t ret = pHeapIon->Invalid_ion_buffer(fd);
		if(ret == 0)
			return SPRD_XRP_STATUS_SUCCESS;
        }
        return SPRD_XRP_STATUS_FAILURE;
}
enum sprd_vdsp_status vdsp_free_ionmem(void* handle)
{
	enum sprd_vdsp_status ret;
	VdspIon* pHeapIon = (VdspIon*) handle;
	ret = SPRD_XRP_STATUS_SUCCESS;
	if(pHeapIon != NULL)
	{
		delete pHeapIon;
	}
	else
	{
		ALOGE("func:%s , input handle is NULL" , __func__);
		ret = SPRD_XRP_STATUS_FAILURE;
	}
	return ret;
}
