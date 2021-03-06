/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include "msm_sensor.h"
#include "msm.h"
#define SENSOR_NAME "ov8825"
#define PLATFORM_DRIVER_NAME "msm_camera_ov8825"
#define ov8825_obj ov8825_##obj

#define OV8825_2LANES 1

#ifdef CDBG
#undef CDBG
#endif
#ifdef CDBG_HIGH
#undef CDBG_HIGH
#endif

#define OV8825_DGB

#ifdef OV8825_DGB
#define CDBG printk
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#else
#define CDBG printk
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#endif


/* TO DO - Currently ov5647 typical values are used
 * Need to get the exact values */
#define OV8825_RG_RATIO_TYPICAL_VALUE 0x51 /* R/G of typical camera module */
#define OV8825_BG_RATIO_TYPICAL_VALUE 0x4e /* B/G of typical camera module */

DEFINE_MUTEX(ov8825_mut);
static struct msm_sensor_ctrl_t ov8825_s_ctrl;

struct otp_struct {
	uint8_t customer_id;
	uint8_t module_integrator_id;
	uint8_t lens_id;
	uint8_t rg_ratio;
	uint8_t bg_ratio;
	uint8_t user_data[5];
} st_ov8825_otp;


static struct msm_camera_i2c_reg_conf ov8825_start_settings[] = {
	{0x0100, 0x01},
	{0x301c, 0xf0}, /* release clock */
	{0x301a, 0x70}, /* MIPI stream on */
};

static struct msm_camera_i2c_reg_conf ov8825_stop_settings[] = {
	{0x301a, 0x71}, /* MIPI stream off */
	{0x301c, 0xf4}, /* clock in LP11 mode */
	{0x0100, 0x00},
};

static struct msm_camera_i2c_reg_conf ov8825_groupon_settings[] = {
	{0x3208, 0x00},
};

static struct msm_camera_i2c_reg_conf ov8825_groupoff_settings[] = {
	{0x3208, 0x10},
	{0x3208, 0xA0},
};

static struct msm_camera_i2c_reg_conf ov8825_video_settings[] = {
	{0x3501, 0x2f},
	{0x3502, 0x90},
	{0x350b, 0x3f},
	{0x3003, 0xce},
	{0x3004, 0xd8},
	{0x3005, 0x00},
	{0x3006, 0x00},
	{0x3007, 0x3b},
	{0x3011, 0x01},
	{0x3012, 0x80},
	{0x3020, 0x01},
	{0x3013, 0x39},
	{0x3106, 0x15},
	{0x3600, 0x06},
	{0x3601, 0x34},
	{0x3602, 0x42},
	{0x3700, 0x20},
	{0x3702, 0x50},
	{0x3703, 0xcc},
	{0x3704, 0x19},
	{0x3705, 0x32},
	{0x3706, 0x4b},
	{0x3708, 0x84},
	{0x3709, 0x40},
	{0x370a, 0x31},
	{0x370e, 0x00},
	{0x3711, 0x0f},
	{0x3712, 0x9c},
	{0x3724, 0x01},
	{0x3725, 0x92},
	{0x3726, 0x01},
	{0x3727, 0xc7},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x01},
	{0x3803, 0x30},
	{0x3804, 0x0c},
	{0x3805, 0xdf},
	{0x3806, 0x08},
	{0x3807, 0x67},
	{0x3808, 0x07},
	{0x3809, 0x80},
	{0x380a, 0x04},
	{0x380b, 0x38},
	{0x380c, 0x0d},
	{0x380d, 0xf0},
	{0x380e, 0x07},
	{0x380f, 0x4c},
	{0x3810, 0x00},
	{0x3811, 0x10},
	{0x3812, 0x00},
	{0x3813, 0x06},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3820, 0x86},
	{0x3821, 0x10},
	{0x3f00, 0x02},
	{0x4005, 0x18},
	{0x404f, 0x8F},
	{0x4600, 0x04},
	{0x4601, 0x01},
	{0x4602, 0x00},
	{0x4837, 0x1e},
	{0x5068, 0x53},
	{0x506a, 0x53},
};

