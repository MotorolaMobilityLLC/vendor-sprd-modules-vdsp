#include <pthread.h>
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
//#define VDSP_LOG	printf

#define T_ENABLE	(1)
#define T_DISABLE	(0)

/*operation parameters*/
struct op_para
{
	int flag;	/*enable/disable*/
	int cnt;	/*count*/
};

/*power hint parameters*/
struct ph_para
{
	int flag;	 /*enable/disable*/
	int acq_cnt; /*power hint acquire count*/
	int rel_cnt; /*power hint release count;*/
	enum sprd_vdsp_power_level level; //ph_level
};

/*test case parameters*/
struct test_para
{
	/*open device related*/
	int dev_id;
	enum sprd_vdsp_worktype work_type;
	enum sprd_vdsp_interface_type int_type;
	int result; /*run result*/
	/*run cmd related*/
	char const *libname;
	int read_flag;  /*whether need read firmware*/
	int cycle;  /*run cycle*/
	struct op_para open;
	struct op_para close;
	struct op_para load;
	struct op_para unload;
	struct op_para send;
	struct ph_para powerhint;
};

static void reset_test_para(struct test_para *para)
{
	VDSP_LOG("reset test para\n");
	/*work/interface type settings.(SPRD_VDSP_INTERFACE_BYSERVER)*/
	para->dev_id = 0;
	para->work_type = SPRD_VDSP_WORK_NORMAL;
	para->int_type = SPRD_VDSP_INTERFACE_DIRECTLY;

	/* record run result*/
	para->result = VDSP_PASS;

	/*run settings*/
	para->read_flag = 1;
	para->cycle = 1;
	para->libname = "test_lib";
	para->open.flag = T_DISABLE;
	para->open.cnt = 1;
	para->close.flag = T_DISABLE;
	para->close.cnt = 1;
	para->load.flag = T_DISABLE;
	para->load.cnt = 1;
	para->unload.flag = T_DISABLE;
	para->unload.cnt = 1;
	para->send.flag = T_DISABLE;
	para->send.cnt = 1;
	para->powerhint.flag = T_DISABLE;
	para->powerhint.acq_cnt = 1;
	para->powerhint.rel_cnt = 1;
	para->powerhint.level = SPRD_VDSP_POWERHINT_LEVEL_2;
}

static void printf_test_para(struct test_para *para)
{
	VDSP_LOG("test para:\n"
	"dev id:%d,work type:%d,interface type:%d,\n"
	"run cycle:%d,libname:%s,\n"
	"open flag:%d/cnt:%d,\n"
	"close flag:%d/cnt:%d,\n"
	"load flag:%d/cnt:%d,\n"
	"unload flag:%d/cnt:%d,\n"
	"send flag:%d/cnt:%d,\n"
	"power hint flag:%d/acq cnt:%d/rel cnt:%d/level:%d\n",
	para->dev_id, para->work_type, para->int_type,
	para->cycle, para->libname,
	para->open.flag, para->open.cnt,
	para->close.flag, para->close.cnt,
	para->load.flag, para->load.cnt,
	para->unload.flag, para->unload.cnt,
	para->send.flag, para->send.cnt,
	para->powerhint.flag, para->powerhint.acq_cnt,
	para->powerhint.rel_cnt, para->powerhint.level);
}

static int check_test_para(struct test_para *para)
{
	if (para->work_type >= SPRD_VDSP_WORK_MAXTYPE ||
		para->int_type >= SPRD_VDSP_INTERFACe_MAX ||
		para->libname == NULL ||
		para->open.cnt < 0 ||
		para->close.cnt < 0 ||
		para->load.cnt < 0 ||
		para->unload.cnt < 0 ||
		para->send.cnt < 0 ||
		para->powerhint.acq_cnt < 0 ||
		para->powerhint.rel_cnt < 0) {
		VDSP_LOG("para error, please check!\n");
		return VDSP_FAIL;
	}
	return VDSP_PASS;
}

