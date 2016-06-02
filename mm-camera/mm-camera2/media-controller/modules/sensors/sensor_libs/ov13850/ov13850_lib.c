/*============================================================================


  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.


============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"


#define SENSOR_MODEL_NO_SKUF_OV13850 "ov13850"
#define SKUF_OV13850_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_SKUF_OV13850"_"#n".so"


#undef DEBUG_INFO
//#define OV13850_DEBUG
#ifdef OV13850_DEBUG
#include <utils/Log.h>
#define SERR(fmt, args...) \
    ALOGE("%s:%d "fmt"\n", __func__, __LINE__, ##args)
#define DEBUG_INFO(fmt, args...) SERR(fmt, ##args)
#else
#define DEBUG_INFO(fmt, args...) do { } while (0)
#endif
#define RES_0_ENABLE  1
#define RES_1_ENABLE  1
#define RES_2_ENABLE  1


static sensor_lib_t sensor_lib_ptr;


static sensor_output_t sensor_output = {
  .output_format = SENSOR_BAYER,
  .connection_mode = SENSOR_MIPI_CSI,
  .raw_output = SENSOR_10_BIT_DIRECT,
};


static struct msm_sensor_output_reg_addr_t output_reg_addr = {
  .x_output = 0x3808,
  .y_output = 0x380a,
  .line_length_pclk = 0x380c,
  .frame_length_lines = 0x380e,
};


static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x3500,
  .global_gain_addr = 0x350a,
  .vert_offset = 4,
};


static sensor_aec_data_t aec_info = {
  .max_gain = 15.5,
  .max_linecount = 30834,
};


static sensor_lens_info_t default_lens_info = {
  .focal_length = 4.92,
  .pix_size = 1.4,
  .f_number = 2.65,
  .total_f_dist = 1.97,
  .hor_view_angle = 55.4,
  .ver_view_angle = 42.7,
};


#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0xe4,
  .csi_lane_mask = 0xf,
  .csi_if = 1,
  .csid_core = {0},
};
#else
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x1f,
  .csi_if = 1,
  .csid_core = {0},
};
#endif


static struct msm_camera_i2c_reg_array init_reg_array0[] = {
  {0x0103,0x01},
};


static struct msm_camera_i2c_reg_array init_reg_array1[] = {
  {0x0300,0x01},// ; PLL
  {0x0301,0x00},// ; PLL
  {0x0302,0x28},// ; PLL
  {0x0303,0x00},//  ; PLL
  {0x030a,0x00},//  ; PLL
  {0x300f,0x11},//  ; MIPI 10-bit mode
  {0x3010,0x01},//  ; MIPI PHY
  {0x3011,0x76},//  ; MIPI PHY
  {0x3012,0x41},//  ; MIPI 4 lane
  {0x3013,0x12},//  ; MIPI control
  {0x3014,0x11},//  ; MIPI control
  {0x301f,0x03},//
  {0x3106,0x00},//
  {0x3210,0x47},//
  {0x3500,0x00},//  ; exposure HH
  {0x3501,0x67},//  ; exposure H
  {0x3502,0x80},//  ; exposure L
  {0x3506,0x00},//  ; short exposure HH
  {0x3507,0x02},//  ; short exposure H
  {0x3508,0x00},//  ; shour exposure L
  {0x350a,0x00},//  ; gain H
  {0x350b,0x10},//  ; gain L
  {0x350e,0x00},//  ; short gain H
  {0x350f,0x10},//  ; short gain L
  {0x3600,0x40},//  ; analog control
  {0x3601,0xfc},//  ; analog control
  {0x3602,0x02},//  ; analog control
  {0x3603,0x48},//  ; analog control
  {0x3604,0xa5},//  ; analog control
  {0x3605,0x9f},//  ; analog control
  {0x3607,0x00},//  ; analog control
  {0x360a,0x40},//  ; analog control
  {0x360b,0x91},//  ; analog control
  {0x360c,0x49},//  ; analog control
  {0x360f,0x8a},//
  {0x3611,0x10},//  ; PLL2
  {0x3612,0x23},//  ; PLL2
  {0x3613,0x33},//  ; PLL2
  {0x3615,0x08},//
  {0x3641,0x02},//
  {0x3660,0x82},//
  {0x3668,0x54},//
  {0x3669,0x40},//
  {0x3667,0xa0},//
  {0x3702,0x40},//
  {0x3703,0x44},//
  {0x3704,0x2c},//
  {0x3705,0x24},//
  {0x3706,0x50},//
  {0x3707,0x44},//
  {0x3708,0x3c},//
  {0x3709,0x1f},//
  {0x370a,0x26},//
  {0x370b,0x3c},//
  {0x3720,0x66},//
  {0x3722,0x84},//
  {0x3728,0x40},//
  {0x372a,0x00},//
  {0x372f,0x90},//
  {0x3710,0x28},//
  {0x3716,0x03},//
  {0x3718,0x10},//
  {0x3719,0x08},//
  {0x371c,0xfc},//
  {0x3760,0x13},//
  {0x3761,0x34},//
  {0x3767,0x24},//
  {0x3768,0x06},//
  {0x3769,0x45},//
  {0x376c,0x23},//
  {0x3d84,0x00},//  ; OTP program disable
  {0x3d85,0x17},//  ; OTP power up load data enable, power load setting enable, software load setting


  {0x3d8c,0x73},//  ; OTP start address H
  {0x3d8d,0xbf},//  ; OTP start address L
  {0x3800,0x00},//  ; H crop start H
  {0x3801,0x08},//  ; H crop start L
  {0x3802,0x00},//  ; V crop start H
  {0x3803,0x04},//  ; V crop start L
  {0x3804,0x10},//  ; H crop end H
  {0x3805,0x97},//  ; H crop end L
  {0x3806,0x0c},//  ; V crop end H
  {0x3807,0x4b},//  ; V crop end L
  {0x3808,0x08},//  ; H output size H
  {0x3809,0x40},//  ; H output size L
  {0x380a,0x06},//  ; V output size H
  {0x380b,0x20},//  ; V output size L
  {0x380c,0x25},//  ; HTS H
  {0x380d,0x80},//  ; HTS L
  {0x380e,0x06},//  ; VTS H
  {0x380f,0x80},//  ; VTS L
  {0x3810,0x00},//  ; H win off H
  {0x3811,0x04},//  ; H win off L
  {0x3812,0x00},//  ; V win off H
  {0x3813,0x02},//  ; V win off L
  {0x3814,0x31},//  ; H inc
  {0x3815,0x31},//  ; V inc
  {0x3820,0x02},//  ; V flip off, V bin on
  {0x3821,0x05},//  ; H mirror on, H bin on
  {0x3834,0x00},//
  {0x3835,0x1c},//  ; cut_en, vts_auto, blk_col_dis
  {0x3836,0x08},//
  {0x3837,0x02},//
  {0x4000,0xf1},//  ; BLC offset trig en, format change trig en, gain trig en, exp trig en, median en
  {0x4001,0x00},//  ; BLC
  {0x400b,0x0c},//  ; BLC
  {0x4011,0x00},//  ; BLC
  {0x401a,0x00},//  ; BLC
  {0x401b,0x00},//  ; BLC
  {0x401c,0x00},//  ; BLC
  {0x401d,0x00},//  ; BLC
  {0x4020,0x00},//  ; BLC
  {0x4021,0xe4},//  ; BLC
  {0x4022,0x07},//  ; BLC
  {0x4023,0x5f},//  ; BLC
  {0x4024,0x08},//  ; BLC
  {0x4025,0x44},//  ; BLC
  {0x4026,0x08},//  ; BLC
  {0x4027,0x47},//  ; BLC
  {0x4028,0x00},//  ; BLC
  {0x4029,0x02},//  ; BLC
  {0x402a,0x04},//  ; BLC
  {0x402b,0x08},//  ; BLC
  {0x402c,0x02},//  ; BLC
  {0x402d,0x02},//  ; BLC
  {0x402e,0x0c},//  ; BLC
  {0x402f,0x08},//  ; BLC
  {0x403d,0x2c},//
  {0x403f,0x7f},//
  {0x4500,0x82},//  ; BLC
  {0x4501,0x38},//  ; BLC
  {0x4601,0x04},//
  {0x4602,0x22},//
  {0x4603,0x01},// ; VFIFO
  {0x4837,0x19},// ; MIPI global timing
  {0x4d00,0x04},//  ; temperature monitor
  {0x4d01,0x42},//  ; temperature monitor
  {0x4d02,0xd1},//  ; temperature monitor
  {0x4d03,0x90},//  ; temperature monitor
  {0x4d04,0x66},//  ; temperature monitor
  {0x4d05,0x65},//  ; temperature monitor
  {0x5000,0x0f},//  ; windowing enable, BPC on, WPC on, Lenc on
  {0x5001,0x03},//  ; BLC enable, MWB on
  {0x5002,0x07},//
  {0x5013,0x40},//
  {0x501c,0x00},//
  {0x501d,0x10},//
  {0x5242,0x00},//
  {0x5243,0xb8},//
  {0x5244,0x00},//
  {0x5245,0xf9},//
  {0x5246,0x00},//
  {0x5247,0xf6},//
  {0x5248,0x00},//
  {0x5249,0xa6},//
  {0x5300,0xfc},//
  {0x5301,0xdf},//
  {0x5302,0x3f},//
  {0x5303,0x08},//
  {0x5304,0x0c},//
  {0x5305,0x10},//
  {0x5306,0x20},//
  {0x5307,0x40},//
  {0x5308,0x08},//
  {0x5309,0x08},//
  {0x530a,0x02},//
  {0x530b,0x01},//
  {0x530c,0x01},//
  {0x530d,0x0c},//
  {0x530e,0x02},//
  {0x530f,0x01},//
  {0x5310,0x01},//
  {0x5400,0x00},//
  {0x5401,0x61},//
  {0x5402,0x00},//
  {0x5403,0x00},//
  {0x5404,0x00},//
  {0x5405,0x40},//
  {0x540c,0x05},//
  {0x5b00,0x00},//
  {0x5b01,0x00},//
  {0x5b02,0x01},//
  {0x5b03,0xff},//
  {0x5b04,0x02},//
  {0x5b05,0x6c},//
  {0x5b09,0x02},//
  {0x5e00,0x00},//  ; test pattern disable
  {0x5e10,0x1c},//  ; ISP test disable
};


static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
  {
    .reg_setting = init_reg_array0,
    .size = ARRAY_SIZE(init_reg_array0),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 50,
  },
  {
    .reg_setting = init_reg_array1,
    .size = ARRAY_SIZE(init_reg_array1),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
};


static struct sensor_lib_reg_settings_array init_settings_array = {
  .reg_settings = init_reg_setting,
  .size = 2,
};


static struct msm_camera_i2c_reg_array start_reg_array[] = {
  {0x0100,0x01},
};


static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};


static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  {0x0100,0x00},
};


static struct msm_camera_i2c_reg_setting stop_settings = {
  .reg_setting = stop_reg_array,
  .size = ARRAY_SIZE(stop_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};


static struct msm_camera_i2c_reg_array groupon_reg_array[] = {
  {0x3208,0x00},
};


static struct msm_camera_i2c_reg_setting groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};


static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
  {0x3208,0x10},
  {0x3208,0xA0},
};


static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};


static struct msm_camera_csid_vc_cfg ov13850_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};


static struct msm_camera_csi2_params ov13850_csi_params = {
  .csid_params = {
    .lane_cnt = 4,
    .lut_params = {
      .num_cid = 2,
      .vc_cfg = {
         &ov13850_cid_cfg[0],
         &ov13850_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 4,
    .settle_cnt = 24,
  },
};


static struct msm_camera_csi2_params *csi_params[] = {
#if RES_0_ENABLE
  &ov13850_csi_params, /* RES 0*/