static struct msm_camera_i2c_reg_conf ov8825_prev_settings[] = {
	{0x3003, 0xce},
	{0x3004, 0xd8},
	{0x3005, 0x00},
	{0x3006, 0x10},
	{0x3007, 0x3b},
	{0x3011, 0x01},
	{0x3013, 0x39},
	{0x3012, 0x80},
	{0x3020, 0x01},
	{0x3106, 0x15},
	{0x3600, 0x06},
	{0x3601, 0x34},
	{0x3602, 0x42},
	{0x3700, 0x20},
	{0x3702, 0x50},
	{0x3703, 0xcc},
	{0x3704, 0x19},
	{0x3705, 0x32},
	{0x3706, 0x4b},
	{0x3708, 0x84},
	{0x3709, 0x40},
	{0x370a, 0x33},
	{0x370e, 0x00},
	{0x3711, 0x0f},
	{0x3712, 0x9c},
	{0x3724, 0x01},
	{0x3725, 0x92},
	{0x3726, 0x01},
	{0x3727, 0xc7},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0c},
	{0x3805, 0xdf},
	{0x3806, 0x09},
	{0x3807, 0x9b},
	{0x3808, 0x06},
	{0x3809, 0x60},
	{0x380a, 0x04},
	{0x380b, 0xc8},
	{0x380c, 0x0d},
	{0x380d, 0xbc},
	{0x380e, 0x04},
	{0x380f, 0xf4},
	{0x3810, 0x00},
	{0x3811, 0x08},
	{0x3812, 0x00},
	{0x3813, 0x04},
	{0x3814, 0x31},
	{0x3815, 0x31},
	{0x3820, 0x87},
	{0x3821, 0x11},
	{0x3f00, 0x00},
	{0x4005, 0x18},
	{0x404f, 0x8F},
	{0x4600, 0x04},
	{0x4601, 0x00},
	{0x4602, 0x30},
	{0x4837, 0x1e},
	{0x5068, 0x00},
	{0x506a, 0x00},
};

static struct msm_camera_i2c_reg_conf ov8825_snap_settings[] = {
	{0x3004, 0xd8},
	{0x3006, 0x10},
	{0x3020, 0x81},
	{0x3501, 0x9a},
	{0x3502, 0xa0},
	{0x3700, 0x10},
	{0x3702, 0x28},
	{0x3703, 0x6c},
	{0x3704, 0x40},
	{0x3705, 0x19},
	{0x3706, 0x27},
	{0x3708, 0x48},
	{0x3709, 0x20},
	{0x370a, 0x31},
	{0x3711, 0x07},
	{0x3712, 0x4e},
	{0x3724, 0x00},
	{0x3725, 0xd4},
	{0x3726, 0x00},
	{0x3727, 0xf0},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3806, 0x09},
	{0x3807, 0x9b},
	{0x3808, 0x0c},
	{0x3809, 0xc0},
	{0x380a, 0x09},
	{0x380b, 0x90},
	{0x380c, 0x0e},
	{0x380d, 0x00},
	{0x380e, 0x09},
	{0x380f, 0xbf},
	{0x3811, 0x10},
	{0x3813, 0x06},
	{0x3814, 0x11},
	{0x3815, 0x11},
	{0x3820, 0x86},
	{0x3821, 0x10},
	{0x3f00, 0x02},
	{0x4005, 0x1a},/* BLC triggered every frame */
	{0x4601, 0x00},
	{0x4602, 0x20},
	{0x4837, 0x1e},
	{0x5068, 0x00},
	{0x506a, 0x00},
};

static struct msm_camera_i2c_reg_conf ov8825_WVGA_60fps_settings[] = {
	{0x3501, 0x2f},
	{0x3502, 0x90},
	{0x350b, 0x3f},
	{0x3003, 0xce},
	{0x3004, 0xbf},
	{0x3005, 0x10},
	{0x3006, 0x00},
	{0x3007, 0x3b},
	{0x3011, 0x01},
	{0x3012, 0x80},
	{0x3013, 0x39},
	{0x3020, 0x01},
	{0x3106, 0x15},
	{0x3600, 0x06},
	{0x3601, 0x34},
	{0x3602, 0x42},
	{0x3700, 0x20},
	{0x3702, 0x50},
	{0x3703, 0xcc},
	{0x3704, 0x19},
	{0x3705, 0x32},
	{0x3706, 0x4b},
	{0x3708, 0x84},
	{0x3709, 0x40},
	{0x370a, 0xb2},
	{0x370e, 0x08},
	{0x3711, 0x0f},
	{0x3712, 0x9c},
	{0x3724, 0x01},
	{0x3725, 0x92},
	{0x3726, 0x01},
	{0x3727, 0xc7},
	{0x3800, 0x00},
	{0x3801, 0x08},
	{0x3802, 0x00},
	{0x3803, 0xf4},
	{0x3804, 0x0c},
	{0x3805, 0xd7},
	{0x3806, 0x08},
	{0x3807, 0xa3},
	{0x3808, 0x03},
	{0x3809, 0x20},
	{0x380a, 0x01},
	{0x380b, 0xe0},
	{0x380c, 0x0a},
	{0x380d, 0x44},
	{0x380e, 0x04},
	{0x380f, 0xf4},
	{0x3810, 0x00},
	{0x3811, 0x0a},
	{0x3812, 0x00},
	{0x3813, 0x06},
	{0x3814, 0x71},
	{0x3815, 0x35},
	{0x3820, 0x86},
	{0x3821, 0x10},
	{0x3f00, 0x00},
	{0x4005, 0x18},
	{0x404f, 0x8f},
	{0x4600, 0x14},
	{0x4601, 0x14},
	{0x4602, 0x00},
	{0x4837, 0x19},
	{0x5068, 0x59},
	{0x506a, 0x5a},
};