static char customize_libname[128] = "";
static int parse_customize_settings(void* test, int argc, char *argv[])
{
	int i;
	struct test_para *para = (struct test_para *)test;

	VDSP_LOG("argc: %d\n", argc);
	for(i = 0; i < argc; i++)
	{
		VDSP_LOG("%d:%s\n", i, argv[i]);
	}
	reset_test_para(para);

	/*parse open device related*/
	if (argc > 2) {
		para->dev_id = atoi(argv[2]);
	}
	if (argc > 3) {
		para->work_type = (enum sprd_vdsp_worktype)atoi(argv[3]);
	}
	if (argc > 4) {
		para->int_type = (enum sprd_vdsp_interface_type)atoi(argv[4]);
	}

	/*parse run cmd related*/
	if (argc > 5) {
		sprintf(customize_libname, "%s", argv[5]);
		para->libname = customize_libname;
	}
	if (argc > 6) {
		para->cycle = atoi(argv[6]);
	}
	if (argc > 8) {
		para->open.flag = atoi(argv[7]);
		para->open.cnt = atoi(argv[8]);
	}
	if (argc > 10) {
		para->load.flag = atoi(argv[9]);
		para->load.cnt = atoi(argv[10]);
	}
	if (argc > 12) {
		para->send.flag = atoi(argv[11]);
		para->send.cnt = atoi(argv[12]);
	}
	if (argc > 14) {
		para->unload.flag = atoi(argv[13]);
		para->unload.cnt = atoi(argv[14]);
	}
	if (argc > 16) {
		para->close.flag = atoi(argv[15]);
		para->close.cnt = atoi(argv[16]);
	}
	if (argc > 20) {
		para->powerhint.flag = atoi(argv[17]);
		para->powerhint.acq_cnt = atoi(argv[18]);
		para->powerhint.rel_cnt = atoi(argv[19]);
		para->powerhint.level = (enum sprd_vdsp_power_level)atoi(argv[20]);
	}
	printf_test_para(para);

	return 0;
}

