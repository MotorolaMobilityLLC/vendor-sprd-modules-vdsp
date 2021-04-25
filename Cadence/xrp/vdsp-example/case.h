#ifndef CASE_H_H
#define CASE_H_H
#include <stdio.h>

int test_only_open_close(__unused void* test);
int test_load_send_unload(__unused void* test);
/*need special vdsp debug nsid on*/
int test_send_default_nsid(__unused void *test);
int test_send_unsupport_nsid(__unused void *test);
int test_vdsp_and_powerhint(__unused void* test);
int test_interface_by_server(__unused void* test);
int test_2file_load_diff_2lib(__unused void* test);
int test_2file_load_same_2lib(__unused void* test);
int test_vdsp_customize(void* test);
int test_faceid_l5pro(__unused void* test);
int test_faceid_n6pro(__unused void* test);
int test_xtensa_add(__unused void* test);

#endif