static struct msm_camera_i2c_reg_conf ov8825_WVGA_90fps_settings[] = {
	{0x3501, 0x2f},
	{0x3502, 0x90},
	{0x350b, 0x3f},
	{0x3003, 0xce},
	{0x3004, 0xbf},
	{0x3005, 0x10},
	{0x3006, 0x00},
	{0x3007, 0x3b},
	{0x3011, 0x01},
	{0x3012, 0x80},
	{0x3013, 0x39},
	{0x3020, 0x01},
	{0x3106, 0x15},
	{0x3600, 0x06},
	{0x3601, 0x34},
	{0x3602, 0x42},
	{0x3700, 0x20},
	{0x3702, 0x50},
	{0x3703, 0xcc},
	{0x3704, 0x19},
	{0x3705, 0x32},
	{0x3706, 0x4b},
	{0x3708, 0x84},
	{0x3709, 0x40},
	{0x370a, 0xb2},
	{0x370e, 0x08},
	{0x3711, 0x0f},
	{0x3712, 0x9c},
	{0x3724, 0x01},
	{0x3725, 0x92},
	{0x3726, 0x01},
	{0x3727, 0xc7},
	{0x3800, 0x00},
	{0x3801, 0x08},
	{0x3802, 0x00},
	{0x3803, 0xf4},
	{0x3804, 0x0c},
	{0x3805, 0xd7},
	{0x3806, 0x08},
	{0x3807, 0xa3},
	{0x3808, 0x03},
	{0x3809, 0x20},
	{0x380a, 0x01},
	{0x380b, 0xe0},
	{0x380c, 0x0a},
	{0x380d, 0x44},
	{0x380e, 0x03},
	{0x380f, 0x50},
	{0x3810, 0x00},
	{0x3811, 0x0a},
	{0x3812, 0x00},
	{0x3813, 0x06},
	{0x3814, 0x71},
	{0x3815, 0x35},
	{0x3820, 0x86},
	{0x3821, 0x10},
	{0x3f00, 0x00},
	{0x4005, 0x18},
	{0x404f, 0x8F},
	{0x4600, 0x14},
	{0x4601, 0x14},
	{0x4602, 0x00},
	{0x4837, 0x19},
	{0x5068, 0x59},
};

static struct msm_camera_i2c_reg_conf ov8825_WVGA_120fps_settings[] = {
	{0x3501, 0x27},
	{0x3502, 0x90},
	{0x350b, 0x3f},
	{0x3003, 0xce},
	{0x3004, 0xbf},
	{0x3005, 0x10},
	{0x3006, 0x00},
	{0x3007, 0x3b},
	{0x3011, 0x01},
	{0x3012, 0x80},
	{0x3013, 0x39},
	{0x3020, 0x01},
	{0x3106, 0x15},
	{0x3600, 0x06},
	{0x3601, 0x34},
	{0x3602, 0x42},
	{0x3700, 0x20},
	{0x3702, 0x50},
	{0x3703, 0xcc},
	{0x3704, 0x19},
	{0x3705, 0x32},
	{0x3706, 0x4b},
	{0x3708, 0x84},
	{0x3709, 0x40},
	{0x370a, 0xb2},
	{0x370e, 0x08},
	{0x3711, 0x0f},
	{0x3712, 0x9c},
	{0x3724, 0x01},
	{0x3725, 0x92},
	{0x3726, 0x01},
	{0x3727, 0xc7},
	{0x3800, 0x00},
	{0x3801, 0x08},
	{0x3802, 0x00},
	{0x3803, 0xf4},
	{0x3804, 0x0c},
	{0x3805, 0xd7},
	{0x3806, 0x08},
	{0x3807, 0xa3},
	{0x3808, 0x03},
	{0x3809, 0x20},
	{0x380a, 0x01},
	{0x380b, 0xe0},
	{0x380c, 0x0a},
	{0x380d, 0x44},
	{0x380e, 0x02},
	{0x380f, 0x7a},
	{0x3810, 0x00},
	{0x3811, 0x0a},
	{0x3812, 0x00},
	{0x3813, 0x06},
	{0x3814, 0x71},
	{0x3815, 0x35},
	{0x3820, 0x86},
	{0x3821, 0x10},
	{0x3f00, 0x00},
	{0x4005, 0x18},
	{0x404f, 0x8F},
	{0x4600, 0x14},
	{0x4601, 0x14},
	{0x4602, 0x00},
	{0x4837, 0x19},
	{0x5068, 0x59},
	{0x506a, 0x5a},
};

