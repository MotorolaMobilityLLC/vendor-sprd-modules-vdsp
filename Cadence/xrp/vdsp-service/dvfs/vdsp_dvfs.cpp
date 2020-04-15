#include <utils/Mutex.h>
#include <utils/List.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <log/log_main.h>
#include "vdsp_dvfs.h"
#include "xrp_host_common.h"
#include "xrp_kernel_defs.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG  "vdsp_dvfs"

#define DVFS_MONITOR_CYCLE_TIME   (1000*1000)
using namespace android;

__attribute__ ((visibility("default"))) int32_t set_powerhint_flag(void *device , enum sprd_vdsp_power_level level , uint32_t acquire_release)
{
	int32_t ret;
	struct xrp_device *dev = (struct xrp_device*) device;
	struct xrp_powerhint_ctrl powerhint_info;
	powerhint_info.level = level;
	powerhint_info.acquire_release = acquire_release;
	ALOGD("fun:%s powerhint level:%d , acquire_release:%d" , __func__ , powerhint_info.level , powerhint_info.acquire_release);
	ret = ioctl(dev->impl.fd, XRP_IOCTL_SET_POWERHINT, &powerhint_info);
        return ret;
}
