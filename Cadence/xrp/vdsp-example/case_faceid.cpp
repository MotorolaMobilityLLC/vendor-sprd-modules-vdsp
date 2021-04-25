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

int test_faceid_l5pro(__unused void* test)
{
	/* file */
	FILE *fp;
	char filename[256];

	/* mem handle*/
	void *inputhandle;
	void *outputhandle;
	void *imagehandle;
	struct sprd_vdsp_client_inout in;
	struct sprd_vdsp_client_inout out;
	struct sprd_vdsp_client_inout image;
	unsigned long inphyaddr;

	/* face id realted*/
	FACEID_INFO *face_info;
	FACEID_IN *faceid_in;
	uint32_t w = 960, h = 720;

	/* device*/
	void *devhandle;
	struct vdsp_open_param open_param;

	int ret = 0;
	int64_t start_time, end_time, duration;

	open_param.idx = 0;
	open_param.work_type = SPRD_VDSP_WORK_FACEID;
	open_param.int_type = SPRD_VDSP_INTERFACE_DIRECTLY;

	/*camera simulation*/
	image.size = w * h * 3 / 2;
	imagehandle = sprd_alloc_ionmem2(image.size, 0, &image.fd, &image.viraddr, &inphyaddr);
	image.phy_addr = inphyaddr;

	sprintf(filename, "/vendor/bin/test.yuv");
	fp = fopen(filename, "rb");
	if (fp) {
		ret = fread(image.viraddr, 1, image.size, fp);
		printf("read firmware size:%d\n", ret);
		fclose(fp);
	}
	else {
		printf("fopen test.yuv failed\n");
		return VDSP_FAIL;
	}

	/*input ion*/
	in.size = sizeof(FACEID_IN);
	inputhandle = sprd_alloc_ionmem(4096, 0, &in.fd, &in.viraddr);
	if (inputhandle != NULL) {
		memset(in.viraddr, 0x00, in.size);
	}

	faceid_in = (FACEID_IN*)in.viraddr;
	faceid_in->height = h;
	faceid_in->width = w;
	faceid_in->workstage = 0;/*enroll*/
	faceid_in->framecount = 0;
	faceid_in->liveness = 1;/*liveness*/
	faceid_in->phyaddr = image.phy_addr;
	faceid_in->help_info[0] = 0xFF;
	faceid_in->l_ir_phyaddr = 0x12;
	faceid_in->r_ir_phyaddr = 0x34;
	faceid_in->bgr_phyaddr = 0x56;
	faceid_in->otp_phyaddr = 0x78;

	/*out ion*/
	out.size = sizeof(FACEID_INFO);
	outputhandle = sprd_alloc_ionmem(out.size, 0, &out.fd, &out.viraddr);
	out.phy_addr = 0;

	if (outputhandle != NULL) {
		memset(out.viraddr, 0x00, out.size);
	}

	start_time = systemTime(CLOCK_MONOTONIC);
	ret = sprd_cavdsp_open_device_compat(&open_param, &devhandle);
	end_time = systemTime(CLOCK_MONOTONIC);
	duration = (int)((end_time - start_time) / 1000000);
	printf("open take %" PRId64 " ms\n", duration);

	if (SPRD_VDSP_RESULT_SUCCESS != ret) {
		printf("open device fail!!\n");
		sprd_free_ionmem(imagehandle);
		sprd_free_ionmem(inputhandle);
		sprd_free_ionmem(outputhandle);
		return VDSP_FAIL;
	}
	start_time = systemTime(CLOCK_MONOTONIC);

	ret = sprd_cavdsp_send_cmd_compat(devhandle, "faceid_fw", &in, &out, NULL, 0, 1);
	if (SPRD_VDSP_RESULT_SUCCESS != ret) {
		printf("xrp_run_faceid_command failed\n");
		ret = VDSP_FAIL;
	}
	else {
		void* out_var = (char*)in.viraddr + 2048;
		face_info = (FACEID_INFO *)out_var;//out.viraddr;
		printf("vdsp result %d,out addr %X\n",
			face_info->ret, face_info->facepoint_addr);
		printf("x %d y %d w %d h %d yaw %d pitch %d\n",
			face_info->x, face_info->y, face_info->width, face_info->height,
			face_info->yawAngle, face_info->pitchAngle);
		ret = VDSP_PASS;
	}

	end_time = systemTime(CLOCK_MONOTONIC);
	duration = (int)((end_time - start_time) / 1000000);
	printf("run 1 time take %" PRId64 " ms\n", duration);
	sprd_cavdsp_close_device_compat(devhandle);
	sprd_free_ionmem(imagehandle);
	sprd_free_ionmem(inputhandle);
	sprd_free_ionmem(outputhandle);

	return ret;
}

