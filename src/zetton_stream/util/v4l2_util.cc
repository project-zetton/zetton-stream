#include "zetton_stream/util/v4l2_util.h"

#include "zetton_common/log/log.h"

namespace zetton {
namespace stream {

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