static struct msm_camera_i2c_reg_conf ov8825_recommend_settings[] = {
	{0x3000, 0x16},
	{0x3001, 0x00},
	{0x3002, 0x6c},
	{0x3003, 0xce},
	{0x3004, 0xd4},
	{0x3005, 0x00},
	{0x3006, 0x10},
	{0x3007, 0x3b},
	{0x300d, 0x00},
	{0x301f, 0x09},
	{0x3020, 0x01},
	{0x3010, 0x00},
	{0x3011, 0x01},
	{0x3012, 0x80},
	{0x3013, 0x39},
	{0x3018, 0x00},
	{0x3104, 0x20},
	{0x3106, 0x15},
	{0x3300, 0x00},
	{0x3500, 0x00},
	{0x3501, 0x4e},
	{0x3502, 0xa0},
	{0x3503, 0x07},
	{0x3509, 0x00},
	{0x350b, 0x1f},
	{0x3600, 0x06},
	{0x3601, 0x34},
	{0x3602, 0x42},
	{0x3603, 0x5c},
	{0x3604, 0x98},
	{0x3605, 0xf5},
	{0x3609, 0xb4},
	{0x360a, 0x7c},
	{0x360b, 0xc9},
	{0x360c, 0x0b},
	{0x3612, 0x00},
	{0x3613, 0x02},
	{0x3614, 0x0f},
	{0x3615, 0x00},
	{0x3616, 0x03},
	{0x3617, 0xa1},
	{0x3618, 0x00},
	{0x3619, 0x00},
	{0x361a, 0xb0},
	{0x361b, 0x04},
	{0x3700, 0x20},
	{0x3701, 0x44},
	{0x3702, 0x50},
	{0x3703, 0xcc},
	{0x3704, 0x19},
	{0x3705, 0x32},
	{0x3706, 0x4b},
	{0x3707, 0x63},
	{0x3708, 0x84},
	{0x3709, 0x40},
	{0x370a, 0x33},
	{0x370b, 0x01},
	{0x370c, 0x50},
	{0x370d, 0x00},
	{0x370e, 0x00},
	{0x3711, 0x0f},
	{0x3712, 0x9c},
	{0x3724, 0x01},
	{0x3725, 0x92},
	{0x3726, 0x01},
	{0x3727, 0xc7},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x00},
	{0x3804, 0x0c},
	{0x3805, 0xdf},
	{0x3806, 0x09},
	{0x3807, 0x9b},
	{0x3808, 0x06},
	{0x3809, 0x60},
	{0x380a, 0x04},
	{0x380b, 0xc8},
	{0x380c, 0x0d},
	{0x380d, 0xbc},
	{0x380e, 0x04},
	{0x380f, 0xf0},
	{0x3810, 0x00},
	{0x3811, 0x08},
	{0x3812, 0x00},
	{0x3813, 0x04},
	{0x3814, 0x31},
	{0x3815, 0x31},
	{0x3816, 0x02},
	{0x3817, 0x40},
	{0x3818, 0x00},
	{0x3819, 0x40},
	{0x3820, 0x86},
	{0x3821, 0x10},
	{0x3b1f, 0x00},
	{0x3d00, 0x00},
	{0x3d01, 0x00},
	{0x3d02, 0x00},
	{0x3d03, 0x00},
	{0x3d04, 0x00},
	{0x3d05, 0x00},
	{0x3d06, 0x00},
	{0x3d07, 0x00},
	{0x3d08, 0x00},
	{0x3d09, 0x00},
	{0x3d0a, 0x00},
	{0x3d0b, 0x00},
	{0x3d0c, 0x00},
	{0x3d0d, 0x00},
	{0x3d0e, 0x00},
	{0x3d0f, 0x00},
	{0x3d10, 0x00},
	{0x3d11, 0x00},
	{0x3d12, 0x00},
	{0x3d13, 0x00},
	{0x3d14, 0x00},
	{0x3d15, 0x00},
	{0x3d16, 0x00},
	{0x3d17, 0x00},
	{0x3d18, 0x00},
	{0x3d19, 0x00},
	{0x3d1a, 0x00},
	{0x3d1b, 0x00},
	{0x3d1c, 0x00},
	{0x3d1d, 0x00},
	{0x3d1e, 0x00},
	{0x3d1f, 0x00},
	{0x3d80, 0x00},
	{0x3d81, 0x00},
	{0x3d84, 0x00},
	{0x3f00, 0x00},
	{0x3f01, 0xfc},
	{0x3f05, 0x10},
	{0x3f06, 0x00},
	{0x3f07, 0x00},
	{0x4000, 0x29},
	{0x4001, 0x02},
	{0x4002, 0x45},
	{0x4003, 0x08},
	{0x4004, 0x04},
	{0x4005, 0x18},
	{0x404e, 0x37},
	{0x404f, 0x8f},
	{0x4300, 0xff},
	{0x4303, 0x00},
	{0x4304, 0x08},
	{0x4307, 0x00},
	{0x4600, 0x04},
	{0x4601, 0x00},
	{0x4602, 0x30},
	{0x4800, 0x04},
	{0x4801, 0x0f},
	{0x4837, 0x28},
	{0x4843, 0x02},
	{0x5000, 0x06},
	{0x5001, 0x00},
	{0x5002, 0x00},
	{0x5068, 0x00},
	{0x506a, 0x00},
	{0x501f, 0x00},
	{0x5780, 0xfc},
	{0x5c00, 0x80},
	{0x5c01, 0x00},
	{0x5c02, 0x00},
	{0x5c03, 0x00},
	{0x5c04, 0x00},
	{0x5c05, 0x00},
	{0x5c06, 0x00},
	{0x5c07, 0x80},
	{0x5c08, 0x10},
	{0x6700, 0x05},
	{0x6701, 0x19},
	{0x6702, 0xfd},
	{0x6703, 0xd7},
	{0x6704, 0xff},
	{0x6705, 0xff},
	{0x6800, 0x10},
	{0x6801, 0x02},
	{0x6802, 0x90},
	{0x6803, 0x10},
	{0x6804, 0x59},
	{0x6900, 0x60},
	{0x6901, 0x04},
	{0x5800, 0x0f},
	{0x5801, 0x0d},
	{0x5802, 0x09},
	{0x5803, 0x0a},
	{0x5804, 0x0d},
	{0x5805, 0x14},
	{0x5806, 0x0a},
	{0x5807, 0x04},
	{0x5808, 0x03},
	{0x5809, 0x03},
	{0x580a, 0x05},
	{0x580b, 0x0a},
	{0x580c, 0x05},
	{0x580d, 0x02},
	{0x580e, 0x00},
	{0x580f, 0x00},
	{0x5810, 0x03},
	{0x5811, 0x05},
	{0x5812, 0x09},
	{0x5813, 0x03},
	{0x5814, 0x01},
	{0x5815, 0x01},
	{0x5816, 0x04},
	{0x5817, 0x09},
	{0x5818, 0x09},
	{0x5819, 0x08},
	{0x581a, 0x06},
	{0x581b, 0x06},
	{0x581c, 0x08},
	{0x581d, 0x06},
	{0x581e, 0x33},
	{0x581f, 0x11},
	{0x5820, 0x0e},
	{0x5821, 0x0f},
	{0x5822, 0x11},
	{0x5823, 0x3f},
	{0x5824, 0x08},
	{0x5825, 0x46},
	{0x5826, 0x46},
	{0x5827, 0x46},
	{0x5828, 0x46},
	{0x5829, 0x46},
	{0x582a, 0x42},
	{0x582b, 0x42},
	{0x582c, 0x44},
	{0x582d, 0x46},
	{0x582e, 0x46},
	{0x582f, 0x60},
	{0x5830, 0x62},
	{0x5831, 0x42},
	{0x5832, 0x46},
	{0x5833, 0x46},
	{0x5834, 0x44},
	{0x5835, 0x44},
	{0x5836, 0x44},
	{0x5837, 0x48},
	{0x5838, 0x28},
	{0x5839, 0x46},
	{0x583a, 0x48},
	{0x583b, 0x68},
	{0x583c, 0x28},
	{0x583d, 0xae},
	{0x5842, 0x00},
	{0x5843, 0xef},
	{0x5844, 0x01},
	{0x5845, 0x3f},
	{0x5846, 0x01},
	{0x5847, 0x3f},
	{0x5848, 0x00},
	{0x5849, 0xd5},
	/* MWB */
	{0x3400, 0x04}, /* red h */
	{0x3401, 0x00}, /* red l */
	{0x3402, 0x04}, /* green h */
	{0x3403, 0x00}, /* green l */
	{0x3404, 0x04}, /* blue h */
	{0x3405, 0x00}, /* blue l */
	{0x3406, 0x01}, /* MWB manual */
	/* ISP */
	{0x5001, 0x01}, /* MWB on */
	{0x5000, 0x86}, /* LENC on, BPC on, WPC on */
};