int test_faceid_n6pro(__unused void* test)
{
	/* file */
	FILE *fp;
	char filename[256];

	/* mem handle*/
	void *inputhandle;
	void *outputhandle;
	void *imagehandle;
	struct sprd_vdsp_client_inout in;
	struct sprd_vdsp_client_inout out;
	struct sprd_vdsp_client_inout image;
	unsigned long inphyaddr;

	/* face id realted*/
	FACEID_INFO *face_info;
	FACEID_FACE_TAG faceTag;
	FACEID_IN *faceid_in;
	uint32_t w = 960, h = 720;

	/* device*/
	void *devhandle;
	struct vdsp_open_param open_param;

	int ret = 0;
	int64_t start_time, end_time, duration;

	open_param.idx = 0;
	open_param.work_type = SPRD_VDSP_WORK_FACEID;
	open_param.int_type = SPRD_VDSP_INTERFACE_DIRECTLY;

	faceTag.face.rect[0] = 347;
	faceTag.face.rect[1] = 259;
	faceTag.face.rect[2] = 347 + 256;
	faceTag.face.rect[3] = 259 + 256;
	faceTag.angle = 90;
	faceTag.pose = -3;
	faceTag.face_num = 1;

	/*camera simulation*/
	image.size = w * h * 3 / 2;
	imagehandle = sprd_alloc_ionmem2(image.size, 0, &image.fd, &image.viraddr, &inphyaddr);
	image.phy_addr = inphyaddr;

	sprintf(filename, "/vendor/bin/test.yuv");
	fp = fopen(filename, "rb");
	if (fp) {
		ret = fread(image.viraddr, 1, image.size, fp);
		printf("read firmware size:%d\n", ret);
		fclose(fp);
	}
	else {
		printf("fopen test.yuv failed\n");
		return VDSP_FAIL;
	}

	/*input ion*/
	in.size = sizeof(FACEID_IN);
	inputhandle = sprd_alloc_ionmem(4096, 0, &in.fd, &in.viraddr);
	if (inputhandle != NULL) {
		memset(in.viraddr, 0x00, in.size);
	}

	faceid_in = (FACEID_IN*)in.viraddr;
	faceid_in->height = h;
	faceid_in->width = w;
	faceid_in->workstage = 0;/*enroll*/
	faceid_in->framecount = 0;
	faceid_in->liveness = 1;/*liveness*/
	faceid_in->phyaddr = image.fd;
	faceid_in->help_info[0] = 1;
	faceid_in->help_info[1] = 1;
	faceid_in->help_info[2] = 50;
	faceid_in->help_info[3] = 1;
	faceid_in->help_info[4] = 8;
	faceid_in->help_info[5] = -8;
	memcpy(&faceid_in->fd_info, &faceTag, sizeof(FACEID_FACE_TAG));
	faceid_in->l_ir_phyaddr = 0x12;
	faceid_in->r_ir_phyaddr = 0x34;
	faceid_in->bgr_phyaddr = 0x56;
	faceid_in->otp_phyaddr = 0x78;

	/*out ion*/
	out.size = sizeof(FACEID_INFO);
	outputhandle = sprd_alloc_ionmem(out.size, 0, &out.fd, &out.viraddr);
	out.phy_addr = 0;

	if (outputhandle != NULL) {
		memset(out.viraddr, 0x00, out.size);
	}

	start_time = systemTime(CLOCK_MONOTONIC);
	ret = sprd_cavdsp_open_device_compat(&open_param, &devhandle);
	end_time = systemTime(CLOCK_MONOTONIC);
	duration = (int)((end_time - start_time) / 1000000);
	printf("open take %" PRId64 " ms\n", duration);

	if (SPRD_VDSP_RESULT_SUCCESS != ret) {
		printf("open device fail!!\n");
		sprd_free_ionmem(imagehandle);
		sprd_free_ionmem(inputhandle);
		sprd_free_ionmem(outputhandle);
		return VDSP_FAIL;
	}
	start_time = systemTime(CLOCK_MONOTONIC);
	ret = sprd_cavdsp_send_cmd_compat(devhandle, "faceid_fw", &in, &image, NULL, 0, 1);
	if (SPRD_VDSP_RESULT_SUCCESS != ret) {
		printf("xrp_run_faceid_command failed\n");
		ret = VDSP_FAIL;
	}
	else {
		void* out_var = (char*)in.viraddr + 2048;
		face_info = (FACEID_INFO *)out_var;//out.viraddr;
		printf("vdsp result %d,out addr %X\n",
			face_info->ret, face_info->facepoint_addr);
		printf("x %d y %d w %d h %d yaw %d pitch %d\n",
			face_info->x, face_info->y, face_info->width, face_info->height,
			face_info->yawAngle, face_info->pitchAngle);
		ret = VDSP_PASS;
	}

	end_time = systemTime(CLOCK_MONOTONIC);
	duration = (int)((end_time - start_time) / 1000000);
	printf("run 1 time take %" PRId64 " ms\n", duration);
	sprd_cavdsp_close_device_compat(devhandle);
	sprd_free_ionmem(imagehandle);
	sprd_free_ionmem(inputhandle);
	sprd_free_ionmem(outputhandle);

	return ret;
}




