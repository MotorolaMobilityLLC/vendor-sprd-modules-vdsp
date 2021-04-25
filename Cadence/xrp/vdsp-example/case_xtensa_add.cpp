#include <utils/Log.h>
#include <sprd_ion.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <utils/Timers.h>
#include "vdsp_interface.h"
#include <stdlib.h>
#include <errno.h>
#include "case_common.h"

#ifdef VDSP_LOG
#undef VDSP_LOG
#endif
#define VDSP_LOG(fmt, ...)	printf("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

int test_xtensa_add(__unused void* test)
{
	/* file */
	FILE *fp;
	char filename[256];

	/* mem handle*/
	void *inputhandle = NULL;
	void *outputhandle = NULL;
	void *libhandle = NULL;
	void *gphandle = NULL;
	struct sprd_vdsp_client_inout in;
	struct sprd_vdsp_client_inout out;
	struct sprd_vdsp_client_inout group;
	struct sprd_vdsp_client_inout lib;
	uint32_t size = 81920;

	/* device */
	void *devhandle = NULL;
	struct vdsp_open_param open_param;

	char nsid[32];
	char *golden;
	int ret;

	open_param.idx = 0;
	open_param.work_type = SPRD_VDSP_WORK_NORMAL;
	open_param.int_type = SPRD_VDSP_INTERFACE_DIRECTLY;

	/* mem allocate */
	in.size = size;
	out.size = size;
	lib.size = size;
	group.size = size;

	golden = (char*)malloc(size);
	memset(golden, 0x22, size);

	inputhandle = sprd_alloc_ionmem(in.size, 0, &in.fd, &in.viraddr);
	if (inputhandle == NULL) {
		ret = VDSP_FAIL;
		goto in_error;
	}
	outputhandle = sprd_alloc_ionmem(out.size, 0, &out.fd, &out.viraddr);
	if (outputhandle == NULL) {
		ret = VDSP_FAIL;
		goto out_error;
	}
	gphandle = sprd_alloc_ionmem(group.size, 0, &group.fd, &group.viraddr);
	if (gphandle == NULL) {
		ret = VDSP_FAIL;
		goto group_error;
	}
	libhandle = sprd_alloc_ionmem(lib.size, 0, &lib.fd, &lib.viraddr);
	if (libhandle == NULL) {
		ret = VDSP_FAIL;
		goto lib_error;
	}
	VDSP_LOG("alloc mem OK\n");

	memset(nsid, 0, 32);
	strcpy(nsid, "xtensa_add");

	sprintf(filename, "/vendor/firmware/%s.bin", nsid);
	fp = fopen(filename, "rb");
	if (fp) {
		ret = fread(lib.viraddr, 1, lib.size, fp);
		fclose(fp);
	}
	else {
		ret = VDSP_FAIL;
		VDSP_LOG("open file %s.bin fail\n", nsid);
		goto file_error;
	}
	VDSP_LOG("in %d %p, out %d %p, lib %d %p, group %d\n",
		in.fd, in.viraddr, out.fd, out.viraddr, lib.fd, lib.viraddr, group.fd);
	memset(in.viraddr, 0x10, size);
	memset(out.viraddr, 0x12, size);
	memset(group.viraddr, 0x00, size);

	/* cmd process*/
	ret = sprd_cavdsp_open_device_compat(&open_param, &devhandle);
	VDSP_LOG("open device, ret:%d\n", ret);
	ret = sprd_cavdsp_loadlibrary_compat(devhandle, nsid, &lib);
	VDSP_LOG("load library, ret:%d\n", ret);
	ret = sprd_cavdsp_send_cmd_compat(devhandle, nsid, &in, &out, &group, 1, 1);
	VDSP_LOG("send cmd, ret:%d\n", ret);
	ret = sprd_cavdsp_unloadlibrary_compat(devhandle, nsid);
	VDSP_LOG("unload library, ret:%d\n", ret);
	ret = sprd_cavdsp_close_device_compat(devhandle);
	VDSP_LOG("close device, ret:%d\n", ret);

	/* check result */
	if(memcmp(group.viraddr, golden, size) == 0) {
		ret = VDSP_PASS;
		VDSP_LOG("test ok, dst is:%x, %x\n",
		((char *)group.viraddr)[0], ((char *)group.viraddr)[size - 1]);
	}
	else {
		ret = VDSP_FAIL;
		VDSP_LOG("test failed, %x, %x\n",
		((char *)group.viraddr)[0], ((char *)group.viraddr)[size - 1]);
	}

file_error:
	sprd_free_ionmem(libhandle);

lib_error:
	sprd_free_ionmem(gphandle);

group_error:
	sprd_free_ionmem(outputhandle);

out_error:
	sprd_free_ionmem(inputhandle);

in_error:
	free(golden);

	return ret;
}