/* vdsp core function */
static void* vdsp_process(void* test)
{
	/* read firmware para */
	FILE *fp;
	char filename[256];
	/* cmd, ion relate */
	struct sprd_vdsp_client_inout in;
	struct sprd_vdsp_client_inout out;
	void *inputhandle;
	void *outputhandle;
	/* device */
	void *handle = NULL;
	struct vdsp_open_param open_param;
	char ver[128];
	struct test_para *para = (struct test_para *)test;
	uint32_t size = 81920;
	int i, ret;

	/*para check*/
	if (para == NULL) {
		VDSP_LOG("Null test para!\n");
		para->result = VDSP_FAIL;
		return NULL;
	}
	printf_test_para(para);
	if(check_test_para(para) != 0) {
		VDSP_LOG("test para error!\n");
		para->result = VDSP_FAIL;
		return NULL;
	}

	open_param.idx = para->dev_id;
	open_param.work_type = para->work_type;
	open_param.int_type = para->int_type;

	/*ion*/
	in.size = size;
	out.size = size;
	inputhandle = sprd_alloc_ionmem(in.size, 0, &in.fd, &in.viraddr);
	outputhandle = sprd_alloc_ionmem(out.size, 0, &out.fd, &out.viraddr);

	if (inputhandle != NULL) {
		memset(in.viraddr, 0xbb, size);
	}
	if (outputhandle != NULL) {
		memset(out.viraddr, 0xcc, size);
	}
	/*read firmware*/
	if (para->read_flag == 1) {
		sprintf(filename, "/vendor/firmware/%s.bin", para->libname);
		fp = fopen(filename, "rb");
		if (fp) {
			ret = fread(in.viraddr, 1, size, fp);
			VDSP_LOG("read [%s] firmware, size:%d\n", para->libname, size);
			fclose(fp);
		}
		else {
			VDSP_LOG("fopen [%s] firmware failed\n", para->libname);
			para->result = VDSP_FAIL;
			return NULL;
		}
	}
	/*get vdsp version*/
	sprd_cavdsp_get_info(SPRD_VDSP_GET_VERSION, (void*)ver);
	VDSP_LOG("get ver is:%s\n", ver);

	ret = 0;
	while (para->cycle) {
		/*open*/
		if (para->open.flag) {
			for (i = 0; i < para->open.cnt; i++) {
				ret |= sprd_cavdsp_open_device_compat(&open_param, &handle);
				VDSP_LOG("open device, ret:%d\n", ret);
			}
		}
		/*power hint acquire*/
		if (para->powerhint.flag) {
			for (i = 0; i < para->powerhint.acq_cnt; i++) {
				ret |= sprd_cavdsp_power_hint_compat(handle, para->powerhint.level,
					SPRD_VDSP_POWERHINT_ACQUIRE);
				VDSP_LOG("power hint acquire, ret:%d\n", ret);
			}
		}
		/*load*/
		if (para->load.flag) {
			for (i = 0; i < para->load.cnt; i++) {
				ret |= sprd_cavdsp_loadlibrary_compat(handle, para->libname, &in);
				VDSP_LOG("load library, ret:%d\n", ret);
			}
		}
		/*send*/
		if (para->send.flag) {
			for (i = 0; i < para->send.cnt; i++) {
				ret |= sprd_cavdsp_send_cmd_compat(handle, para->libname, &in,
					NULL, NULL, 0, 1);
				VDSP_LOG("send cmd libname:%s ret:%d\n", para->libname, ret);
			}
		}
		/*unload*/
		if (para->unload.flag) {
			for (i = 0; i < para->unload.cnt; i++) {
				ret |= sprd_cavdsp_unloadlibrary_compat(handle, para->libname);
				VDSP_LOG("unload library, ret:%d\n", ret);
			}
		}
		/*powerhint release*/
		if (para->powerhint.flag) {
			for (i = 0; i < para->powerhint.rel_cnt; i++) {
				ret |= sprd_cavdsp_power_hint_compat(handle, para->powerhint.level,
					SPRD_VDSP_POWERHINT_RELEASE);
				VDSP_LOG("power hint release, ret:%d\n", ret);
			}
		}
		/*close*/
		if (para->close.flag) {
			for (i = 0; i < para->close.cnt; i++) {
				ret |= sprd_cavdsp_close_device_compat(handle);
				VDSP_LOG("close device, ret:%d\n", ret);
			}
		}
		para->cycle--;
	}
	sprd_free_ionmem(inputhandle);
	sprd_free_ionmem(outputhandle);

	if (ret != 0) {
		para->result = VDSP_FAIL;
	}
	else {
		para->result = VDSP_PASS;
	}
	VDSP_LOG("run end(%d)\n", para->result);
	return NULL;
}

/* test case */
int test_only_open_close(__unused void *test)
{
	struct test_para para;

	reset_test_para(&para);
	para.open.flag = T_ENABLE;
	para.close.flag = T_ENABLE;
	vdsp_process((void *)&para);

	return para.result;
}

int test_load_send_unload(__unused void *test)
{
	struct test_para para;

	reset_test_para(&para);
	para.open.flag = T_ENABLE;
	para.load.flag = T_ENABLE;
	para.send.flag = T_ENABLE;
	para.unload.flag = T_ENABLE;
	para.close.flag = T_ENABLE;
	vdsp_process((void *)&para);

	return para.result;
}

/*test vdsp default support nsid, */
int test_send_default_nsid(__unused void *test)
{
	struct test_para para;

	reset_test_para(&para);
	para.read_flag = 0;
	para.libname = "testlib";/*vdsp interal nsid, need vdsp support(debug on)*/
	para.open.flag = T_ENABLE;
	para.send.flag = T_ENABLE;
	para.close.flag = T_ENABLE;
	vdsp_process((void *)&para);

	return para.result;
}
/*test vdsp unsupport nsid, */
int test_send_unsupport_nsid(__unused void *test)
{
	struct test_para para;

	reset_test_para(&para);
	para.read_flag = 0;
	para.libname = "testunsupportlib";/*vdsp unload nsid,vdsp not support*/
	para.open.flag = T_ENABLE;
	para.send.flag = T_ENABLE;
	para.close.flag = T_ENABLE;
	vdsp_process((void *)&para);

/* error test, vdsp process fail means case pass*/
	if (para.result == VDSP_FAIL) {
		return VDSP_PASS;
	}
	else {
		return VDSP_FAIL;
	}
}