static struct v4l2_subdev_info ov8825_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array ov8825_init_conf[] = {
	{&ov8825_recommend_settings[0],
	ARRAY_SIZE(ov8825_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array ov8825_confs[] = {
	{&ov8825_snap_settings[0],
		ARRAY_SIZE(ov8825_snap_settings),
		0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov8825_prev_settings[0],
		ARRAY_SIZE(ov8825_prev_settings),
		0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov8825_WVGA_60fps_settings[0],
		ARRAY_SIZE(ov8825_WVGA_60fps_settings),
		0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov8825_WVGA_90fps_settings[0],
		ARRAY_SIZE(ov8825_WVGA_90fps_settings),
		0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov8825_WVGA_120fps_settings[0],
		ARRAY_SIZE(ov8825_WVGA_120fps_settings),
		0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov8825_video_settings[0],
		ARRAY_SIZE(ov8825_video_settings),
		0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t ov8825_dimensions[] = {
	{
		.x_output = 0xCC0,
		.y_output = 0x990,
		.line_length_pclk = 0xE00,
		.frame_length_lines = 0x9BF,
#if OV8825_2LANES
		.vt_pixel_clk = 133400000,
#else
		.vt_pixel_clk = 133000000,
#endif
		.op_pixel_clk = 176000000,
		.binning_factor = 1,
	},
	{
		.x_output = 1632,
		.y_output = 1224,
		.line_length_pclk = 3516,
		.frame_length_lines = 1268,
		.vt_pixel_clk = 133400000,
		.op_pixel_clk = 176000000,
		.binning_factor = 1,
	},
	/* WVGA 60fps */
	{
		.x_output = 800,
		.y_output = 480,
		.line_length_pclk = 0xa44,
		.frame_length_lines = 0x4f4,
		.vt_pixel_clk = 200000000,
		.op_pixel_clk = 176000000,
		.binning_factor = 0,
	},
	/* WVGA 90fps */
	{
		.x_output = 800,
		.y_output = 480,
		.line_length_pclk = 0xa44,
		.frame_length_lines = 0x350,
		.vt_pixel_clk = 200000000,
		.op_pixel_clk = 176000000,
		.binning_factor = 0,
	},
	/* WVGA 120fps */
	{
		.x_output = 800,
		.y_output = 480,
		.line_length_pclk = 0xa44,
		.frame_length_lines = 0x27a,
		.vt_pixel_clk = 200000000,
		.op_pixel_clk = 176000000,
		.binning_factor = 0,
	},
	{
		.x_output = 1920,
		.y_output = 1080,
		.line_length_pclk = 3568,
		.frame_length_lines = 1868,
		.vt_pixel_clk = 200000000,
		.op_pixel_clk = 176000000,
		.binning_factor = 1,
	},
};

static struct msm_sensor_output_reg_addr_t ov8825_reg_addr = {
	.x_output = 0x3808,
	.y_output = 0x380a,
	.line_length_pclk = 0x380c,
	.frame_length_lines = 0x380e,
};

static struct msm_sensor_id_info_t ov8825_id_info = {
	.sensor_id_reg_addr = 0x300A,
	.sensor_id = 0x8825,
};

static struct msm_sensor_exp_gain_info_t ov8825_exp_gain_info = {
	.coarse_int_time_addr = 0x3500,
	.global_gain_addr = 0x350A,
	.vert_offset = 6,
};

/********************************************
 * index: index of otp group. (0, 1, 2)
 * return value:
 *     0, group index is empty
 *     1, group index has invalid data
 *     2, group index has valid data
 **********************************************/
uint16_t ov8825_check_otp_wb(struct msm_sensor_ctrl_t *s_ctrl, uint16_t index)
{
	uint16_t temp, i;
	uint16_t address;

	/* clear otp buffer */

	/* select otp bank 0 */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3d84, 0x08,
			MSM_CAMERA_I2C_BYTE_DATA);

	/* load otp into buffer */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3d81, 0x01,
			MSM_CAMERA_I2C_BYTE_DATA);

	/* read from group [index] */
	address = 0x3d05 + index * 9;
	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, address, &temp,
			MSM_CAMERA_I2C_BYTE_DATA);

	/* disable otp read */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3d81, 0x00,
			MSM_CAMERA_I2C_BYTE_DATA);