#endif
#if RES_1_ENABLE
  &ov13850_csi_params, /* RES 1*/
#endif
#if RES_2_ENABLE
  &ov13850_csi_params, /* RES 2*/
#endif
};


static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};


static struct sensor_pix_fmt_info_t ov13850_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SBGGR10 },
};


static struct sensor_pix_fmt_info_t ov13850_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};


static sensor_stream_info_t ov13850_stream_info[] = {
  {1, &ov13850_cid_cfg[0], ov13850_pix_fmt0_fourcc},
  {1, &ov13850_cid_cfg[1], ov13850_pix_fmt1_fourcc},
};


static sensor_stream_info_array_t ov13850_stream_info_array = {
  .sensor_stream_info = ov13850_stream_info,
  .size = ARRAY_SIZE(ov13850_stream_info),
};


#if RES_0_ENABLE
static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  {0x0300,0x00},// ; PLL
  {0x0302,0x21},// ; PLL
  {0x3613,0x22},// ; PLL
  {0x3614,0x1b},// ; PLL
  {0x3501,0xcf},// ; Exposure H
  {0x370a,0x24},//
  {0x372a,0x04},//
  {0x372f,0xa0},//
  {0x3801,0x14},// ; H crop start L
  {0x3803,0x0c},// ; V crop start L
  {0x3805,0x8b},// ; H crop end L
  {0x3807,0x43},// ; V crop end L
  {0x3808,0x10},// ; H output size H
  {0x3809,0x70},// ; H output size L
  {0x380a,0x0c},// ; V output size H
  {0x380b,0x30},// ; V output size L
  {0x380c,0x12},//0x25},// ; HTS H
  {0x380d,0x70},//0x80},// ; HTS L
  {0x380e,0x0c},//0x0d},// ; VTS H
  {0x380f,0xe0},//0x00},// ; VTS L
  {0x3813,0x04},// ; V win off
  {0x3814,0x11},// ; H inc
  {0x3815,0x11},// ; V inc
  {0x3820,0x00},// ; V flip off, V bin on
  {0x3821,0x04},// ; H mirror on, H bin on
  {0x3836,0x04},//
  {0x3837,0x01},//
  {0x4020,0x02},//
  {0x4021,0x3c},//
  {0x4022,0x0e},//
  {0x4023,0x37},//
  {0x4024,0x0f},//
  {0x4025,0x1c},//
  {0x4026,0x0f},//
  {0x4027,0x1f},//
  {0x4603,0x01},// ; VFIFO
  {0x4837,0x14},//0x19},// ; MIPI global timing
  {0x5401,0x71},//
  {0x5405,0x80},//
};
#endif