/*test vdsp load without unload, make Memory overflow*/
int test_load_send_without_unload(__unused void *test)
{
	struct test_para para;

	reset_test_para(&para);
	para.open.flag = T_ENABLE;
	para.load.flag = T_ENABLE;
	para.send.flag = T_ENABLE;
	para.close.flag = T_ENABLE;
	vdsp_process((void *)&para);

	return para.result;
}

/*test vdsp normal process + power hint*/
int test_vdsp_and_powerhint(__unused void *test)
{
	struct test_para para;

	reset_test_para(&para);
	para.open.flag = T_ENABLE;
	para.load.flag = T_ENABLE;
	para.send.flag = T_ENABLE;
	para.unload.flag = T_ENABLE;
	para.close.flag = T_ENABLE;
	para.powerhint.flag = T_ENABLE;
	vdsp_process((void *)&para);

	return para.result;
}

/*test vdsp normal process through by server api*/
int test_interface_by_server(__unused void *test)
{
	struct test_para para;

	reset_test_para(&para);
	para.int_type = SPRD_VDSP_INTERFACE_BYSERVER;
	para.open.flag = T_ENABLE;
	para.load.flag = T_ENABLE;
	para.send.flag = T_ENABLE;
	para.unload.flag = T_ENABLE;
	para.close.flag = T_ENABLE;
	vdsp_process((void *)&para);

	return para.result;
}

/* thread test*/
static pthread_t tha, thb;
static struct test_para tha_para, thb_para;

/*two file descriptors load different libname*/
int test_2file_load_diff_2lib(__unused void *test)
{
	struct test_para *para = &tha_para;
	struct test_para *para2 = &thb_para;

	reset_test_para(para);
	para->libname = "test_lib",
	para->open.flag = T_ENABLE;
	para->load.flag = T_ENABLE;
	para->send.flag = T_ENABLE;
	para->unload.flag = T_ENABLE;
	para->close.flag = T_ENABLE;

	reset_test_para(para2);
	para2->libname = "test_lib1",
	para2->open.flag = T_ENABLE;
	para2->load.flag = T_ENABLE;
	para2->send.flag = T_ENABLE;
	para2->unload.flag = T_ENABLE;
	para2->close.flag = T_ENABLE;

	pthread_create(&tha, NULL, vdsp_process, (void*)para);
	pthread_create(&thb, NULL, vdsp_process, (void*)para2);
	pthread_join(tha, NULL);
	pthread_join(thb, NULL);

	if (para->result == VDSP_PASS && para2->result == VDSP_PASS) {
		return VDSP_PASS;
	}
	else {
		return VDSP_FAIL;
	}
}

/*two file descriptors load same libname, no unload*/
int test_2file_load_same_2lib(__unused void *test)
{
	struct test_para *para = &tha_para;
	struct test_para *para2 = &thb_para;

	reset_test_para(para);
	para->open.flag = T_ENABLE;
	para->load.flag = T_ENABLE;
	para->send.flag = T_ENABLE;
	para->unload.flag = T_ENABLE;
	para->close.flag = T_ENABLE;

	reset_test_para(para2);
	para2->open.flag = T_ENABLE;
	para2->load.flag = T_ENABLE;
	para2->send.flag = T_ENABLE;
	para2->unload.flag = T_ENABLE;
	para2->close.flag = T_ENABLE;

	pthread_create(&tha, NULL, vdsp_process, (void*)para);
	pthread_create(&thb, NULL, vdsp_process, (void*)para2);
	pthread_join(tha, NULL);
	pthread_join(thb, NULL);

	if (para->result == VDSP_PASS && para2->result == VDSP_PASS) {
		return VDSP_PASS;
	}
	else {
		return VDSP_FAIL;
	}
}

int test_vdsp_customize(void *test)
{
	struct arg_stru *arg = (struct arg_stru *) test;
	struct test_para para;

	reset_test_para(&para);
	parse_customize_settings(&para, arg->argc, arg->argv);
	vdsp_process(&para);

	return para.result;
}