	/* clear otp buffer */
	for (i = 0; i < 32; i++) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, (0x3d00+i),
				0x00, MSM_CAMERA_I2C_BYTE_DATA);
	}

	if (!temp)
		return 0;
	else if ((!(temp & 0x80)) && (temp & 0x7f))
		return 2;
	else
		return 1;
}

void ov8825_read_otp_wb(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t index, struct otp_struct *potp)
{
	uint16_t temp, i;
	uint16_t address;

	/* select otp bank 0 */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3d84, 0x08,
			MSM_CAMERA_I2C_BYTE_DATA);

	/* load otp data into buffer */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3d81, 0x01,
			MSM_CAMERA_I2C_BYTE_DATA);

	/* read otp data from 0x3d00 - 0x3d1f*/
	address = 0x3d05 + index * 9;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, address, &temp,
			MSM_CAMERA_I2C_BYTE_DATA);

	potp->module_integrator_id = temp;
	potp->customer_id = temp & 0x7f;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+1), &temp,
			MSM_CAMERA_I2C_BYTE_DATA);
	potp->lens_id = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+2), &temp,
			MSM_CAMERA_I2C_BYTE_DATA);
	potp->rg_ratio = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+3), &temp,
			MSM_CAMERA_I2C_BYTE_DATA);
	potp->bg_ratio = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+4), &temp,
			MSM_CAMERA_I2C_BYTE_DATA);
	potp->user_data[0] = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+5), &temp,
			MSM_CAMERA_I2C_BYTE_DATA);
	potp->user_data[1] = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+6), &temp,
			MSM_CAMERA_I2C_BYTE_DATA);
	potp->user_data[2] = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+7), &temp,
			MSM_CAMERA_I2C_BYTE_DATA);
	potp->user_data[3] = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+8), &temp,
			MSM_CAMERA_I2C_BYTE_DATA);
	potp->user_data[4] = temp;

	CDBG("%s customer_id  = 0x%02x\r\n", __func__, potp->customer_id);
	CDBG("%s lens_id      = 0x%02x\r\n", __func__, potp->lens_id);
	CDBG("%s rg_ratio     = 0x%02x\r\n", __func__, potp->rg_ratio);
	CDBG("%s bg_ratio     = 0x%02x\r\n", __func__, potp->bg_ratio);
	CDBG("%s user_data[0] = 0x%02x\r\n", __func__, potp->user_data[0]);
	CDBG("%s user_data[1] = 0x%02x\r\n", __func__, potp->user_data[1]);
	CDBG("%s user_data[2] = 0x%02x\r\n", __func__, potp->user_data[2]);
	CDBG("%s user_data[3] = 0x%02x\r\n", __func__, potp->user_data[3]);
	CDBG("%s user_data[4] = 0x%02x\r\n", __func__, potp->user_data[4]);

	/* disable otp read */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3d81, 0x00,
			MSM_CAMERA_I2C_BYTE_DATA);

	/* clear otp buffer */
	for (i = 0; i < 32; i++)
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, (0x3d00+i),
				0x00, MSM_CAMERA_I2C_BYTE_DATA);
}

