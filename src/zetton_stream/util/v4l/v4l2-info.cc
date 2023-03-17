// SPDX-License-Identifier: LGPL-2.1-only
/*
 * Copyright 2018 Cisco Systems, Inc. and/or its affiliates. All rights
 * reserved.
 */

#include "zetton_stream/util/v4l/v4l2-info.h"

static std::string num2s(unsigned num, bool is_hex = true) {
  char buf[16];

  if (is_hex)
    sprintf(buf, "0x%08x", num);
  else
    sprintf(buf, "%u", num);
  return buf;
}

std::string flags2s(unsigned val, const flag_def *def) {
  std::string s;

  while (def->flag) {
    if (val & def->flag) {
      if (s.length()) s += ", ";
      s += def->str;
      val &= ~def->flag;
    }
    def++;
  }
  if (val) {
    if (s.length()) s += ", ";
    s += num2s(val);
  }
  return s;
}

static std::string cap2s(unsigned cap) {
  std::string s;

  if (cap & V4L2_CAP_VIDEO_CAPTURE) s += "\t\tVideo Capture\n";
  if (cap & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
    s += "\t\tVideo Capture Multiplanar\n";
  if (cap & V4L2_CAP_VIDEO_OUTPUT) s += "\t\tVideo Output\n";
  if (cap & V4L2_CAP_VIDEO_OUTPUT_MPLANE) s += "\t\tVideo Output Multiplanar\n";
  if (cap & V4L2_CAP_VIDEO_M2M) s += "\t\tVideo Memory-to-Memory\n";
  if (cap & V4L2_CAP_VIDEO_M2M_MPLANE)
    s += "\t\tVideo Memory-to-Memory Multiplanar\n";
  if (cap & V4L2_CAP_VIDEO_OVERLAY) s += "\t\tVideo Overlay\n";
  if (cap & V4L2_CAP_VIDEO_OUTPUT_OVERLAY) s += "\t\tVideo Output Overlay\n";
  if (cap & V4L2_CAP_VBI_CAPTURE) s += "\t\tVBI Capture\n";
  if (cap & V4L2_CAP_VBI_OUTPUT) s += "\t\tVBI Output\n";
  if (cap & V4L2_CAP_SLICED_VBI_CAPTURE) s += "\t\tSliced VBI Capture\n";
  if (cap & V4L2_CAP_SLICED_VBI_OUTPUT) s += "\t\tSliced VBI Output\n";
  if (cap & V4L2_CAP_RDS_CAPTURE) s += "\t\tRDS Capture\n";
  if (cap & V4L2_CAP_RDS_OUTPUT) s += "\t\tRDS Output\n";
  if (cap & V4L2_CAP_SDR_CAPTURE) s += "\t\tSDR Capture\n";
  if (cap & V4L2_CAP_SDR_OUTPUT) s += "\t\tSDR Output\n";
  if (cap & V4L2_CAP_META_CAPTURE) s += "\t\tMetadata Capture\n";
  // if (cap & V4L2_CAP_META_OUTPUT) s += "\t\tMetadata Output\n";
  if (cap & V4L2_CAP_TUNER) s += "\t\tTuner\n";
  if (cap & V4L2_CAP_TOUCH) s += "\t\tTouch Device\n";
  if (cap & V4L2_CAP_HW_FREQ_SEEK) s += "\t\tHW Frequency Seek\n";
  if (cap & V4L2_CAP_MODULATOR) s += "\t\tModulator\n";
  if (cap & V4L2_CAP_AUDIO) s += "\t\tAudio\n";
  if (cap & V4L2_CAP_RADIO) s += "\t\tRadio\n";
  // if (cap & V4L2_CAP_IO_MC) s += "\t\tI/O MC\n";
  if (cap & V4L2_CAP_READWRITE) s += "\t\tRead/Write\n";
  if (cap & V4L2_CAP_STREAMING) s += "\t\tStreaming\n";
  if (cap & V4L2_CAP_EXT_PIX_FORMAT) s += "\t\tExtended Pix Format\n";
  if (cap & V4L2_CAP_DEVICE_CAPS) s += "\t\tDevice Capabilities\n";
  return s;
}

// static std::string subdevcap2s(unsigned cap) {
//   std::string s;

//   if (cap & V4L2_SUBDEV_CAP_RO_SUBDEV) s += "\t\tRead-Only Sub-Device\n";
//   if (cap & V4L2_SUBDEV_CAP_STREAMS) s += "\t\tStreams Support\n";
//   return s;
// }

void v4l2_info_capability(const v4l2_capability &vcap) {
  printf("\tDriver name      : %s\n", vcap.driver);
  printf("\tCard type        : %s\n", vcap.card);
  printf("\tBus info         : %s\n", vcap.bus_info);
  printf("\tDriver version   : %d.%d.%d\n", vcap.version >> 16,
         (vcap.version >> 8) & 0xff, vcap.version & 0xff);
  printf("\tCapabilities     : 0x%08x\n", vcap.capabilities);
  printf("%s", cap2s(vcap.capabilities).c_str());
  if (vcap.capabilities & V4L2_CAP_DEVICE_CAPS) {
    printf("\tDevice Caps      : 0x%08x\n", vcap.device_caps);
    printf("%s", cap2s(vcap.device_caps).c_str());
  }
}

// void v4l2_info_subdev_capability(const v4l2_subdev_capability &subdevcap) {
//   printf("\tDriver version   : %d.%d.%d\n", subdevcap.version >> 16,
//          (subdevcap.version >> 8) & 0xff, subdevcap.version & 0xff);
//   printf("\tCapabilities     : 0x%08x\n", subdevcap.capabilities);
//   printf("%s", subdevcap2s(subdevcap.capabilities).c_str());
// }

std::string fcc2s(__u32 val) {
  std::string s;

  s += val & 0x7f;
  s += (val >> 8) & 0x7f;
  s += (val >> 16) & 0x7f;
  s += (val >> 24) & 0x7f;
  if (val & (1U << 31)) s += "-BE";
  return s;
}

std::string pixfmt2s(__u32 format) {
  switch (format) {
    case V4L2_PIX_FMT_RGB332:
      return "8-bit RGB 3-3-2";
    case V4L2_PIX_FMT_RGB444:
      return "16-bit A/XRGB 4-4-4-4";
    case V4L2_PIX_FMT_ARGB444:
      return "16-bit ARGB 4-4-4-4";
    case V4L2_PIX_FMT_XRGB444:
      return "16-bit XRGB 4-4-4-4";
    // case V4L2_PIX_FMT_RGBA444:
    //   return "16-bit RGBA 4-4-4-4";
    // case V4L2_PIX_FMT_RGBX444:
    //   return "16-bit RGBX 4-4-4-4";
    // case V4L2_PIX_FMT_ABGR444:
    //   return "16-bit ABGR 4-4-4-4";
    // case V4L2_PIX_FMT_XBGR444:
    //   return "16-bit XBGR 4-4-4-4";
    // case V4L2_PIX_FMT_BGRA444:
    //   return "16-bit BGRA 4-4-4-4";
    // case V4L2_PIX_FMT_BGRX444:
    //   return "16-bit BGRX 4-4-4-4";
    case V4L2_PIX_FMT_RGB555:
      return "16-bit A/XRGB 1-5-5-5";
    case V4L2_PIX_FMT_ARGB555:
      return "16-bit ARGB 1-5-5-5";
    case V4L2_PIX_FMT_XRGB555:
      return "16-bit XRGB 1-5-5-5";
    // case V4L2_PIX_FMT_ABGR555:
    //   return "16-bit ABGR 1-5-5-5";
    // case V4L2_PIX_FMT_XBGR555:
    //   return "16-bit XBGR 1-5-5-5";
    // case V4L2_PIX_FMT_RGBA555:
    //   return "16-bit RGBA 5-5-5-1";
    // case V4L2_PIX_FMT_RGBX555:
    //   return "16-bit RGBX 5-5-5-1";
    // case V4L2_PIX_FMT_BGRA555:
    //   return "16-bit BGRA 5-5-5-1";
    // case V4L2_PIX_FMT_BGRX555:
    //   return "16-bit BGRX 5-5-5-1";
    case V4L2_PIX_FMT_RGB565:
      return "16-bit RGB 5-6-5";
    case V4L2_PIX_FMT_RGB555X:
      return "16-bit A/XRGB 1-5-5-5 BE";
    case V4L2_PIX_FMT_ARGB555X:
      return "16-bit ARGB 1-5-5-5 BE";
    case V4L2_PIX_FMT_XRGB555X:
      return "16-bit XRGB 1-5-5-5 BE";
    case V4L2_PIX_FMT_RGB565X:
      return "16-bit RGB 5-6-5 BE";
    case V4L2_PIX_FMT_BGR666:
      return "18-bit BGRX 6-6-6-14";
    case V4L2_PIX_FMT_BGR24:
      return "24-bit BGR 8-8-8";
    case V4L2_PIX_FMT_RGB24:
      return "24-bit RGB 8-8-8";
    case V4L2_PIX_FMT_BGR32:
      return "32-bit BGRA/X 8-8-8-8";
    case V4L2_PIX_FMT_ABGR32:
      return "32-bit BGRA 8-8-8-8";
    case V4L2_PIX_FMT_XBGR32:
      return "32-bit BGRX 8-8-8-8";
    case V4L2_PIX_FMT_RGB32:
      return "32-bit A/XRGB 8-8-8-8";
    case V4L2_PIX_FMT_ARGB32:
      return "32-bit ARGB 8-8-8-8";
    case V4L2_PIX_FMT_XRGB32:
      return "32-bit XRGB 8-8-8-8";
    // case V4L2_PIX_FMT_BGRA32:
    //   return "32-bit ABGR 8-8-8-8";
    // case V4L2_PIX_FMT_BGRX32:
    //   return "32-bit XBGR 8-8-8-8";
    // case V4L2_PIX_FMT_RGBA32:
    //   return "32-bit RGBA 8-8-8-8";
    // case V4L2_PIX_FMT_RGBX32:
    //   return "32-bit RGBX 8-8-8-8";
    case V4L2_PIX_FMT_GREY:
      return "8-bit Greyscale";
    case V4L2_PIX_FMT_Y4:
      return "4-bit Greyscale";
    case V4L2_PIX_FMT_Y6:
      return "6-bit Greyscale";
    case V4L2_PIX_FMT_Y10:
      return "10-bit Greyscale";
    case V4L2_PIX_FMT_Y12:
      return "12-bit Greyscale";
    // case V4L2_PIX_FMT_Y14:
    //   return "14-bit Greyscale";
    case V4L2_PIX_FMT_Y16:
      return "16-bit Greyscale";
    case V4L2_PIX_FMT_Y16_BE:
      return "16-bit Greyscale BE";
    case V4L2_PIX_FMT_Y10BPACK:
      return "10-bit Greyscale (Packed)";
    // case V4L2_PIX_FMT_Y10P:
    //   return "10-bit Greyscale (MIPI Packed)";
    // case V4L2_PIX_FMT_IPU3_Y10:
    //   return "10-bit greyscale (IPU3 Packed)";
    case V4L2_PIX_FMT_Y8I:
      return "Interleaved 8-bit Greyscale";
    case V4L2_PIX_FMT_Y12I:
      return "Interleaved 12-bit Greyscale";
    case V4L2_PIX_FMT_Z16:
      return "16-bit Depth";
    case V4L2_PIX_FMT_INZI:
      return "Planar 10:16 Greyscale Depth";
    // case V4L2_PIX_FMT_CNF4:
    //   return "4-bit Depth Confidence (Packed)";
    case V4L2_PIX_FMT_PAL8:
      return "8-bit Palette";
    case V4L2_PIX_FMT_UV8:
      return "8-bit Chrominance UV 4-4";
    case V4L2_PIX_FMT_YVU410:
      return "Planar YVU 4:1:0";
    case V4L2_PIX_FMT_YVU420:
      return "Planar YVU 4:2:0";
    case V4L2_PIX_FMT_YUYV:
      return "YUYV 4:2:2";
    case V4L2_PIX_FMT_YYUV:
      return "YYUV 4:2:2";
    case V4L2_PIX_FMT_YVYU:
      return "YVYU 4:2:2";
    case V4L2_PIX_FMT_UYVY:
      return "UYVY 4:2:2";
    case V4L2_PIX_FMT_VYUY:
      return "VYUY 4:2:2";
    case V4L2_PIX_FMT_YUV422P:
      return "Planar YUV 4:2:2";
    case V4L2_PIX_FMT_YUV411P:
      return "Planar YUV 4:1:1";
    case V4L2_PIX_FMT_Y41P:
      return "YUV 4:1:1 (Packed)";
    case V4L2_PIX_FMT_YUV444:
      return "16-bit A/XYUV 4-4-4-4";
    case V4L2_PIX_FMT_YUV555:
      return "16-bit A/XYUV 1-5-5-5";
    case V4L2_PIX_FMT_YUV565:
      return "16-bit YUV 5-6-5";
    // case V4L2_PIX_FMT_YUV24:
    //   return "24-bit YUV 4:4:4 8-8-8";
    case V4L2_PIX_FMT_YUV32:
      return "32-bit A/XYUV 8-8-8-8";
    // case V4L2_PIX_FMT_AYUV32:
    //   return "32-bit AYUV 8-8-8-8";
    // case V4L2_PIX_FMT_XYUV32:
    //   return "32-bit XYUV 8-8-8-8";
    // case V4L2_PIX_FMT_VUYA32:
    //   return "32-bit VUYA 8-8-8-8";
    // case V4L2_PIX_FMT_VUYX32:
    //   return "32-bit VUYX 8-8-8-8";
    // case V4L2_PIX_FMT_YUVA32:
    //   return "32-bit YUVA 8-8-8-8";
    // case V4L2_PIX_FMT_YUVX32:
    //   return "32-bit YUVX 8-8-8-8";
    case V4L2_PIX_FMT_YUV410:
      return "Planar YUV 4:1:0";
    case V4L2_PIX_FMT_YUV420:
      return "Planar YUV 4:2:0";
    case V4L2_PIX_FMT_HI240:
      return "8-bit Dithered RGB (BTTV)";
    case V4L2_PIX_FMT_M420:
      return "YUV 4:2:0 (M420)";
    case V4L2_PIX_FMT_NV12:
      return "Y/UV 4:2:0";
    case V4L2_PIX_FMT_NV21:
      return "Y/VU 4:2:0";
    case V4L2_PIX_FMT_NV16:
      return "Y/UV 4:2:2";
    case V4L2_PIX_FMT_NV61:
      return "Y/VU 4:2:2";
    case V4L2_PIX_FMT_NV24:
      return "Y/UV 4:4:4";
    case V4L2_PIX_FMT_NV42:
      return "Y/VU 4:4:4";
    // case V4L2_PIX_FMT_P010:
    //   return "10-bit Y/UV 4:2:0";
    // case V4L2_PIX_FMT_NV12_4L4:
    //   return "Y/UV 4:2:0 (4x4 Linear)";
    // case V4L2_PIX_FMT_NV12_16L16:
    //   return "Y/UV 4:2:0 (16x16 Linear)";
    // case V4L2_PIX_FMT_NV12_32L32:
    //   return "Y/UV 4:2:0 (32x32 Linear)";
    // case V4L2_PIX_FMT_P010_4L4:
    //   return "10-bit Y/UV 4:2:0 (4x4 Linear)";
    case V4L2_PIX_FMT_NV12M:
      return "Y/UV 4:2:0 (N-C)";
    case V4L2_PIX_FMT_NV21M:
      return "Y/VU 4:2:0 (N-C)";
    case V4L2_PIX_FMT_NV16M:
      return "Y/UV 4:2:2 (N-C)";
    case V4L2_PIX_FMT_NV61M:
      return "Y/VU 4:2:2 (N-C)";
    case V4L2_PIX_FMT_NV12MT:
      return "Y/UV 4:2:0 (64x32 MB, N-C)";
    case V4L2_PIX_FMT_NV12MT_16X16:
      return "Y/UV 4:2:0 (16x16 MB, N-C)";
    case V4L2_PIX_FMT_YUV420M:
      return "Planar YUV 4:2:0 (N-C)";
    case V4L2_PIX_FMT_YVU420M:
      return "Planar YVU 4:2:0 (N-C)";
    case V4L2_PIX_FMT_YUV422M:
      return "Planar YUV 4:2:2 (N-C)";
    case V4L2_PIX_FMT_YVU422M:
      return "Planar YVU 4:2:2 (N-C)";
    case V4L2_PIX_FMT_YUV444M:
      return "Planar YUV 4:4:4 (N-C)";
    case V4L2_PIX_FMT_YVU444M:
      return "Planar YVU 4:4:4 (N-C)";
    case V4L2_PIX_FMT_SBGGR8:
      return "8-bit Bayer BGBG/GRGR";
    case V4L2_PIX_FMT_SGBRG8:
      return "8-bit Bayer GBGB/RGRG";
    case V4L2_PIX_FMT_SGRBG8:
      return "8-bit Bayer GRGR/BGBG";
    case V4L2_PIX_FMT_SRGGB8:
      return "8-bit Bayer RGRG/GBGB";
    case V4L2_PIX_FMT_SBGGR10:
      return "10-bit Bayer BGBG/GRGR";
    case V4L2_PIX_FMT_SGBRG10:
      return "10-bit Bayer GBGB/RGRG";
    case V4L2_PIX_FMT_SGRBG10:
      return "10-bit Bayer GRGR/BGBG";
    case V4L2_PIX_FMT_SRGGB10:
      return "10-bit Bayer RGRG/GBGB";
    case V4L2_PIX_FMT_SBGGR10P:
      return "10-bit Bayer BGBG/GRGR Packed";
    case V4L2_PIX_FMT_SGBRG10P:
      return "10-bit Bayer GBGB/RGRG Packed";
    case V4L2_PIX_FMT_SGRBG10P:
      return "10-bit Bayer GRGR/BGBG Packed";
    case V4L2_PIX_FMT_SRGGB10P:
      return "10-bit Bayer RGRG/GBGB Packed";
    // case V4L2_PIX_FMT_IPU3_SBGGR10:
    //   return "10-bit bayer BGGR IPU3 Packed";
    // case V4L2_PIX_FMT_IPU3_SGBRG10:
    //   return "10-bit bayer GBRG IPU3 Packed";
    // case V4L2_PIX_FMT_IPU3_SGRBG10:
    //   return "10-bit bayer GRBG IPU3 Packed";
    // case V4L2_PIX_FMT_IPU3_SRGGB10:
    //   return "10-bit bayer RGGB IPU3 Packed";
    case V4L2_PIX_FMT_SBGGR10ALAW8:
      return "8-bit Bayer BGBG/GRGR (A-law)";
    case V4L2_PIX_FMT_SGBRG10ALAW8:
      return "8-bit Bayer GBGB/RGRG (A-law)";
    case V4L2_PIX_FMT_SGRBG10ALAW8:
      return "8-bit Bayer GRGR/BGBG (A-law)";
    case V4L2_PIX_FMT_SRGGB10ALAW8:
      return "8-bit Bayer RGRG/GBGB (A-law)";
    case V4L2_PIX_FMT_SBGGR10DPCM8:
      return "8-bit Bayer BGBG/GRGR (DPCM)";
    case V4L2_PIX_FMT_SGBRG10DPCM8:
      return "8-bit Bayer GBGB/RGRG (DPCM)";
    case V4L2_PIX_FMT_SGRBG10DPCM8:
      return "8-bit Bayer GRGR/BGBG (DPCM)";
    case V4L2_PIX_FMT_SRGGB10DPCM8:
      return "8-bit Bayer RGRG/GBGB (DPCM)";
    case V4L2_PIX_FMT_SBGGR12:
      return "12-bit Bayer BGBG/GRGR";
    case V4L2_PIX_FMT_SGBRG12:
      return "12-bit Bayer GBGB/RGRG";
    case V4L2_PIX_FMT_SGRBG12:
      return "12-bit Bayer GRGR/BGBG";
    case V4L2_PIX_FMT_SRGGB12:
      return "12-bit Bayer RGRG/GBGB";
    case V4L2_PIX_FMT_SBGGR12P:
      return "12-bit Bayer BGBG/GRGR Packed";
    case V4L2_PIX_FMT_SGBRG12P:
      return "12-bit Bayer GBGB/RGRG Packed";
    case V4L2_PIX_FMT_SGRBG12P:
      return "12-bit Bayer GRGR/BGBG Packed";
    case V4L2_PIX_FMT_SRGGB12P:
      return "12-bit Bayer RGRG/GBGB Packed";
    // case V4L2_PIX_FMT_SBGGR14:
    //   return "14-bit Bayer BGBG/GRGR";
    // case V4L2_PIX_FMT_SGBRG14:
    //   return "14-bit Bayer GBGB/RGRG";
    // case V4L2_PIX_FMT_SGRBG14:
    //   return "14-bit Bayer GRGR/BGBG";
    // case V4L2_PIX_FMT_SRGGB14:
    //   return "14-bit Bayer RGRG/GBGB";
    // case V4L2_PIX_FMT_SBGGR14P:
    //   return "14-bit Bayer BGBG/GRGR Packed";
    // case V4L2_PIX_FMT_SGBRG14P:
    //   return "14-bit Bayer GBGB/RGRG Packed";
    // case V4L2_PIX_FMT_SGRBG14P:
    //   return "14-bit Bayer GRGR/BGBG Packed";
    // case V4L2_PIX_FMT_SRGGB14P:
    //   return "14-bit Bayer RGRG/GBGB Packed";
    case V4L2_PIX_FMT_SBGGR16:
      return "16-bit Bayer BGBG/GRGR";
    case V4L2_PIX_FMT_SGBRG16:
      return "16-bit Bayer GBGB/RGRG";
    case V4L2_PIX_FMT_SGRBG16:
      return "16-bit Bayer GRGR/BGBG";
    case V4L2_PIX_FMT_SRGGB16:
      return "16-bit Bayer RGRG/GBGB";
    case V4L2_PIX_FMT_SN9C20X_I420:
      return "GSPCA SN9C20X I420";
    case V4L2_PIX_FMT_SPCA501:
      return "GSPCA SPCA501";
    case V4L2_PIX_FMT_SPCA505:
      return "GSPCA SPCA505";
    case V4L2_PIX_FMT_SPCA508:
      return "GSPCA SPCA508";
    case V4L2_PIX_FMT_STV0680:
      return "GSPCA STV0680";
    case V4L2_PIX_FMT_TM6000:
      return "A/V + VBI Mux Packet";
    case V4L2_PIX_FMT_CIT_YYVYUY:
      return "GSPCA CIT YYVYUY";
    case V4L2_PIX_FMT_KONICA420:
      return "GSPCA KONICA420";
    // case V4L2_PIX_FMT_MM21:
    //   return "Mediatek 8-bit Block Format";
    case V4L2_PIX_FMT_HSV24:
      return "24-bit HSV 8-8-8";
    case V4L2_PIX_FMT_HSV32:
      return "32-bit XHSV 8-8-8-8";
    case V4L2_SDR_FMT_CU8:
      return "Complex U8";
    case V4L2_SDR_FMT_CU16LE:
      return "Complex U16LE";
    case V4L2_SDR_FMT_CS8:
      return "Complex S8";
    case V4L2_SDR_FMT_CS14LE:
      return "Complex S14LE";
    case V4L2_SDR_FMT_RU12LE:
      return "Real U12LE";
    case V4L2_SDR_FMT_PCU16BE:
      return "Planar Complex U16BE";
    case V4L2_SDR_FMT_PCU18BE:
      return "Planar Complex U18BE";
    case V4L2_SDR_FMT_PCU20BE:
      return "Planar Complex U20BE";
    case V4L2_TCH_FMT_DELTA_TD16:
      return "16-bit Signed Deltas";
    case V4L2_TCH_FMT_DELTA_TD08:
      return "8-bit Signed Deltas";
    case V4L2_TCH_FMT_TU16:
      return "16-bit Unsigned Touch Data";
    case V4L2_TCH_FMT_TU08:
      return "8-bit Unsigned Touch Data";
    case V4L2_META_FMT_VSP1_HGO:
      return "R-Car VSP1 1-D Histogram";
    case V4L2_META_FMT_VSP1_HGT:
      return "R-Car VSP1 2-D Histogram";
    // case V4L2_META_FMT_UVC:
    //   return "UVC Payload Header Metadata";
    // case V4L2_META_FMT_D4XX:
    //   return "Intel D4xx UVC Metadata";
    // case V4L2_META_FMT_VIVID:
    //   return "Vivid Metadata";
    // case V4L2_META_FMT_RK_ISP1_PARAMS:
    //   return "Rockchip ISP1 3A Parameters";
    // case V4L2_META_FMT_RK_ISP1_STAT_3A:
    //   return "Rockchip ISP1 3A Statistics";
    // case V4L2_PIX_FMT_NV12_8L128:
    //   return "NV12 (8x128 Linear)";
    // case V4L2_PIX_FMT_NV12M_8L128:
    //   return "NV12M (8x128 Linear)";
    // case V4L2_PIX_FMT_NV12_10BE_8L128:
    //   return "10-bit NV12 (8x128 Linear, BE)";
    // case V4L2_PIX_FMT_NV12M_10BE_8L128:
    //   return "10-bit NV12M (8x128 Linear, BE)";
    case V4L2_PIX_FMT_MJPEG:
      return "Motion-JPEG";
    case V4L2_PIX_FMT_JPEG:
      return "JFIF JPEG";
    case V4L2_PIX_FMT_DV:
      return "1394";
    case V4L2_PIX_FMT_MPEG:
      return "MPEG-1/2/4";
    case V4L2_PIX_FMT_H264:
      return "H.264";
    case V4L2_PIX_FMT_H264_NO_SC:
      return "H.264 (No Start Codes)";
    case V4L2_PIX_FMT_H264_MVC:
      return "H.264 MVC";
    // case V4L2_PIX_FMT_H264_SLICE:
    //   return "H.264 Parsed Slice Data";
    case V4L2_PIX_FMT_H263:
      return "H.263";
    case V4L2_PIX_FMT_MPEG1:
      return "MPEG-1 ES";
    case V4L2_PIX_FMT_MPEG2:
      return "MPEG-2 ES";
    // case V4L2_PIX_FMT_MPEG2_SLICE:
    //   return "MPEG-2 Parsed Slice Data";
    case V4L2_PIX_FMT_MPEG4:
      return "MPEG-4 Part 2 ES";
    case V4L2_PIX_FMT_XVID:
      return "Xvid";
    case V4L2_PIX_FMT_VC1_ANNEX_G:
      return "VC-1 (SMPTE 412M Annex G)";
    case V4L2_PIX_FMT_VC1_ANNEX_L:
      return "VC-1 (SMPTE 412M Annex L)";
    case V4L2_PIX_FMT_VP8:
      return "VP8";
    // case V4L2_PIX_FMT_VP8_FRAME:
    //   return "VP8 Frame";
    case V4L2_PIX_FMT_VP9:
      return "VP9";
    // case V4L2_PIX_FMT_VP9_FRAME:
    //   return "VP9 Frame";
    case V4L2_PIX_FMT_HEVC:
      return "HEVC";
    // case V4L2_PIX_FMT_FWHT:
    //   return "FWHT";
    // case V4L2_PIX_FMT_FWHT_STATELESS:
    //   return "FWHT Stateless";
    case V4L2_PIX_FMT_CPIA1:
      return "GSPCA CPiA YUV";
    case V4L2_PIX_FMT_WNVA:
      return "WNVA";
    case V4L2_PIX_FMT_SN9C10X:
      return "GSPCA SN9C10X";
    case V4L2_PIX_FMT_PWC1:
      return "Raw Philips Webcam Type (Old)";
    case V4L2_PIX_FMT_PWC2:
      return "Raw Philips Webcam Type (New)";
    case V4L2_PIX_FMT_ET61X251:
      return "GSPCA ET61X251";
    case V4L2_PIX_FMT_SPCA561:
      return "GSPCA SPCA561";
    case V4L2_PIX_FMT_PAC207:
      return "GSPCA PAC207";
    case V4L2_PIX_FMT_MR97310A:
      return "GSPCA MR97310A";
    case V4L2_PIX_FMT_JL2005BCD:
      return "GSPCA JL2005BCD";
    case V4L2_PIX_FMT_SN9C2028:
      return "GSPCA SN9C2028";
    case V4L2_PIX_FMT_SQ905C:
      return "GSPCA SQ905C";
    case V4L2_PIX_FMT_PJPG:
      return "GSPCA PJPG";
    case V4L2_PIX_FMT_OV511:
      return "GSPCA OV511";
    case V4L2_PIX_FMT_OV518:
      return "GSPCA OV518";
    case V4L2_PIX_FMT_JPGL:
      return "JPEG Lite";
    case V4L2_PIX_FMT_SE401:
      return "GSPCA SE401";
    case V4L2_PIX_FMT_S5C_UYVY_JPG:
      return "S5C73MX interleaved UYVY/JPEG";
    case V4L2_PIX_FMT_MT21C:
      return "Mediatek Compressed Format";
    // case V4L2_PIX_FMT_QC08C:
    //   return "QCOM Compressed 8-bit Format";
    // case V4L2_PIX_FMT_QC10C:
    //   return "QCOM Compressed 10-bit Format";
    // case V4L2_PIX_FMT_AJPG:
    //   return "Aspeed JPEG";
    default:
      return std::string("Unknown (") + num2s(format) + ")";
  }
}

std::string buftype2s(int type) {
  switch (type) {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
      return "Video Capture";
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
      return "Video Capture Multiplanar";
    case V4L2_BUF_TYPE_VIDEO_OUTPUT:
      return "Video Output";
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
      return "Video Output Multiplanar";
    case V4L2_BUF_TYPE_VIDEO_OVERLAY:
      return "Video Overlay";
    case V4L2_BUF_TYPE_VBI_CAPTURE:
      return "VBI Capture";
    case V4L2_BUF_TYPE_VBI_OUTPUT:
      return "VBI Output";
    case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
      return "Sliced VBI Capture";
    case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
      return "Sliced VBI Output";
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
      return "Video Output Overlay";
    case V4L2_BUF_TYPE_SDR_CAPTURE:
      return "SDR Capture";
    case V4L2_BUF_TYPE_SDR_OUTPUT:
      return "SDR Output";
    case V4L2_BUF_TYPE_META_CAPTURE:
      return "Metadata Capture";
    // case V4L2_BUF_TYPE_META_OUTPUT:
    //   return "Metadata Output";
    case V4L2_BUF_TYPE_PRIVATE:
      return "Private";
    default:
      return std::string("Unknown (") + num2s(type) + ")";
  }
}

// static constexpr flag_def bufcap_def[] = {
//     {V4L2_BUF_CAP_SUPPORTS_MMAP, "mmap"},
//     {V4L2_BUF_CAP_SUPPORTS_USERPTR, "userptr"},
//     {V4L2_BUF_CAP_SUPPORTS_DMABUF, "dmabuf"},
//     {V4L2_BUF_CAP_SUPPORTS_REQUESTS, "requests"},
//     {V4L2_BUF_CAP_SUPPORTS_ORPHANED_BUFS, "orphaned-bufs"},
//     {V4L2_BUF_CAP_SUPPORTS_M2M_HOLD_CAPTURE_BUF, "m2m-hold-capture-buf"},
//     {V4L2_BUF_CAP_SUPPORTS_MMAP_CACHE_HINTS, "mmap-cache-hints"},
//     {0, nullptr}};

// std::string bufcap2s(__u32 caps) { return flags2s(caps, bufcap_def); }

std::string field2s(int val) {
  switch (val) {
    case V4L2_FIELD_ANY:
      return "Any";
    case V4L2_FIELD_NONE:
      return "None";
    case V4L2_FIELD_TOP:
      return "Top";
    case V4L2_FIELD_BOTTOM:
      return "Bottom";
    case V4L2_FIELD_INTERLACED:
      return "Interlaced";
    case V4L2_FIELD_SEQ_TB:
      return "Sequential Top-Bottom";
    case V4L2_FIELD_SEQ_BT:
      return "Sequential Bottom-Top";
    case V4L2_FIELD_ALTERNATE:
      return "Alternating";
    case V4L2_FIELD_INTERLACED_TB:
      return "Interlaced Top-Bottom";
    case V4L2_FIELD_INTERLACED_BT:
      return "Interlaced Bottom-Top";
    default:
      return "Unknown (" + num2s(val) + ")";
  }
}

std::string colorspace2s(int val) {
  switch (val) {
    case V4L2_COLORSPACE_DEFAULT:
      return "Default";
    case V4L2_COLORSPACE_SMPTE170M:
      return "SMPTE 170M";
    case V4L2_COLORSPACE_SMPTE240M:
      return "SMPTE 240M";
    case V4L2_COLORSPACE_REC709:
      return "Rec. 709";
    case V4L2_COLORSPACE_BT878:
      return "Broken Bt878";
    case V4L2_COLORSPACE_470_SYSTEM_M:
      return "470 System M";
    case V4L2_COLORSPACE_470_SYSTEM_BG:
      return "470 System BG";
    case V4L2_COLORSPACE_JPEG:
      return "JPEG";
    case V4L2_COLORSPACE_SRGB:
      return "sRGB";
    case V4L2_COLORSPACE_OPRGB:
      return "opRGB";
    case V4L2_COLORSPACE_DCI_P3:
      return "DCI-P3";
    case V4L2_COLORSPACE_BT2020:
      return "BT.2020";
    case V4L2_COLORSPACE_RAW:
      return "Raw";
    default:
      return "Unknown (" + num2s(val) + ")";
  }
}

std::string xfer_func2s(int val) {
  switch (val) {
    case V4L2_XFER_FUNC_DEFAULT:
      return "Default";
    case V4L2_XFER_FUNC_709:
      return "Rec. 709";
    case V4L2_XFER_FUNC_SRGB:
      return "sRGB";
    case V4L2_XFER_FUNC_OPRGB:
      return "opRGB";
    case V4L2_XFER_FUNC_DCI_P3:
      return "DCI-P3";
    case V4L2_XFER_FUNC_SMPTE2084:
      return "SMPTE 2084";
    case V4L2_XFER_FUNC_SMPTE240M:
      return "SMPTE 240M";
    case V4L2_XFER_FUNC_NONE:
      return "None";
    default:
      return "Unknown (" + num2s(val) + ")";
  }
}

std::string ycbcr_enc2s(int val) {
  switch (val) {
    case V4L2_YCBCR_ENC_DEFAULT:
      return "Default";
    case V4L2_YCBCR_ENC_601:
      return "ITU-R 601";
    case V4L2_YCBCR_ENC_709:
      return "Rec. 709";
    case V4L2_YCBCR_ENC_XV601:
      return "xvYCC 601";
    case V4L2_YCBCR_ENC_XV709:
      return "xvYCC 709";
    case V4L2_YCBCR_ENC_BT2020:
      return "BT.2020";
    case V4L2_YCBCR_ENC_BT2020_CONST_LUM:
      return "BT.2020 Constant Luminance";
    case V4L2_YCBCR_ENC_SMPTE240M:
      return "SMPTE 240M";
    case V4L2_HSV_ENC_180:
      return "HSV with Hue 0-179";
    case V4L2_HSV_ENC_256:
      return "HSV with Hue 0-255";
    default:
      return "Unknown (" + num2s(val) + ")";
  }
}

std::string quantization2s(int val) {
  switch (val) {
    case V4L2_QUANTIZATION_DEFAULT:
      return "Default";
    case V4L2_QUANTIZATION_FULL_RANGE:
      return "Full Range";
    case V4L2_QUANTIZATION_LIM_RANGE:
      return "Limited Range";
    default:
      return "Unknown (" + num2s(val) + ")";
  }
}

static constexpr flag_def pixflags_def[] = {
    {V4L2_PIX_FMT_FLAG_PREMUL_ALPHA, "premultiplied-alpha"},
    // {V4L2_PIX_FMT_FLAG_SET_CSC, "set-csc"},
    {0, nullptr}};

std::string pixflags2s(unsigned flags) { return flags2s(flags, pixflags_def); }

static constexpr flag_def service_def[] = {{V4L2_SLICED_TELETEXT_B, "teletext"},
                                           {V4L2_SLICED_VPS, "vps"},
                                           {V4L2_SLICED_CAPTION_525, "cc"},
                                           {V4L2_SLICED_WSS_625, "wss"},
                                           {0, nullptr}};

std::string service2s(unsigned service) {
  return flags2s(service, service_def);
}

// #define FMTDESC_DEF(enc_type)                                           \
//   static constexpr flag_def fmtdesc_##enc_type##_def[] = {              \
//       {V4L2_FMT_FLAG_COMPRESSED, "compressed"},                         \
//       {V4L2_FMT_FLAG_EMULATED, "emulated"},                             \
//       {V4L2_FMT_FLAG_CONTINUOUS_BYTESTREAM, "continuous-bytestream"},   \
//       {V4L2_FMT_FLAG_DYN_RESOLUTION, "dyn-resolution"},                 \
//       {V4L2_FMT_FLAG_ENC_CAP_FRAME_INTERVAL, "enc-cap-frame-interval"}, \
//       {V4L2_FMT_FLAG_CSC_COLORSPACE, "csc-colorspace"},                 \
//       {V4L2_FMT_FLAG_CSC_YCBCR_ENC, "csc-" #enc_type},                  \
//       {V4L2_FMT_FLAG_CSC_QUANTIZATION, "csc-quantization"},             \
//       {V4L2_FMT_FLAG_CSC_XFER_FUNC, "csc-xfer-func"},                   \
//       {0, NULL}};

// FMTDESC_DEF(ycbcr)
// FMTDESC_DEF(hsv)

// std::string fmtdesc2s(unsigned flags, bool is_hsv) {
//   if (is_hsv) return flags2s(flags, fmtdesc_hsv_def);
//   return flags2s(flags, fmtdesc_ycbcr_def);
// }

// #define MBUS_DEF(enc_type)                                          \
//   static constexpr flag_def mbus_##enc_type##_def[] = {             \
//       {V4L2_SUBDEV_MBUS_CODE_CSC_COLORSPACE, "csc-colorspace"},     \
//       {V4L2_SUBDEV_MBUS_CODE_CSC_YCBCR_ENC, "csc-" #enc_type},      \
//       {V4L2_SUBDEV_MBUS_CODE_CSC_QUANTIZATION, "csc-quantization"}, \
//       {V4L2_SUBDEV_MBUS_CODE_CSC_XFER_FUNC, "csc-xfer-func"},       \
//       {0, NULL}};

// MBUS_DEF(ycbcr)
// MBUS_DEF(hsv)

// std::string mbus2s(unsigned flags, bool is_hsv) {
//   if (is_hsv) return flags2s(flags, mbus_hsv_def);
//   return flags2s(flags, mbus_ycbcr_def);
// }

static constexpr flag_def selection_targets_def[] = {
    {V4L2_SEL_TGT_CROP_ACTIVE, "crop"},
    {V4L2_SEL_TGT_CROP_DEFAULT, "crop_default"},
    {V4L2_SEL_TGT_CROP_BOUNDS, "crop_bounds"},
    {V4L2_SEL_TGT_COMPOSE_ACTIVE, "compose"},
    {V4L2_SEL_TGT_COMPOSE_DEFAULT, "compose_default"},
    {V4L2_SEL_TGT_COMPOSE_BOUNDS, "compose_bounds"},
    {V4L2_SEL_TGT_COMPOSE_PADDED, "compose_padded"},
    {V4L2_SEL_TGT_NATIVE_SIZE, "native_size"},
    {0, nullptr}};

bool valid_seltarget_at_idx(unsigned i) {
  return i <
         sizeof(selection_targets_def) / sizeof(selection_targets_def[0]) - 1;
}

unsigned seltarget_at_idx(unsigned i) {
  if (valid_seltarget_at_idx(i)) return selection_targets_def[i].flag;
  return 0;
}

std::string seltarget2s(__u32 target) {
  int i = 0;

  while (selection_targets_def[i].str != nullptr) {
    if (selection_targets_def[i].flag == target)
      return selection_targets_def[i].str;
    i++;
  }
  return std::string("Unknown (") + num2s(target) + ")";
}

const flag_def selection_flags_def[] = {
    {V4L2_SEL_FLAG_GE, "ge"},
    {V4L2_SEL_FLAG_LE, "le"},
    {V4L2_SEL_FLAG_KEEP_CONFIG, "keep-config"},
    {0, nullptr}};

std::string selflags2s(__u32 flags) {
  return flags2s(flags, selection_flags_def);
}

static const char *std_pal[] = {"B", "B1", "G", "H",  "I",  "D",    "D1",
                                "K", "M",  "N", "Nc", "60", nullptr};
static const char *std_ntsc[] = {"M", "M-JP", "443", "M-KR", nullptr};
static const char *std_secam[] = {"B",  "D", "G",  "H",    "K",
                                  "K1", "L", "Lc", nullptr};
static const char *std_atsc[] = {"8-VSB", "16-VSB", nullptr};

static std::string partstd2s(const char *prefix, const char *stds[],
                             unsigned long long std) {
  std::string s = std::string(prefix) + "-";
  int first = 1;

  while (*stds) {
    if (std & 1) {
      if (!first) s += "/";
      first = 0;
      s += *stds;
    }
    stds++;
    std >>= 1;
  }
  return s;
}

std::string std2s(v4l2_std_id std, const char *sep) {
  std::string s;

  if (std & 0xfff) {
    s += partstd2s("PAL", std_pal, std);
  }
  if (std & 0xf000) {
    if (s.length()) s += sep;
    s += partstd2s("NTSC", std_ntsc, std >> 12);
  }
  if (std & 0xff0000) {
    if (s.length()) s += sep;
    s += partstd2s("SECAM", std_secam, std >> 16);
  }
  if (std & 0xf000000) {
    if (s.length()) s += sep;
    s += partstd2s("ATSC", std_atsc, std >> 24);
  }
  return s;
}

std::string ctrlflags2s(__u32 flags) {
  static constexpr flag_def def[] = {
      {V4L2_CTRL_FLAG_GRABBED, "grabbed"},
      {V4L2_CTRL_FLAG_DISABLED, "disabled"},
      {V4L2_CTRL_FLAG_READ_ONLY, "read-only"},
      {V4L2_CTRL_FLAG_UPDATE, "update"},
      {V4L2_CTRL_FLAG_INACTIVE, "inactive"},
      {V4L2_CTRL_FLAG_SLIDER, "slider"},
      {V4L2_CTRL_FLAG_WRITE_ONLY, "write-only"},
      {V4L2_CTRL_FLAG_VOLATILE, "volatile"},
      {V4L2_CTRL_FLAG_HAS_PAYLOAD, "has-payload"},
      {V4L2_CTRL_FLAG_EXECUTE_ON_WRITE, "execute-on-write"},
      {V4L2_CTRL_FLAG_MODIFY_LAYOUT, "modify-layout"},
      // {V4L2_CTRL_FLAG_DYNAMIC_ARRAY, "dynamic-array"},
      {0, nullptr}};
  return flags2s(flags, def);
}

static constexpr flag_def in_status_def[] = {
    {V4L2_IN_ST_NO_POWER, "no power"},
    {V4L2_IN_ST_NO_SIGNAL, "no signal"},
    {V4L2_IN_ST_NO_COLOR, "no color"},
    {V4L2_IN_ST_HFLIP, "hflip"},
    {V4L2_IN_ST_VFLIP, "vflip"},
    {V4L2_IN_ST_NO_H_LOCK, "no hsync lock"},
    {V4L2_IN_ST_NO_V_LOCK, "no vsync lock"},
    {V4L2_IN_ST_NO_STD_LOCK, "no standard format lock"},
    {V4L2_IN_ST_COLOR_KILL, "color kill"},
    {V4L2_IN_ST_NO_SYNC, "no sync lock"},
    {V4L2_IN_ST_NO_EQU, "no equalizer lock"},
    {V4L2_IN_ST_NO_CARRIER, "no carrier"},
    {V4L2_IN_ST_MACROVISION, "macrovision"},
    {V4L2_IN_ST_NO_ACCESS, "no conditional access"},
    {V4L2_IN_ST_VTR, "VTR time constant"},
    {0, nullptr}};

std::string in_status2s(__u32 status) {
  return status ? flags2s(status, in_status_def) : "ok";
}

static constexpr flag_def input_cap_def[] = {
    {V4L2_IN_CAP_DV_TIMINGS, "DV timings"},
    {V4L2_IN_CAP_STD, "SDTV standards"},
    {V4L2_IN_CAP_NATIVE_SIZE, "Native Size"},
    {0, nullptr}};

std::string input_cap2s(__u32 capabilities) {
  return capabilities ? flags2s(capabilities, input_cap_def) : "not defined";
}

static constexpr flag_def output_cap_def[] = {
    {V4L2_OUT_CAP_DV_TIMINGS, "DV timings"},
    {V4L2_OUT_CAP_STD, "SDTV standards"},
    {V4L2_OUT_CAP_NATIVE_SIZE, "Native Size"},
    {0, nullptr}};

std::string output_cap2s(__u32 capabilities) {
  return capabilities ? flags2s(capabilities, output_cap_def) : "not defined";
}

std::string fbufcap2s(unsigned cap) {
  std::string s;

  if (cap & V4L2_FBUF_CAP_EXTERNOVERLAY) s += "\t\t\tExtern Overlay\n";
  if (cap & V4L2_FBUF_CAP_CHROMAKEY) s += "\t\t\tChromakey\n";
  if (cap & V4L2_FBUF_CAP_SRC_CHROMAKEY) s += "\t\t\tSource Chromakey\n";
  if (cap & V4L2_FBUF_CAP_GLOBAL_ALPHA) s += "\t\t\tGlobal Alpha\n";
  if (cap & V4L2_FBUF_CAP_LOCAL_ALPHA) s += "\t\t\tLocal Alpha\n";
  if (cap & V4L2_FBUF_CAP_LOCAL_INV_ALPHA) s += "\t\t\tLocal Inverted Alpha\n";
  if (cap & V4L2_FBUF_CAP_LIST_CLIPPING) s += "\t\t\tClipping List\n";
  if (cap & V4L2_FBUF_CAP_BITMAP_CLIPPING) s += "\t\t\tClipping Bitmap\n";
  if (s.empty()) s += "\t\t\t\n";
  return s;
}

std::string fbufflags2s(unsigned fl) {
  std::string s;

  if (fl & V4L2_FBUF_FLAG_PRIMARY) s += "\t\t\tPrimary Graphics Surface\n";
  if (fl & V4L2_FBUF_FLAG_OVERLAY)
    s += "\t\t\tOverlay Matches Capture/Output Size\n";
  if (fl & V4L2_FBUF_FLAG_CHROMAKEY) s += "\t\t\tChromakey\n";
  if (fl & V4L2_FBUF_FLAG_SRC_CHROMAKEY) s += "\t\t\tSource Chromakey\n";
  if (fl & V4L2_FBUF_FLAG_GLOBAL_ALPHA) s += "\t\t\tGlobal Alpha\n";
  if (fl & V4L2_FBUF_FLAG_LOCAL_ALPHA) s += "\t\t\tLocal Alpha\n";
  if (fl & V4L2_FBUF_FLAG_LOCAL_INV_ALPHA) s += "\t\t\tLocal Inverted Alpha\n";
  if (s.empty()) s += "\t\t\t\n";
  return s;
}

static constexpr flag_def dv_standards_def[] = {
    {V4L2_DV_BT_STD_CEA861, "CTA-861"}, {V4L2_DV_BT_STD_DMT, "DMT"},
    {V4L2_DV_BT_STD_CVT, "CVT"},        {V4L2_DV_BT_STD_GTF, "GTF"},
    {V4L2_DV_BT_STD_SDI, "SDI"},        {0, nullptr}};

std::string dv_standards2s(__u32 flags) {
  return flags2s(flags, dv_standards_def);
}

std::string dvflags2s(unsigned vsync, int val) {
  std::string s;

  if (val & V4L2_DV_FL_REDUCED_BLANKING)
    s += vsync == 8 ? "reduced blanking v2, " : "reduced blanking, ";
  if (val & V4L2_DV_FL_CAN_REDUCE_FPS)
    s += "framerate can be reduced by 1/1.001, ";
  if (val & V4L2_DV_FL_REDUCED_FPS) s += "framerate is reduced by 1/1.001, ";
  // if (val & V4L2_DV_FL_CAN_DETECT_REDUCED_FPS)
  //   s += "can detect reduced framerates, ";
  if (val & V4L2_DV_FL_HALF_LINE) s += "half-line, ";
  if (val & V4L2_DV_FL_IS_CE_VIDEO) s += "CE-video, ";
  if (val & V4L2_DV_FL_FIRST_FIELD_EXTRA_LINE)
    s += "first field has extra line, ";
  if (val & V4L2_DV_FL_HAS_PICTURE_ASPECT) s += "has picture aspect, ";
  if (val & V4L2_DV_FL_HAS_CEA861_VIC) s += "has CTA-861 VIC, ";
  if (val & V4L2_DV_FL_HAS_HDMI_VIC) s += "has HDMI VIC, ";
  if (s.length()) return s.erase(s.length() - 2, 2);
  return s;
}

static constexpr flag_def dv_caps_def[] = {
    {V4L2_DV_BT_CAP_INTERLACED, "Interlaced"},
    {V4L2_DV_BT_CAP_PROGRESSIVE, "Progressive"},
    {V4L2_DV_BT_CAP_REDUCED_BLANKING, "Reduced Blanking"},
    {V4L2_DV_BT_CAP_CUSTOM, "Custom Formats"},
    {0, nullptr}};

std::string dv_caps2s(__u32 flags) { return flags2s(flags, dv_caps_def); }

static constexpr flag_def tc_flags_def[] = {
    {V4L2_TC_FLAG_DROPFRAME, "dropframe"},
    {V4L2_TC_FLAG_COLORFRAME, "colorframe"},
    {V4L2_TC_USERBITS_field, "userbits-field"},
    {V4L2_TC_USERBITS_USERDEFINED, "userbits-userdefined"},
    {V4L2_TC_USERBITS_8BITCHARS, "userbits-8bitchars"},
    {0, nullptr}};

std::string tc_flags2s(__u32 flags) { return flags2s(flags, tc_flags_def); }

static constexpr flag_def buffer_flags_def[] = {
    {V4L2_BUF_FLAG_MAPPED, "mapped"},
    {V4L2_BUF_FLAG_QUEUED, "queued"},
    {V4L2_BUF_FLAG_DONE, "done"},
    {V4L2_BUF_FLAG_KEYFRAME, "keyframe"},
    {V4L2_BUF_FLAG_PFRAME, "P-frame"},
    {V4L2_BUF_FLAG_BFRAME, "B-frame"},
    {V4L2_BUF_FLAG_ERROR, "error"},
    {V4L2_BUF_FLAG_TIMECODE, "timecode"},
    // {V4L2_BUF_FLAG_M2M_HOLD_CAPTURE_BUF, "m2m-hold-capture-buf"},
    {V4L2_BUF_FLAG_PREPARED, "prepared"},
    {V4L2_BUF_FLAG_NO_CACHE_INVALIDATE, "no-cache-invalidate"},
    {V4L2_BUF_FLAG_NO_CACHE_CLEAN, "no-cache-clean"},
    {V4L2_BUF_FLAG_LAST, "last"},
    // {V4L2_BUF_FLAG_REQUEST_FD, "request-fd"},
    // {V4L2_BUF_FLAG_IN_REQUEST, "in-request"},
    {0, nullptr}};

std::string bufferflags2s(__u32 flags) {
  const unsigned ts_mask =
      V4L2_BUF_FLAG_TIMESTAMP_MASK | V4L2_BUF_FLAG_TSTAMP_SRC_MASK;
  std::string s = flags2s(flags & ~ts_mask, buffer_flags_def);

  if (s.length()) s += ", ";

  switch (flags & V4L2_BUF_FLAG_TIMESTAMP_MASK) {
    case V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN:
      s += "ts-unknown";
      break;
    case V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC:
      s += "ts-monotonic";
      break;
    case V4L2_BUF_FLAG_TIMESTAMP_COPY:
      s += "ts-copy";
      break;
    default:
      s += "ts-invalid";
      break;
  }
  switch (flags & V4L2_BUF_FLAG_TSTAMP_SRC_MASK) {
    case V4L2_BUF_FLAG_TSTAMP_SRC_EOF:
      s += ", ts-src-eof";
      break;
    case V4L2_BUF_FLAG_TSTAMP_SRC_SOE:
      s += ", ts-src-soe";
      break;
    default:
      s += ", ts-src-invalid";
      break;
  }
  return s;
}

static const flag_def vbi_def[] = {{V4L2_VBI_UNSYNC, "unsynchronized"},
                                   {V4L2_VBI_INTERLACED, "interlaced"},
                                   {0, nullptr}};

std::string vbiflags2s(__u32 flags) { return flags2s(flags, vbi_def); }

std::string ttype2s(int type) {
  switch (type) {
    case V4L2_TUNER_RADIO:
      return "radio";
    case V4L2_TUNER_ANALOG_TV:
      return "Analog TV";
    case V4L2_TUNER_DIGITAL_TV:
      return "Digital TV";
    case V4L2_TUNER_SDR:
      return "SDR";
    case V4L2_TUNER_RF:
      return "RF";
    default:
      return "unknown";
  }
}

std::string audmode2s(int audmode) {
  switch (audmode) {
    case V4L2_TUNER_MODE_STEREO:
      return "stereo";
    case V4L2_TUNER_MODE_LANG1:
      return "lang1";
    case V4L2_TUNER_MODE_LANG2:
      return "lang2";
    case V4L2_TUNER_MODE_LANG1_LANG2:
      return "bilingual";
    case V4L2_TUNER_MODE_MONO:
      return "mono";
    default:
      return "unknown";
  }
}

std::string rxsubchans2s(int rxsubchans) {
  std::string s;

  if (rxsubchans & V4L2_TUNER_SUB_MONO) s += "mono ";
  if (rxsubchans & V4L2_TUNER_SUB_STEREO) s += "stereo ";
  if (rxsubchans & V4L2_TUNER_SUB_LANG1) s += "lang1 ";
  if (rxsubchans & V4L2_TUNER_SUB_LANG2) s += "lang2 ";
  if (rxsubchans & V4L2_TUNER_SUB_RDS) s += "rds ";
  return s;
}

std::string txsubchans2s(int txsubchans) {
  std::string s;

  if (txsubchans & V4L2_TUNER_SUB_MONO) s += "mono ";
  if (txsubchans & V4L2_TUNER_SUB_STEREO) s += "stereo ";
  if (txsubchans & V4L2_TUNER_SUB_LANG1) s += "bilingual ";
  if (txsubchans & V4L2_TUNER_SUB_SAP) s += "sap ";
  if (txsubchans & V4L2_TUNER_SUB_RDS) s += "rds ";
  return s;
}

std::string tcap2s(unsigned cap) {
  std::string s;

  if (cap & V4L2_TUNER_CAP_LOW)
    s += "62.5 Hz ";
  else if (cap & V4L2_TUNER_CAP_1HZ)
    s += "1 Hz ";
  else
    s += "62.5 kHz ";
  if (cap & V4L2_TUNER_CAP_NORM) s += "multi-standard ";
  if (cap & V4L2_TUNER_CAP_HWSEEK_BOUNDED) s += "hwseek-bounded ";
  if (cap & V4L2_TUNER_CAP_HWSEEK_WRAP) s += "hwseek-wrap ";
  if (cap & V4L2_TUNER_CAP_STEREO) s += "stereo ";
  if (cap & V4L2_TUNER_CAP_LANG1) s += "lang1 ";
  if (cap & V4L2_TUNER_CAP_LANG2) s += "lang2 ";
  if (cap & V4L2_TUNER_CAP_RDS) s += "rds ";
  if (cap & V4L2_TUNER_CAP_RDS_BLOCK_IO) s += "rds-block-I/O ";
  if (cap & V4L2_TUNER_CAP_RDS_CONTROLS) s += "rds-controls ";
  if (cap & V4L2_TUNER_CAP_FREQ_BANDS) s += "freq-bands ";
  if (cap & V4L2_TUNER_CAP_HWSEEK_PROG_LIM) s += "hwseek-prog-lim ";
  return s;
}

std::string modulation2s(unsigned modulation) {
  switch (modulation) {
    case V4L2_BAND_MODULATION_VSB:
      return "VSB";
    case V4L2_BAND_MODULATION_FM:
      return "FM";
    case V4L2_BAND_MODULATION_AM:
      return "AM";
  }
  return "Unknown";
}
