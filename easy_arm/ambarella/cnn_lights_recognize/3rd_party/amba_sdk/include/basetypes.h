/*************************************************************************************
 Copyright (c) 2018-2022, Unisinsight Technologies Co., Ltd. All rights reserved.
-------------------------------------------------------------------------------------
                            basetypes.h
   Project Code: 基本数据类型定义
   Module Name :
   Date Created: 2021-2-4
   Author      :  
   Description : 基本数据类型定义
**************************************************************************************/
#ifndef __BASE_TYPES_H__
#define __BASE_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char       u8; 
typedef unsigned short     u16; 
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef signed char         s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;

#define AMBA_API __attribute__((visibility("default")))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))
#endif

struct iav_window {
	u32 width;  
	u32 height; 
};

struct iav_offset {
	u32 x; 
	u32 y; 
};

struct iav_rect {
	u32 x;      
	u32 y;      
	u32 width;  
	u32 height; 
};

#define rect_init(_x, _y, _w, _h)(	\
{				\
	struct iav_rect _r;		\
	_r.x = (_x); _r.y = (_y); _r.width = (_w); _r.height = (_h);	\
	_r;			\
})

#define win_init(w, h)		rect_init(0, 0, w, h)

#define win_invalid(w)		((w).width == 0 || (w).height == 0)

#define rect_invalid(w)		(win_invalid(w) || (w).x > 0xffff || (w).y > 0xffff)

#define rect_contain(r1, r2) ({			\
	u32 _x1 = (r1).x, _y1 = (r1).y, _w1 = (r1).width, _h1 = (r1).height; \
	u32 _x2 = (r2).x, _y2 = (r2).y, _w2 = (r2).width, _h2 = (r2).height; \
	_x1 <= _x2 && (_x1 + _w1) >= (_x2 + _w2) &&		\
	_y1 <= _y2 && (_y1 + _h1) >= (_y2 + _h2) ? 1 : 0;	\
	 })
     
static inline void iav_set_bit(u32 *bit_map, int nr)
{
	*bit_map |= (1 << nr);
}

static inline int iav_test_bit(u32 *bit_map, int nr)
{
	return !!((*bit_map) & (1 << nr));
}

#ifdef __cplusplus
}
#endif

#endif
