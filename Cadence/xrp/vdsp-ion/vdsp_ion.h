
#ifndef __VDSP_ION_H_
#define __VDSP_ION_H_

#include "vdsp_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

void* vdsp_alloc_ionmem(uint32_t size, uint8_t iscache, int32_t* fd, void** viraddr);
void* vdsp_alloc_ionmem2(uint32_t size, uint8_t iscache, int32_t* fd, void** viraddr, unsigned long* phyaddr);
enum sprd_vdsp_status vdsp_free_ionmem(void* handle);
enum sprd_vdsp_status vdsp_flush_ionmem(void* handle, void* vaddr, void* paddr, uint32_t size);
enum sprd_vdsp_status vdsp_invalid_ionmem(void* handle);


#ifdef __cplusplus
}
#endif

#endif


