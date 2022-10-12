#include "zetton_stream/util/pixel_format.h"

#include "zetton_common/log/log.h"

namespace zetton {
namespace stream {

const unsigned char uchar_clipping_table[] = {
    0,   0,   0,   0,   0,   0,   0,
    0,  // -128 - -121
    0,   0,   0,   0,   0,   0,   0,
    0,  // -120 - -113
    0,   0,   0,   0,   0,   0,   0,
    0,  // -112 - -105
    0,   0,   0,   0,   0,   0,   0,
    0,  // -104 -  -97
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -96 -  -89
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -88 -  -81
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -80 -  -73
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -72 -  -65
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -64 -  -57
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -56 -  -49
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -48 -  -41
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -40 -  -33
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -32 -  -25
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -24 -  -17
    0,   0,   0,   0,   0,   0,   0,
    0,  //  -16 -   -9
    0,   0,   0,   0,   0,   0,   0,
    0,  //   -8 -   -1
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
    75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
    255, 255, 255, 255, 255, 255, 255, 255, 255,  // 256-263
    255, 255, 255, 255, 255, 255, 255, 255,       // 264-271
    255, 255, 255, 255, 255, 255, 255, 255,       // 272-279
    255, 255, 255, 255, 255, 255, 255, 255,       // 280-287
    255, 255, 255, 255, 255, 255, 255, 255,       // 288-295
    255, 255, 255, 255, 255, 255, 255, 255,       // 296-303
    255, 255, 255, 255, 255, 255, 255, 255,       // 304-311
    255, 255, 255, 255, 255, 255, 255, 255,       // 312-319
    255, 255, 255, 255, 255, 255, 255, 255,       // 320-327
    255, 255, 255, 255, 255, 255, 255, 255,       // 328-335
    255, 255, 255, 255, 255, 255, 255, 255,       // 336-343
    255, 255, 255, 255, 255, 255, 255, 255,       // 344-351
    255, 255, 255, 255, 255, 255, 255, 255,       // 352-359
    255, 255, 255, 255, 255, 255, 255, 255,       // 360-367
    255, 255, 255, 255, 255, 255, 255, 255,       // 368-375
    255, 255, 255, 255, 255, 255, 255, 255,       // 376-383
};
const int clipping_table_offset = 128;

/** Clip a value to the range 0<val<255. For speed this is done using an
 * array, so can only cope with numbers in the range -128<val<383.
 */
unsigned char CLIPVALUE(int val) {
  // Old method (if)
  /*   val = val < 0 ? 0 : val; */
  /*   return val > 255 ? 255 : val; */

  // New method (array)
  return uchar_clipping_table[val + clipping_table_offset];
}

/**
 * Conversion from YUV to RGB.
 * The normal conversion matrix is due to Julien (surname unknown):
 *
 * [ R ]   [  1.0   0.0     1.403 ] [ Y ]
 * [ G ] = [  1.0  -0.344  -0.714 ] [ U ]
 * [ B ]   [  1.0   1.770   0.0   ] [ V ]
 *
 * and the firewire one is similar:
 *
 * [ R ]   [  1.0   0.0     0.700 ] [ Y ]
 * [ G ] = [  1.0  -0.198  -0.291 ] [ U ]
 * [ B ]   [  1.0   1.015   0.0   ] [ V ]
 *
 * Corrected by BJT (coriander's transforms RGB->YUV and YUV->RGB
 *                   do not get you back to the same RGB!)
 * [ R ]   [  1.0   0.0     1.136 ] [ Y ]
 * [ G ] = [  1.0  -0.396  -0.578 ] [ U ]
 * [ B ]   [  1.0   2.041   0.002 ] [ V ]
 *
 */
void YUV2RGB(const unsigned char y, const unsigned char u,
             const unsigned char v, unsigned char* r, unsigned char* g,
             unsigned char* b) {
  const int y2 = (int)y;
  const int u2 = (int)u - 128;
  const int v2 = (int)v - 128;
  // std::cerr << "YUV=("<<y2<<","<<u2<<","<<v2<<")"<<std::endl;

  // This is the normal YUV conversion, but
  // appears to be incorrect for the firewire cameras
  //   int r2 = y2 + ( (v2*91947) >> 16);
  //   int g2 = y2 - ( ((u2*22544) + (v2*46793)) >> 16 );
  //   int b2 = y2 + ( (u2*115999) >> 16);
  // This is an adjusted version (UV spread out a bit)
  int r2 = y2 + ((v2 * 37221) >> 15);
  int g2 = y2 - (((u2 * 12975) + (v2 * 18949)) >> 15);
  int b2 = y2 + ((u2 * 66883) >> 15);
  // std::cerr << "   RGB=("<<r2<<","<<g2<<","<<b2<<")"<<std::endl;

  // Cap the values.
  *r = CLIPVALUE(r2);
  *g = CLIPVALUE(g2);
  *b = CLIPVALUE(b2);
}

void uyvy2yuyv(char* src, int len) {
  unsigned char yuyvbuf[len];
  unsigned char uyvybuf[len];
  memcpy(yuyvbuf, src, len);
  for (int index = 0; index < len; index = index + 2) {
    uyvybuf[index] = yuyvbuf[index + 1];
    uyvybuf[index + 1] = yuyvbuf[index];
  }
  memcpy(src, uyvybuf, len);
}

void uyvy2rgb(char* YUV, char* RGB, int NumPixels) {
  int i, j;
  unsigned char y0, y1, u, v;
  unsigned char r, g, b;
  for (i = 0, j = 0; i < (NumPixels << 1); i += 4, j += 6) {
    u = (unsigned char)YUV[i + 0];
    y0 = (unsigned char)YUV[i + 1];
    v = (unsigned char)YUV[i + 2];
    y1 = (unsigned char)YUV[i + 3];
    YUV2RGB(y0, u, v, &r, &g, &b);
    RGB[j + 0] = r;
    RGB[j + 1] = g;
    RGB[j + 2] = b;
    YUV2RGB(y1, u, v, &r, &g, &b);
    RGB[j + 3] = r;
    RGB[j + 4] = g;
    RGB[j + 5] = b;
  }
}

void mono102mono8(char* RAW, char* MONO, int NumPixels) {
  int i, j;
  for (i = 0, j = 0; i < (NumPixels << 1); i += 2, j += 1) {
    // first byte is low byte, second byte is high byte; smash together and
    // convert to 8-bit
    MONO[j] = (unsigned char)(((RAW[i + 0] >> 2) & 0x3F) |
                              ((RAW[i + 1] << 6) & 0xC0));
  }
}

void yuyv2rgb(char* YUV, char* RGB, int NumPixels) {
  int i, j;
  unsigned char y0, y1, u, v;
  unsigned char r, g, b;

  for (i = 0, j = 0; i < (NumPixels << 1); i += 4, j += 6) {
    y0 = (unsigned char)YUV[i + 0];
    u = (unsigned char)YUV[i + 1];
    y1 = (unsigned char)YUV[i + 2];
    v = (unsigned char)YUV[i + 3];
    YUV2RGB(y0, u, v, &r, &g, &b);
    RGB[j + 0] = r;
    RGB[j + 1] = g;
    RGB[j + 2] = b;
    YUV2RGB(y1, u, v, &r, &g, &b);
    RGB[j + 3] = r;
    RGB[j + 4] = g;
    RGB[j + 5] = b;
  }
}

void rgb242rgb(char* YUV, char* RGB, int NumPixels) {
  memcpy(RGB, YUV, NumPixels * 3);
}

int convert_yuv_to_rgb_pixel(int y, int u, int v) {
  unsigned int pixel32 = 0;
  auto* pixel = (unsigned char*)&pixel32;
  int r, g, b;
  r = (int)((double)y + (1.370705 * ((double)v - 128.0)));
  g = (int)((double)y - (0.698001 * ((double)v - 128.0)) -
            (0.337633 * ((double)u - 128.0)));
  b = (int)((double)y + (1.732446 * ((double)u - 128.0)));
  if (r > 255) r = 255;
  if (g > 255) g = 255;
  if (b > 255) b = 255;
  if (r < 0) r = 0;
  if (g < 0) g = 0;
  if (b < 0) b = 0;
  pixel[0] = (unsigned char)r;
  pixel[1] = (unsigned char)g;
  pixel[2] = (unsigned char)b;
  return pixel32;
}

int convert_yuv_to_rgb_buffer(unsigned char* yuv, unsigned char* rgb,
                              unsigned int width, unsigned int height) {
  unsigned int in, out = 0;
  unsigned int pixel_16;
  unsigned char pixel_24[3];
  unsigned int pixel32;
  int y0, u, y1, v;

  for (in = 0; in < width * height * 2; in += 4) {
    pixel_16 =
        yuv[in + 3] << 24 | yuv[in + 2] << 16 | yuv[in + 1] << 8 | yuv[in + 0];
    y0 = (pixel_16 & 0x000000ff);
    u = (pixel_16 & 0x0000ff00) >> 8;
    y1 = (pixel_16 & 0x00ff0000) >> 16;
    v = (pixel_16 & 0xff000000) >> 24;
    pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
    pixel_24[0] = (unsigned char)(pixel32 & 0x000000ff);
    pixel_24[1] = (unsigned char)((pixel32 & 0x0000ff00) >> 8);
    pixel_24[2] = (unsigned char)((pixel32 & 0x00ff0000) >> 16);
    rgb[out++] = pixel_24[0];
    rgb[out++] = pixel_24[1];
    rgb[out++] = pixel_24[2];
    pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
    pixel_24[0] = (unsigned char)(pixel32 & 0x000000ff);
    pixel_24[1] = (unsigned char)((pixel32 & 0x0000ff00) >> 8);
    pixel_24[2] = (unsigned char)((pixel32 & 0x00ff0000) >> 16);
    rgb[out++] = pixel_24[0];
    rgb[out++] = pixel_24[1];
    rgb[out++] = pixel_24[2];
  }
  return 0;
}

#ifdef WITH_AVX
void print_m256(__m256i a) {
  unsigned char snoop[32];
  bool dst_align = Aligned(reinterpret_cast<void*>(snoop));
  if (dst_align)
    Store<true>(reinterpret_cast<__m256i*>(snoop), a);
  else
    Store<false>(reinterpret_cast<__m256i*>(snoop), a);
  for (int i = 0; i < 32; ++i) {
    printf("DEBUG8 %d %u \n", i, snoop[i]);
  }
}
void print_m256_i32(const __m256i a) {
  unsigned int snoop[8];
  bool dst_align = Aligned(reinterpret_cast<void*>(snoop));
  if (dst_align)
    Store<true>(reinterpret_cast<__m256i*>(snoop), a);
  else
    Store<false>(reinterpret_cast<__m256i*>(snoop), a);
  for (int i = 0; i < 8; ++i) {
    printf("DEBUG32 %d %u \n", i, snoop[i]);
  }
}

void print_m256_i16(const __m256i a) {
  uint16_t snoop[16];
  bool dst_align = Aligned(reinterpret_cast<void*>(snoop));
  if (dst_align)
    Store<true>(reinterpret_cast<__m256i*>(snoop), a);
  else
    Store<false>(reinterpret_cast<__m256i*>(snoop), a);
  for (int i = 0; i < 16; ++i) {
    printf("DEBUG16 %d %u \n", i, snoop[i]);
  }
}

template <bool align>
SIMD_INLINE void yuv_separate_avx2(uint8_t* y, __m256i* y0, __m256i* y1,
                                   __m256i* u0, __m256i* v0) {
  __m256i yuv_m256[4];

  if (align) {
    yuv_m256[0] = Load<true>(reinterpret_cast<__m256i*>(y));
    yuv_m256[1] = Load<true>(reinterpret_cast<__m256i*>(y) + 1);
    yuv_m256[2] = Load<true>(reinterpret_cast<__m256i*>(y) + 2);
    yuv_m256[3] = Load<true>(reinterpret_cast<__m256i*>(y) + 3);
  } else {
    yuv_m256[0] = Load<false>(reinterpret_cast<__m256i*>(y));
    yuv_m256[1] = Load<false>(reinterpret_cast<__m256i*>(y) + 1);
    yuv_m256[2] = Load<false>(reinterpret_cast<__m256i*>(y) + 2);
    yuv_m256[3] = Load<false>(reinterpret_cast<__m256i*>(y) + 3);
  }

  *y0 =
      _mm256_or_si256(_mm256_permute4x64_epi64(
                          _mm256_shuffle_epi8(yuv_m256[0], Y_SHUFFLE0), 0xD8),
                      _mm256_permute4x64_epi64(
                          _mm256_shuffle_epi8(yuv_m256[1], Y_SHUFFLE1), 0xD8));
  *y1 =
      _mm256_or_si256(_mm256_permute4x64_epi64(
                          _mm256_shuffle_epi8(yuv_m256[2], Y_SHUFFLE0), 0xD8),
                      _mm256_permute4x64_epi64(
                          _mm256_shuffle_epi8(yuv_m256[3], Y_SHUFFLE1), 0xD8));

  *u0 = _mm256_permutevar8x32_epi32(
      _mm256_or_si256(
          _mm256_or_si256(_mm256_shuffle_epi8(yuv_m256[0], U_SHUFFLE0),
                          _mm256_shuffle_epi8(yuv_m256[1], U_SHUFFLE1)),
          _mm256_or_si256(_mm256_shuffle_epi8(yuv_m256[2], U_SHUFFLE2),
                          _mm256_shuffle_epi8(yuv_m256[3], U_SHUFFLE3))),
      U_SHUFFLE4);
  *v0 = _mm256_permutevar8x32_epi32(
      _mm256_or_si256(
          _mm256_or_si256(_mm256_shuffle_epi8(yuv_m256[0], V_SHUFFLE0),
                          _mm256_shuffle_epi8(yuv_m256[1], V_SHUFFLE1)),
          _mm256_or_si256(_mm256_shuffle_epi8(yuv_m256[2], V_SHUFFLE2),
                          _mm256_shuffle_epi8(yuv_m256[3], V_SHUFFLE3))),
      U_SHUFFLE4);
}

template <bool align>
void yuv2rgb_avx2(__m256i y0, __m256i u0, __m256i v0, uint8_t* rgb) {
  __m256i r0 = YuvToRed(y0, v0);
  __m256i g0 = YuvToGreen(y0, u0, v0);
  __m256i b0 = YuvToBlue(y0, u0);

  Store<align>(reinterpret_cast<__m256i*>(rgb) + 0,
               InterleaveBgr<0>(r0, g0, b0));
  Store<align>(reinterpret_cast<__m256i*>(rgb) + 1,
               InterleaveBgr<1>(r0, g0, b0));
  Store<align>(reinterpret_cast<__m256i*>(rgb) + 2,
               InterleaveBgr<2>(r0, g0, b0));
}

template <bool align>
void yuv2rgb_avx2(uint8_t* yuv, uint8_t* rgb) {
  __m256i y0, y1, u0, v0;

  yuv_separate_avx2<align>(yuv, &y0, &y1, &u0, &v0);
  __m256i u0_u0 = _mm256_permute4x64_epi64(u0, 0xD8);
  __m256i v0_v0 = _mm256_permute4x64_epi64(v0, 0xD8);
  yuv2rgb_avx2<align>(y0, _mm256_unpacklo_epi8(u0_u0, u0_u0),
                      _mm256_unpacklo_epi8(v0_v0, v0_v0), rgb);
  yuv2rgb_avx2<align>(y1, _mm256_unpackhi_epi8(u0_u0, u0_u0),
                      _mm256_unpackhi_epi8(v0_v0, v0_v0),
                      rgb + 3 * sizeof(__m256i));
}

void yuyv2rgb_avx(unsigned char* YUV, unsigned char* RGB, int NumPixels) {
  // ACHECK(NumPixels == (1920 * 1080));
  bool align = Aligned(YUV) & Aligned(RGB);
  uint8_t* yuv_offset = YUV;
  uint8_t* rgb_offset = RGB;
  if (align) {
    for (int i = 0; i < NumPixels;
         i = i + static_cast<int>(2 * sizeof(__m256i)),
             yuv_offset += 4 * static_cast<int>(sizeof(__m256i)),
             rgb_offset += 6 * static_cast<int>(sizeof(__m256i))) {
      yuv2rgb_avx2<true>(yuv_offset, rgb_offset);
    }
  } else {
    for (int i = 0; i < NumPixels;
         i = i + static_cast<int>(2 * sizeof(__m256i)),
             yuv_offset += 4 * static_cast<int>(sizeof(__m256i)),
             rgb_offset += 6 * static_cast<int>(sizeof(__m256i))) {
      yuv2rgb_avx2<false>(yuv_offset, rgb_offset);
    }
  }
}
#endif

}  // namespace stream
}  // namespace zetton