/**********************************************
 * r_gain, sensor red gain of AWB, 0x400 =1
 * g_gain, sensor green gain of AWB, 0x400 =1
 * b_gain, sensor blue gain of AWB, 0x400 =1
 ***********************************************/
void ov8825_update_awb_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t r_gain, uint16_t g_gain, uint16_t b_gain)
{
	CDBG("%s r_gain = 0x%04x\r\n", __func__, r_gain);
	CDBG("%s g_gain = 0x%04x\r\n", __func__, g_gain);
	CDBG("%s b_gain = 0x%04x\r\n", __func__, b_gain);
	if (r_gain > 0x400) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x5186,
				(r_gain>>8), MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x5187,
				(r_gain&0xff), MSM_CAMERA_I2C_BYTE_DATA);
	}
	if (g_gain > 0x400) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x5188,
				(g_gain>>8), MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x5189,
				(g_gain&0xff), MSM_CAMERA_I2C_BYTE_DATA);
	}
	if (b_gain > 0x400) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x518a,
				(b_gain>>8), MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x518b,
				(b_gain&0xff), MSM_CAMERA_I2C_BYTE_DATA);
	}
}

/**************************************************
 * call this function after OV8825 initialization
 * return value:
 *     0, update success
 *     1, no OTP
 ***************************************************/
uint16_t ov8825_update_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t i;
	uint16_t otp_index;
	uint16_t temp;
	uint16_t r_gain, g_gain, b_gain, g_gain_r, g_gain_b;

	/* R/G and B/G of current camera module is read out from sensor OTP */
	/* check first OTP with valid data */
	for (i = 0; i < 3; i++) {
		temp = ov8825_check_otp_wb(s_ctrl, i);
		if (temp == 2) {
			otp_index = i;
			break;
		}
	}
	if (i == 3) {
		/* no valid wb OTP data */
		CDBG("no valid wb OTP data\r\n");
		return 1;
	}
	ov8825_read_otp_wb(s_ctrl, otp_index, &st_ov8825_otp);
	/* calculate g_gain */
	/* 0x400 = 1x gain */
	if (st_ov8825_otp.bg_ratio < OV8825_BG_RATIO_TYPICAL_VALUE) {
		if (st_ov8825_otp.rg_ratio < OV8825_RG_RATIO_TYPICAL_VALUE) {
			g_gain = 0x400;
			b_gain = 0x400 *
				OV8825_BG_RATIO_TYPICAL_VALUE /
				st_ov8825_otp.bg_ratio;
			r_gain = 0x400 *
				OV8825_RG_RATIO_TYPICAL_VALUE /
				st_ov8825_otp.rg_ratio;
		} else {
			r_gain = 0x400;
			g_gain = 0x400 *
				st_ov8825_otp.rg_ratio /
				OV8825_RG_RATIO_TYPICAL_VALUE;
			b_gain = g_gain *
				OV8825_BG_RATIO_TYPICAL_VALUE /
				st_ov8825_otp.bg_ratio;
		}
	} else {
		if (st_ov8825_otp.rg_ratio < OV8825_RG_RATIO_TYPICAL_VALUE) {
			b_gain = 0x400;
			g_gain = 0x400 *
				st_ov8825_otp.bg_ratio /
				OV8825_BG_RATIO_TYPICAL_VALUE;
			r_gain = g_gain *
				OV8825_RG_RATIO_TYPICAL_VALUE /
				st_ov8825_otp.rg_ratio;
		} else {
			g_gain_b = 0x400 *
				st_ov8825_otp.bg_ratio /
				OV8825_BG_RATIO_TYPICAL_VALUE;
			g_gain_r = 0x400 *
				st_ov8825_otp.rg_ratio /
				OV8825_RG_RATIO_TYPICAL_VALUE;
			if (g_gain_b > g_gain_r) {
				b_gain = 0x400;
				g_gain = g_gain_b;
				r_gain = g_gain *
					OV8825_RG_RATIO_TYPICAL_VALUE /
					st_ov8825_otp.rg_ratio;
			} else {
				r_gain = 0x400;
				g_gain = g_gain_r;
				b_gain = g_gain *
					OV8825_BG_RATIO_TYPICAL_VALUE /
					st_ov8825_otp.bg_ratio;
			}
		}
	}
	ov8825_update_awb_gain(s_ctrl, r_gain, g_gain, b_gain);
	return 0;
}

