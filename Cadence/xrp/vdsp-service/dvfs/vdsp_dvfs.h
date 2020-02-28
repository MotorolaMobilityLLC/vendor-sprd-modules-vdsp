#ifndef _SPRD_VDSP_DVFS_H_
#define _SPRD_VDSP_DVFS_H_
#include "vdsp_interface.h"

#ifdef __cplusplus
extern "C" {
#endif
int32_t set_powerhint_flag(void* device , enum sprd_vdsp_power_level level , uint32_t permanent);
#ifdef __cplusplus
}
#endif

#endif

