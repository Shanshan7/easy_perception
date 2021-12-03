#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

int capture_yuv_init(void);

int capture_yuv(int buffer_id, unsigned char* buffer);

int capture_yuv_close(void);

#ifdef __cplusplus
}
#endif

#endif