#if RES_1_ENABLE
static struct msm_camera_i2c_reg_array res1_reg_array[] = {
  {0x0300,0x01},// ; PLL
  {0x0302,0x28},// ; PLL
  {0x3613,0x33},// ; PLL
  {0x3614,0x28},// ; PLL
  {0x0300,0x01},// ; PLL
  {0x0302,0x28},// ; PLL
  {0x3501,0x67},// ; Exposure H
  {0x370a,0x26},//
  {0x372a,0x00},//
  {0x372f,0x90},//
  {0x3801,0x08},// ; H crop start L
  {0x3803,0x04},// ; V crop start L
  {0x3805,0x97},// ; H crop end L
  {0x3807,0x4b},// ; V crop end L
  {0x3808,0x08},// ; H output size H
  {0x3809,0x38},// ; H output size L
  {0x380a,0x06},// ; V output size H
  {0x380b,0x18},// ; V output size L
  {0x380c,0x25},// ; HTS H
  {0x380d,0x80},// ; HTS L
  {0x380e,0x06},// ; VTS H
  {0x380f,0x80},// ; VTS L
  {0x3813,0x02},// ; V win off
  {0x3814,0x31},// ; H inc
  {0x3815,0x31},// ; V inc
  {0x3820,0x00},// ; V flip off, V bin on
  {0x3821,0x05},// ; H mirror on, H bin on
  {0x3836,0x08},//
  {0x3837,0x02},//
  {0x4020,0x00},//
  {0x4021,0xe4},//
  {0x4022,0x07},//
  {0x4023,0x5f},//
  {0x4024,0x08},//
  {0x4025,0x44},//
  {0x4026,0x08},//
  {0x4027,0x47},//
  {0x4603,0x01},// ; VFIFO
  {0x4837,0x19},// ; MIPI global timing
  {0x5401,0x61},//
  {0x5405,0x40},//
};
#endif


