/**
 * Copyright (C) 2020 UNISOC Technologies Co.,Ltd.
 */

#include <gtest/gtest.h>
#include "../vdsp-interface/vdsp_interface.h"


namespace{

	struct vdsp_handle g_handle = {0};

	TEST(vdsp_interfacetest, TestSendCmd) {

		enum sprd_vdsp_result ret_result = (enum sprd_vdsp_result)0;
		struct vdsp_handle *handle = &g_handle;
		enum sprd_vdsp_worktype type = SPRD_VDSP_WORK_NORMAL;

		ret_result = sprd_cavdsp_open_device(type, handle);

		EXPECT_EQ(SPRD_VDSP_RESULT_SUCCESS, ret_result);

	}

};
