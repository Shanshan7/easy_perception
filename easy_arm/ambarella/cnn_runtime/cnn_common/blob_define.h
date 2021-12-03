#ifndef BLOBDEFINE_H
#define BLOBDEFINE_H

/**
 * @brief submodule to operate array data
 *
 */
#define BLOB_DATA(a, K, H, W, n, k, h, w) (a)[(( (n) * (K) + (k) ) * (H) + (h) ) * (W) + (w)]
#define DIM2_DATA(a, W, h, w) (a)[(h) * (W) + (w)]
#define DIM1_DATA(a, i) (a)[(i)]


#define ROUND_UP_32(x) ((x)&0x1f ? (((x)&0xffffffe0) + 32) : (x))

#define LAYER_P(w) (ROUND_UP_32(4 * w))/4

#endif /* BLOBDEFINE_H */