#if RES_2_ENABLE
static struct msm_camera_i2c_reg_array res2_reg_array[] = {
  {0x0300,0x01},
  {0x0301,0x00},
  {0x0302,0x28},
  {0x0303,0x00},
  {0x0304,0x03},
  {0x0305,0x01},
  {0x0306,0x01},
  {0x030a,0x00},
  {0x3611,0x10},
  {0x3612,0x23},
  {0x3613,0x33},
  {0x3614,0x28},
  {0x3615,0x08},
  {0x3103,0x00},
  {0x3012,0x41},
  {0x300f,0x11},
  {0x3501,0x60},
  {0x370a,0x26},
  {0x372a,0x00},
  {0x372f,0x90},
  {0x3801,0x08},
  {0x3803,0x04},
  {0x3805,0x97},
  {0x3807,0x4b},
  {0x3808,0x05},
  {0x3809,0x00},
  {0x380a,0x02},
  {0x380b,0xd0},
  {0x380c,0x12},
  {0x380d,0x70},
  {0x380e,0x06},
  {0x380f,0x80},
  {0x3810,0x01},
  {0x3811,0xa4},
  {0x3812,0x01},
  {0x3813,0xaa},
  {0x3814,0x31},
  {0x3815,0x31},
  {0x3820,0x02},
  {0x3821,0x05},
  {0x3834,0x00},
  {0x3836,0x08},
  {0x3837,0x02},
  {0x4020,0x02},
  {0x4021,0x40},
  {0x4022,0x03},
  {0x4023,0x3f},
  {0x4024,0x06},
  {0x4025,0xf8},
  {0x4026,0x07},
  {0x4027,0xf7},
  {0x4601,0x04},
  {0x4603,0x00},
  {0x4837,0x19},
  {0x5401,0x61},
  {0x5405,0x40},
};
#endif