static int32_t ov8825_write_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line, int32_t luma_avg, uint16_t fgain)
{
	uint32_t offset;
	uint8_t int_time[3];
	uint32_t fl_lines = s_ctrl->curr_frame_length_lines;
	if(s_ctrl->curr_res < MSM_SENSOR_RES_2)
		fl_lines =
			(s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider) / Q10;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (line > (fl_lines - offset))
		fl_lines = line + offset;
	CDBG("ov8825_write_exp_gain: %d %d %d\n", fl_lines, gain, line);
	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
		MSM_CAMERA_I2C_WORD_DATA);
	int_time[0] = line >> 12;
	int_time[1] = line >> 4;
	int_time[2] = line << 4;
	msm_camera_i2c_write_seq(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		&int_time[0], 3);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
		MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	return 0;
}

static int32_t ov8825_write_pic_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line, int32_t luma_avg, uint16_t fgain)
{
	uint32_t fl_lines, offset;
	uint8_t int_time[3];

	fl_lines =
		(s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider) / Q10;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (line > (fl_lines - offset))
		fl_lines = line + offset;
	CDBG("ov8825_write_exp_gain: %d %d %d\n", fl_lines, gain, line);
	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
		MSM_CAMERA_I2C_WORD_DATA);
	int_time[0] = line >> 12;
	int_time[1] = line >> 4;
	int_time[2] = line << 4;
	msm_camera_i2c_write_seq(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		&int_time[0], 3);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
		MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	return 0;
}

static const struct i2c_device_id ov8825_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov8825_s_ctrl},
	{ }
};

static struct i2c_driver ov8825_i2c_driver = {
	.id_table = ov8825_i2c_id,
	.probe  = msm_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov8825_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&ov8825_i2c_driver);
}

static struct v4l2_subdev_core_ops ov8825_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov8825_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov8825_subdev_ops = {
	.core = &ov8825_subdev_core_ops,
	.video  = &ov8825_subdev_video_ops,
};

int32_t ov8825_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;

	CDBG("function:%s,line:%d\n", __func__, __LINE__);

	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
		msm_sensor_enable_debugfs(s_ctrl);
		msm_sensor_write_init_settings(s_ctrl);
		CDBG("Update OTP\n");
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x100, 0x1,
		MSM_CAMERA_I2C_BYTE_DATA);
		msleep(66);
		ov8825_update_otp(s_ctrl);
		usleep_range(10000, 11000);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x100, 0x0,
		MSM_CAMERA_I2C_BYTE_DATA);
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
		msm_sensor_write_res_settings(s_ctrl, res);
		msleep(30);
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
			output_settings[res].op_pixel_clk);
	}
	return rc;
}

static struct msm_sensor_fn_t ov8825_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = ov8825_write_exp_gain,
	.sensor_write_snapshot_exp_gain = ov8825_write_pic_exp_gain,
	.sensor_setting = ov8825_sensor_setting,    /*msm_sensor_setting,*/
	.sensor_csi_setting = msm_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = msm_sensor_power_up,
	.sensor_power_down = msm_sensor_power_down,
	.sensor_adjust_frame_lines = msm_sensor_adjust_frame_lines1,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t ov8825_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov8825_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov8825_start_settings),
	.stop_stream_conf = ov8825_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov8825_stop_settings),
	.group_hold_on_conf = ov8825_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(ov8825_groupon_settings),
	.group_hold_off_conf = ov8825_groupoff_settings,
	.group_hold_off_conf_size =	ARRAY_SIZE(ov8825_groupoff_settings),
	.init_settings = &ov8825_init_conf[0],
	.init_size = ARRAY_SIZE(ov8825_init_conf),
	.mode_settings = &ov8825_confs[0],
	.output_settings = &ov8825_dimensions[0],
	.num_conf = ARRAY_SIZE(ov8825_confs),
};

static struct msm_sensor_ctrl_t ov8825_s_ctrl = {
	.msm_sensor_reg = &ov8825_regs,
	.sensor_i2c_client = &ov8825_sensor_i2c_client,
	.sensor_i2c_addr = 0x6C,
	.sensor_output_reg_addr = &ov8825_reg_addr,
	.sensor_id_info = &ov8825_id_info,
	.sensor_exp_gain_info = &ov8825_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.msm_sensor_mutex = &ov8825_mut,
	.sensor_i2c_driver = &ov8825_i2c_driver,
	.sensor_v4l2_subdev_info = ov8825_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov8825_subdev_info),
	.sensor_v4l2_subdev_ops = &ov8825_subdev_ops,
	.func_tbl = &ov8825_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("OV 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
