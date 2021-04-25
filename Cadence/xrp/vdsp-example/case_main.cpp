#include <utils/Log.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include "case_common.h"
#include "case.h"


#ifdef VDSP_LOG
#undef VDSP_LOG
#endif
//#define VDSP_LOG(fmt, ...)	printf("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)	//ALOGD
#define VDSP_LOG	printf


enum case_tag{
	TAG_NORMAL_ALL,
	TAG_NORMAL,	/* standard case process*/
	TAG_THREAD,	/* thread case process*/
	TAG_CUSTOMIZE, /*diy*/
	TAG_ABNORMAL,
	TAG_FACE_ID,
};

/* test case struction*/
struct test_case_struct{
	int id;
	enum case_tag tag;
	char const *name;
	int (*func)(void*);
};

/*
* case define
* The case function requires to return 0 for normal and 1 for abnormal
*/
static struct test_case_struct test_case[] = {
	{0, TAG_NORMAL_ALL, "run all normal test case", NULL},
	{1, TAG_NORMAL, "test vdsp only open close", test_only_open_close},
	{2, TAG_NORMAL, "test vdsp open load send unload close", test_load_send_unload},
	{3, TAG_NORMAL, "test vdsp standard process + power hint", test_vdsp_and_powerhint},
	{4, TAG_NORMAL, "test vdsp interface by server process", test_interface_by_server},
	{5, TAG_ABNORMAL, "test vdsp unsupport nsid, fail", test_send_unsupport_nsid},
	{6, TAG_THREAD, "test vdsp two thread load two different lib", test_2file_load_diff_2lib},
	{7, TAG_THREAD, "test vdsp two thread load two same lib", test_2file_load_same_2lib},
	{8, TAG_CUSTOMIZE, "test vdsp customize settings", test_vdsp_customize},
	{9, TAG_NORMAL, "test xtensa add function", test_xtensa_add},
	{10, TAG_FACE_ID, "test face id l5pro", test_faceid_l5pro},
	{11, TAG_FACE_ID, "test face id n6pro", test_faceid_n6pro},
};

static int show_test_case(void)
{
	VDSP_LOG("Case List: (ID / Name)\n");
	for(int i = 0; i < sizeof(test_case) / sizeof(struct test_case_struct); i++) {
		VDSP_LOG("	%d : %s\n", i, test_case[i].name);
	}

	/*demo show*/
	VDSP_LOG("\nCustomize Cmd:dspclient case_id(1) dev_id(2) work_type(3) int_type(4)\n"
		"libname(5) cycle(6) open.flag(7) open.cnt(8)\n"
		"load.flag(9) load.cnt(10) send.flag(11) send.cnt(12)\n"
		"unload.flag(13) unload.cnt(14) close.flag(15) close.cnt(16)\n"
		"powerhint:flag(17) acq_cnt(18) rel_cnt(19) level(20)\n");
	VDSP_LOG("Demo:dspclient 8 0 0 0 test_lib 10 1 1 1 1 1 2 1 1 1 1 0 1 1 2\n");

	return 0;
}
static int run_tag_case(enum case_tag tag)
{
	void *para = NULL;
	int ret = 0;
	int total_ret = 0;

	for(int i = 1; i < sizeof(test_case) / sizeof(struct test_case_struct); i++) {
		if(test_case[i].tag == tag) {
			ret = test_case[i].func(para);
			VDSP_LOG("case id(%d):%s\n", i, CHECK_RESULT(ret));
			total_ret += ret;
		}
	}
	VDSP_LOG("All tag(%d) case result:%s\n", tag, CHECK_RESULT(total_ret));
	return total_ret;
}

int main(__unused int argc, char *argv[])
{
	struct arg_stru arg;
	void *para = NULL;
	int case_num = sizeof(test_case) / sizeof(struct test_case_struct);
	int case_id;
	int ret = 0;

	arg.argc = argc;
	for (int i = 0; i < argc; i++) {
		arg.argv[i] = argv[i];
	}
	para = &arg;
	if (argc < 2) {
		VDSP_LOG("please input dspclient **\n");
		show_test_case();
		return 0;
	}
	case_id = atoi(argv[1]);
	VDSP_LOG("test caseid :%d\n", case_id);
	if (case_id >= case_num) {
		VDSP_LOG("caseid exceed range\n");
		return 0;
	}

	if (case_id == 0) {
		ret = run_tag_case(TAG_NORMAL);
	}
	else {
		ret = test_case[case_id].func(para);
	}

	VDSP_LOG("case id: %d (%s) :%s\n", case_id, test_case[case_id].name,
		CHECK_RESULT(ret));

	return 0;
}