static struct msm_camera_i2c_reg_setting res_settings[] = {
#if RES_0_ENABLE
  {
    .reg_setting = res0_reg_array,
    .size = ARRAY_SIZE(res0_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
#endif
#if RES_1_ENABLE
  {
    .reg_setting = res1_reg_array,
    .size = ARRAY_SIZE(res1_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
#endif
#if RES_2_ENABLE
  {
    .reg_setting = res2_reg_array,
    .size = ARRAY_SIZE(res2_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
#endif
};


static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};


static struct sensor_crop_parms_t crop_params[] = {
#if RES_0_ENABLE
  {0, 0, 0, 0}, /* RES 0 */
#endif
#if RES_1_ENABLE
  {0, 0, 0, 0}, /* RES 1 */
#endif
#if RES_2_ENABLE
  {0, 0, 0, 0}, /* RES 2 */
#endif
};


static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};


static struct sensor_lib_out_info_t sensor_out_info[] = {
#if RES_0_ENABLE
  {
    .x_output = 4208,
    .y_output = 3120,
    .line_length_pclk = 4720,
    .frame_length_lines = 3296,
    .vt_pixel_clk = 323600000,
    .op_pixel_clk = 316800000,
    .binning_factor = 0,
    .max_fps = 20.8,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#endif
#if RES_1_ENABLE
  {
    .x_output = 2104,
    .y_output = 1560,
    .line_length_pclk = 9600,
    .frame_length_lines = 1664,
    .vt_pixel_clk = 480000000,
    .op_pixel_clk = 256000000,
    .binning_factor = 1,
    .max_fps = 30,
    .min_fps = 7.5,
    .mode = SENSOR_DEFAULT_MODE,
  },
#endif
#if RES_2_ENABLE
  {
    .x_output = 1280,
    .y_output = 720,
    .line_length_pclk = 4720,
    .frame_length_lines = 1664,
    .vt_pixel_clk = 480000000,
    .op_pixel_clk = 320000000,
    .binning_factor = 1,
    .max_fps = 61.0,
    .min_fps = 7.5,
    .mode = SENSOR_HFR_MODE,
  },
#endif
};


static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};


static sensor_res_cfg_type_t ov13850_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};


static struct sensor_res_cfg_table_t ov13850_res_table = {
  .res_cfg_type = ov13850_res_cfg,
  .size = ARRAY_SIZE(ov13850_res_cfg),
};


static struct sensor_lib_chromatix_t ov13850_chromatix[] = {
#if RES_0_ENABLE
  {
    .common_chromatix = SKUF_OV13850_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = SKUF_OV13850_LOAD_CHROMATIX(snapshot),
    .camera_snapshot_chromatix = SKUF_OV13850_LOAD_CHROMATIX(snapshot),
    .camcorder_chromatix = SKUF_OV13850_LOAD_CHROMATIX(snapshot),
    .liveshot_chromatix = SKUF_OV13850_LOAD_CHROMATIX(snapshot),
  },
#endif
#if RES_1_ENABLE
  {
    .common_chromatix = SKUF_OV13850_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = SKUF_OV13850_LOAD_CHROMATIX(preview),
    .camera_snapshot_chromatix = SKUF_OV13850_LOAD_CHROMATIX(preview),
    .camcorder_chromatix = SKUF_OV13850_LOAD_CHROMATIX(default_video),
    .liveshot_chromatix = SKUF_OV13850_LOAD_CHROMATIX(preview),
  },
#endif
#if RES_2_ENABLE
  {
    .common_chromatix = SKUF_OV13850_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = SKUF_OV13850_LOAD_CHROMATIX(hfr_60fps),
    .camera_snapshot_chromatix = SKUF_OV13850_LOAD_CHROMATIX(hfr_60fps),
    .camcorder_chromatix = SKUF_OV13850_LOAD_CHROMATIX(hfr_60fps),
    .liveshot_chromatix = SKUF_OV13850_LOAD_CHROMATIX(hfr_60fps),
  },
#endif
};


static struct sensor_lib_chromatix_array ov13850_lib_chromatix_array = {
  .sensor_lib_chromatix = ov13850_chromatix,
  .size = ARRAY_SIZE(ov13850_chromatix),
};


/*===========================================================================
 * FUNCTION    - ov13850_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov13850_real_to_register_gain(float gain)
{
  uint16_t reg_gain, reg_temp;
  if (gain < 1.0) {
      gain = 1.0;
  } else if (gain > 15.5) {
      gain = 15.5;
  }
  gain = (gain) * 16.0;
  reg_gain = (uint16_t) gain;
  return reg_gain;
}




/*===========================================================================
 * FUNCTION    - ov13850_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float ov13850_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;


  if (reg_gain < 0x10) {
      reg_gain = 0x10;
  } else if (reg_gain >= 0xf8) {
      reg_gain = 0xf8;
  }
  real_gain = (float) reg_gain / 16.0;
  return real_gain;
}




/*===========================================================================
 * FUNCTION    - ov13850_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov13850_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = ov13850_real_to_register_gain(real_gain);
  float sensor_real_gain =
    ov13850_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / sensor_real_gain;
  exp_info->line_count = line_count;
  return 0;
}


/*===========================================================================
 * FUNCTION    - ov13850_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov13850_fill_exposure_array(uint16_t gain,
        uint32_t line,uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
        struct msm_camera_i2c_reg_setting *reg_setting)
{
  int32_t rc = 0;
  uint16_t reg_count = 0;
  uint16_t i = 0;
  DEBUG_INFO("%s:gain=%d,line=%d,fl_lines=%d",__func__,
    gain,line,fl_lines) ;


  if (!reg_setting) {
    return -1;
  }


  for (i = 0; i < sensor_lib_ptr.groupon_settings->size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupon_settings->reg_setting[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupon_settings->reg_setting[i].reg_data;
    reg_count = reg_count + 1;
  }


  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr->frame_length_lines;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF00) >> 8;
  DEBUG_INFO("%s:fl_lines:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;


  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr->frame_length_lines + 1;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF);
  DEBUG_INFO("%s:fl_lines:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;


  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = (line&0xffff) >> 12;
  DEBUG_INFO("%s:lines:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;


  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (line&0xfff) >> 4;
  DEBUG_INFO("%s:lines:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;


  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 2;
  reg_setting->reg_setting[reg_count].reg_data = (line&0x0f) << 4;
  DEBUG_INFO("%s:lines:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;


  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0x3FF) >> 8;
  DEBUG_INFO("%s:gain:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;


  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
  DEBUG_INFO("%s:gain:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;


  for (i = 0; i < sensor_lib_ptr.groupoff_settings->size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupoff_settings->reg_setting[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupoff_settings->reg_setting[i].reg_data;
    reg_count = reg_count + 1;
  }


  reg_setting->size = reg_count;
  reg_setting->addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  reg_setting->data_type = MSM_CAMERA_I2C_BYTE_DATA;
  reg_setting->delay = 0;


  return rc;
}


static sensor_exposure_table_t ov13850_expsoure_tbl = {
  .sensor_calculate_exposure = ov13850_calculate_exposure,
  .sensor_fill_exposure_array = ov13850_fill_exposure_array,
};


static sensor_lib_t sensor_lib_ptr = {
  /* sensor output settings */
  .sensor_output = &sensor_output,
  /* sensor output register address */
  .output_reg_addr = &output_reg_addr,
  /* sensor exposure gain register address */
  .exp_gain_info = &exp_gain_info,
  /* sensor aec info */
  .aec_info = &aec_info,
  /* sensor snapshot exposure wait frames info */
  .snapshot_exp_wait_frames = 2,
  /* number of frames to skip after start stream */
  .sensor_num_frame_skip = 2,
  /* sensor exposure table size */
  .exposure_table_size = 10,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = ov13850_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(ov13850_cid_cfg),
  /* init settings */
  .init_settings_array = &init_settings_array,
  /* start settings */
  .start_settings = &start_settings,
  /* stop settings */
  .stop_settings = &stop_settings,
  /* group on settings */
  .groupon_settings = &groupon_settings,
  /* group off settings */
  .groupoff_settings = &groupoff_settings,
  /* resolution cfg table */
  .sensor_res_cfg_table = &ov13850_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &ov13850_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &ov13850_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &ov13850_lib_chromatix_array,
};


/*===========================================================================
 * FUNCTION    - ov13850_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *ov13850_open_lib(void)
{
  return &sensor_lib_ptr;
}