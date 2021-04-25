#ifndef CASE_COMMON_H
#define CASE_COMMON_H
#include <stdio.h>

#define VDSP_PASS	(0)	/*same as SPRD_VDSP_RESULT_SUCCESS*/
#define VDSP_FAIL	(1)	/*same as SPRD_VDSP_RESULT_FAIL*/
#define CHECK_RESULT(ret) (((ret) == 0)?"PASS":"FAIL")

struct arg_stru{
	int argc;
	char *argv[100];
};

/* face id related */
typedef struct
{
	int x, y, width, height;
	int yawAngle, pitchAngle;
	int ret;
	unsigned int facepoint_addr;
}FACEID_INFO;

/*
 * Help info from hal.
 * The first part: one int32 is CONTROL_AE_STATE;
 * The second part: one int32 is ae info.
 * 		Bit 31-16 is the bv value,which is a signed value.
 * 		Bit 10-1 is the probability of backlight, range[0-1000].
 *  	Bit 0 is whether ae is stable, range[0, 1].
 * The third part: 256 int32 is hist_statistics.
 * The fouth part: one int32 is the phone orientation info.
 */
typedef struct
{
	const int32_t *helpInfo;
	int32_t count;
} FACEID_HELPINFO;

typedef struct
{
	uint32_t width, height;
	uint32_t phyaddr;		/*image phyaddr*/
	uint32_t workstage;		/*enroll:0,auth:1*/
	uint32_t framecount;
	uint32_t liveness;		/*0:off 1:faceid_single 2:faceid_3D 3:pay_3D*/
	int32_t  help_info[263];/*AE BV*/
	uint32_t fd_info[15];	/*Face Info from Hal*/
	uint32_t l_ir_phyaddr;	/*Left IR phyaddr*/
	uint32_t r_ir_phyaddr;	/*Right IR phyaddr*/
	uint32_t bgr_phyaddr;	/*bgr phyaddr*/
	uint32_t otp_phyaddr;	/*otp phyaddr*/
}FACEID_IN;

typedef struct camera_face {
	int32_t rect[4];
	int32_t score;
	int32_t id;
	int32_t left_eye[2];
	int32_t right_eye[2];
	int32_t mouth[2];
}camera_face_t;

typedef struct
{
	camera_face_t face;
	int angle;
	int pose;
	uint8_t face_num;
}FACEID_FACE_TAG;

#endif

