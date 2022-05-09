/**
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*!
 *  @brief      Configurations for MMPF
 */
#ifndef MMPF_MMPFCONFIGURE_H_INCLUDED
#define MMPF_MMPFCONFIGURE_H_INCLUDED

//// Default, sm7250
#define _MMPF_CHIPSET_VENDOR_QCT_
#define _MMPF_CHIPSET_CATEGORY_QCT_SM7250_
////

// MMPF version
#define MMPF_VERSION "MMPF_VER_2.0.0_160519"

// platform configuration

#define MMPF_PLATFORM_ANDROID
// #define MMPF_PLATFORM_WIN32
// #define MMPF_PLATFORM_REX

#if defined(MMPF_PLATFORM_ANDROID) || defined(MMPF_PLATFORM_WIN32)
#define MMPF_PLATFORM_GROUP_AP
#else
#define MMPF_PLATFORM_GROUP_CP
#endif

#ifdef MMPF_PLATFORM_GROUP_AP
#define MMPF_RUN_BASE_THREAD
#define MMPF_ENABLE_SCHEDULER
#else
#define MMPF_RUN_BASE_EVENT
// #define MMPF_ENABLE_SCHEDULER
#endif

#ifdef MMPF_PLATFORM_REX

#ifdef MMPF_ENABLE_CP_PROXY
#define _MMPF_CHIPSET_VENDOR_QCT_
// #define _MMPF_AUDIOSUBSYSTEM_MVS_
#define _MMPF_AUDIOSUBSYSTEM_CVD_
#endif
#define _MMPF_ENABLE_TEXT_
#define _MMPF_TEXTSUBSYSTEM_CVD_

#define _MMPF_DEBUG_BUILD_
// #define MMPF_MVS_VOICE_PACKET_SIZE 320
#define MMPF_CVD_VOICE_PACKET_SIZE     322
#define MMPF_CVD_VOICE_PACKET_HDR_SIZE 1
#define MMPF_CVD_TEXT_PACKET_SIZE      4
// #define MMPF_ENABLE_SCHEDULER
// #define MMPF_ENABLE_VIDEO
// #define MMPF_ENABLE_3GPPSYSTEM
// #define MMPF_3GPPLOCALRENDERING
// #define MMPF_USE_TEST_AUDIO_CONTROL
// #define USE_MMPF_HARDWARE_RENDERER // use QCT HW renderer, we can use HW renderer at MSM8660 GB.
// #define MMPF_DEBUG_BUILD
// #define MMPF_ANDROID_VOICE_INTERFACE_V2
// #define MMPF_AUDIOSUBSYSTEM_MVS_ALSA

#endif  // MMPF_PLATFORM_REX

#ifdef MMPF_PLATFORM_ANDROID

#define MMPF_MEDIA_RESOURCE_ROOT_DIR "/system/media/ims"

#ifdef _MMPF_IF_SERVICE_  // defined in libmmpf/Configure.mk
#define MMPF_IF_SERVICE
#else
#define MMPF_IF_MANAGER
#endif

#ifdef _MMPF_TESTMODE__  // defined in libmmpf/Configure.mk
#define MMPF_DUMP_PATH "/data/user_de/0/com.test.imsmedia/"
#endif

// Android version configuration
#if defined(_MMPF_ANDROID_VER_1_5_)  // defined in libmmpf/Configure.mk
#error Current MMPF does not support android 1.5, use MMPF_VER 1.2.5 for android 1.5
#elif defined(_MMPF_ANDROID_VER_2_1_) || defined(MMPF_ANDROID_VER_2_2_)
#define __MMPF_ANDROID_2_2__
#elif defined(_MMPF_ANDROID_VER_3_0_)
#define __MMPF_ANDROID_3_0__
#elif defined(_MMPF_ANDROID_VER_4_0_)
#define __MMPF_ANDROID_4_0__
#elif defined(_MMPF_ANDROID_VER_4_1_)
#define __MMPF_ANDROID_4_1__
#elif defined(_MMPF_ANDROID_VER_4_4_)
#define __MMPF_ANDROID_4_4__
#endif

#else

#define MMPF_MEDIA_RESOURCE_ROOT_DIR "/system/media/ims"

#endif  // MMPF_PLATFORM_ANDROID

#if defined(_MMPF_CHIPSET_VENDOR_QCT_)
#define MMPF_CHIPSET_VENDOR_QCT
#elif defined(_MMPF_CHIPSET_VENDOR_NVIDIA_)
#define MMPF_CHIPSET_VENDOR_NVIDIA
#elif defined(_MMPF_CHIPSET_VENDOR_TI_)
#define MMPF_CHIPSET_VENDOR_TI
#elif defined(_MMPF_CHIPSET_VENDOR_MTK_)
#define MMPF_CHIPSET_VENDOR_MTK
#elif defined(_MMPF_CHIPSET_VENDOR_XXX_LIGER_)
#define MMPF_CHIPSET_VENDOR_XXX
#define MMPF_CHIPSET_VENDOR_XXX_LIGER
#elif defined(_MMPF_CHIPSET_VENDOR_XXX_L5000_)
#define MMPF_CHIPSET_VENDOR_XXX
#define MMPF_CHIPSET_VENDOR_XXX_L5000

#endif

#if defined(_MMPF_CHIPSET_CATEGORY_QCT_FUSION3_)
#define MMPF_CHIPSET_CATEGORY_QCT_FUSION3  // APQ8064
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8960_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8960
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8926_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8926
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8x26_W_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8x26_W  // W5
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8994_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8994
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8992_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8992
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8909_)  // MSM8909
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8909
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8996_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8996
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8916_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8916  // MSM8916
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8952_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8952  // MSM8952
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8976_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8976  // MSM8976
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8937_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8937  // MSM8937
#elif defined(_MMPF_CHIPSET_CATEGORY_MTK_MT6753_)
#define MMPF_CHIPSET_CATEGORY_MTK_MT6753
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8917_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8917  // MSM8917
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8940_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8940  // MSM8940
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_MSM8998_)
#define MMPF_CHIPSET_CATEGORY_QCT_MSM8998
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_SDM845_)
#define MMPF_CHIPSET_CATEGORY_QCT_SDM845
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_SDM450_)
#define MMPF_CHIPSET_CATEGORY_QCT_SDM450  // SDM450
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_SM8150_)
#define MMPF_CHIPSET_CATEGORY_QCT_SM8150  // SM8150
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_SM6150_)
#define MMPF_CHIPSET_CATEGORY_QCT_SM6150  // SM6150
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_SM6115_)
#define MMPF_CHIPSET_CATEGORY_QCT_SM6115  // SM6115
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_SM7250_)
#define MMPF_CHIPSET_CATEGORY_QCT_SM7250  // SM7250
#elif defined(_MMPF_CHIPSET_CATEGORY_QCT_SM8250_)
#define MMPF_CHIPSET_CATEGORY_QCT_SM8250  // SM8250
#elif defined(_MMPF_CHIPSET_CATEGORY_MTK_MT6883_)
#define MMPF_CHIPSET_CATEGORY_MTK_MT6883
#else
#define MMPF_CHIPSET_CATEGORY_DEFAULT  // MSM8974, MSM8x26
#endif

#ifdef _MMPF_ENABLE_CP_PROXY_  // defined in libmmpf/Configure.mk
#define MMPF_ENABLE_CP_PROXY
#endif

#ifdef _MMPF_ENABLE_AUDIO_  // defined in libmmpf/Configure.mk
#define MMPF_ENABLE_AUDIO
#endif

#ifdef _MMPF_AUDIOSUBSYSTEM_MVS_  // defined in libmmpf/Configure.mk
#define MMPF_AUDIOSUBSYSTEM_MVS
#endif

#ifdef _MMPF_AUDIOSUBSYSTEM_MVS_KERNEL_  // defined in libmmpf/Configure.mk
#define MMPF_AUDIOSUBSYSTEM_MVS_KERNEL
#endif

#ifdef _MMPF_AUDIOSUBSYSTEM_MVS_ALSA_  // defined in libmmpf/Configure.mk
#define MMPF_AUDIOSUBSYSTEM_MVS_ALSA
#endif

#ifdef _MMPF_AUDIOSUBSYSTEM_AUDIOFLINGER_  // defined in libmmpf/Configure.mk
#define MMPF_AUDIOSUBSYSTEM_AUDIOFLINGER
#endif

#ifdef _MMPF_AUDIOSUBSYSTEM_CVD_  // defined in libmmpf/Configure.mk
#define MMPF_AUDIOSUBSYSTEM_CVD
#endif

#ifdef _MMPF_ENABLE_VIDEO_  // defined in libmmpf/Configure.mk
#define MMPF_ENABLE_VIDEO
#endif

#ifdef _MMPF_ENABLE_JPEG_
#define MMPF_ENABLE_JPEG  // defined in libmmpf/Configure.mk
#endif

#ifdef _MMPF_ENABLE_3GPPSYSTEM_COMPOSER_  // defined in libmmpf/Configure.mk
#define MMPF_ENABLE_3GPPSYSTEM_COMPOSER
#endif

#ifdef _MMPF_ENABLE_3GPPSYSTEM_PARSER_  // defined in libmmpf/Configure.mk
#define MMPF_ENABLE_3GPPSYSTEM_PARSER
#endif

#ifdef _MMPF_3GPPLOCALRENDERING_  // defined in libmmpf/Configure.mk
#define MMPF_3GPPLOCALRENDERING
#endif

#ifdef _MMPF_USE_TEST_AUDIO_CONTROL_  // defined in libmmpf/Configure.mk
#define MMPF_USE_TEST_AUDIO_CONTROL
#endif

#ifdef _MMPF_ENABLE_VIDEO_HARDWARE_RENDERER_
#define USE_MMPF_HARDWARE_RENDERER  // use QCT HW renderer, we can use HW renderer at MSM8660 GB.
#endif

#ifdef _MMPF_DEBUG_BUILD_  // defined in libmmpf/Configure.mk
#define MMPF_DEBUG_BUILD
#endif

#ifdef _MMPF_ENABLE_TEXT_  // defined in libmmpf/Configure.mk
#define MMPF_ENABLE_TEXT
#endif

#ifdef _MMPF_TEXTSUBSYSTEM_CVD_  // defined in libmmpf/Configure.mk
#define MMPF_TEXTSUBSYSTEM_CVD
#endif

#ifdef _MMPF_MEDIA_ENGINE_  // defined in libmmpf/Configure.mk
#define MMPF_MEDIA_ENGINE
#endif

#ifdef _MMPF_ENABLE_CVO_SUPPORT_180_ROTATION_
#define USE_CVO_180_ROTATION
#endif

/*  2014/12/1 SRTP Start */
#ifdef _MMPF_ENABLE_SRTP_  // defined at the makefile
#define ENABLE_SRTP_LIB_WITH_LIBSRTP144
#endif
/*  2014/12/1 SRTP End */
// Android version specific configurations

#ifdef _MMPF_VOCODER_FORCE_MVS_  // defined at the makefile
#define MMPF_VOCODER_FORCE_MVS
#endif

#ifdef _MMPF_CODEC_AMRWB_DISABLE_  // defined at the makefile
#define MMPF_CODEC_AMRWB_DISABLE
#endif

#ifdef _MMPF_CODEC_EVS_DISABLE_  // defined at the makefile
#define MMPF_CODEC_EVS_DISABLE
#endif

#ifdef _MMPF_CODEC_EVS_SWB_DISABLE_  // defined at the makefile
#define MMPF_CODEC_EVS_SWB_DISABLE
#endif

#ifdef _MMPF_USE_VADAPTOR_  // defined at the makefile
#define MMPF_AUDIO_USE_VADAPTOR
#endif

#ifdef _MMPF_CODEC_HEVC_ENABLE_
#define MMPF_CODEC_HEVC_ENABLE
#endif

#ifdef _MMPF_CODEC_RTCPXR_ENABLE_
#define MMPF_CODEC_RTCPXR_ENABLE
#endif

#ifdef _MMPF_SUPPORT_CHP_ENABLE_
#define MMPF_SUPPORT_CHP_ENABLE
#endif

#ifdef _MMPF_USE_LEGACY_CAMERA_
#define MMPF_USE_LEGACY_CAMERA
#endif

#ifdef _MMPF_CODEC2_SUPPORT_
#define MMPF_CODEC2_SUPPORT
#endif

#if defined(__MMPF_ANDROID_4_4__)
#define MMPF_CAMERA_USE_GRAPHICBUFFER
#define MMPF_VIDEO_RENDERER_USE_GRAPHICBUFFER
#define MMPF_CVD_VOICE_PACKET_SIZE     322
#define MMPF_CVD_VOICE_PACKET_HDR_SIZE 1
#define MMPF_MVS_VOICE_PACKET_SIZE     640
#define MMPF_CVD_TEXT_PACKET_SIZE      4
#elif defined(__MMPF_ANDROID_3_0__) || defined(__MMPF_ANDROID_4_0__) || \
        defined(__MMPF_ANDROID_4_1__)
#define MMPF_CAMERA_USE_SURFACE
// #define MMPF_CAMERA_LANDSCAPE_DEVICE        // tablet
#define MMPF_VIDEO_RENDERER_USE_SURFACE
#define MMPF_MVS_VOICE_PACKET_SIZE     640
#define MMPF_CVD_VOICE_PACKET_SIZE     322
#define MMPF_CVD_VOICE_PACKET_HDR_SIZE 1
#define MMPF_CVD_TEXT_PACKET_SIZE      4
#else
#define MMPF_MVS_VOICE_PACKET_SIZE     320
#define MMPF_CVD_VOICE_PACKET_SIZE     322
#define MMPF_CVD_VOICE_PACKET_HDR_SIZE 1
#define MMPF_CVD_TEXT_PACKET_SIZE      4
#endif

#if !defined(__MMPF_ANDROID_3_0__)
#define MMPF_ANDROID_CAMEERA_INTERFACE_V2
#define MMPF_ANDROID_VOICE_INTERFACE_V2
#endif

#if defined(__MMPF_ANDROID_3_0__) || defined(__MMPF_ANDROID_4_0__)
#define MMPF_SYSTEM_SURFACE_H_PATH  <surfaceflinger/Surface.h>
#define MMPF_SYSTEM_ISURFACE_H_PATH <surfaceflinger/ISurface.h>
#elif defined(__MMPF_ANDROID_4_4__)
#define MMPF_SYSTEM_SURFACE_H_PATH  <gui/Surface.h>
#define MMPF_SYSTEM_ISURFACE_H_PATH <gui / Surface.h>  // ISurface is not exist anymore
#else
#define MMPF_SYSTEM_SURFACE_H_PATH  <gui/Surface.h>
#define MMPF_SYSTEM_ISURFACE_H_PATH <gui/ISurface.h>
#endif

#if defined(_MMPF_USE_IP_SINGLE_)
#define MMPF_USE_IP_SINGLE
#endif

// RTCP Configuration

// 131029: Actually, RTCP interval will be handled by external interface (defined MMPFDefinition.h)
// #define MMPF_RTCP_PERIOD    3    // SKT/KT/U+, default
#define MMPF_RTCP_PERIOD                     1  // EU VDF

// Encoder Configuration
// Intra frame period in second unit, 1 for LGU+, 2 for SKT, default value is 1
#define MMPF_VIDEOENCODER_INTRA_FRAME_PERIOD 1

// from D1L - MSM8960 ... only to disable preview in VT conference call.
#ifdef MMPF_CHIPSET_VENDOR_QCT
#define MMPF_CAMERA_SET_VT_MODE_ONLY_TO_DISABLE_PREVIEW
#endif

// Using MMPF Camera Jni (in case of Camera HAL 3.0)
#ifndef MMPF_USE_LEGACY_CAMERA
#define USING_MMPF_CAMERA_JNI
#endif

// UBWC On/Off
#if defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8996) || defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8998)
#define MMPF_VIDEO_FORMAT_ENCODER_UBWC
#endif
// #if defined(MMPF_CHIPSET_CATEGORY_QCT_SDM845)
// #define MMPF_VIDEO_FORMAT_ENCODER_UBWC
// #endif

// vadaptor
#ifndef MMPF_AUDIO_USE_VADAPTOR
#if defined(MMPF_CHIPSET_VENDOR_QCT)
#define MMPF_AUDIO_USE_VADAPTOR
#endif
#endif

// VoNR Socket usage (MTK only)
#if defined(MMPF_CHIPSET_CATEGORY_MTK_MT6883)
#define MMPF_VONR_SOCKET_SUPPORT
#endif

// Camera output data
#if defined(MMPF_CHIPSET_VENDOR_TI) || defined(MMPF_CHIPSET_VENDOR_XXX)
#define MMPF_CAMERA_USE_PREVIEW_CALLBACK
#else
#define MMPF_CAMERA_USE_DATA_CALLBACK
#endif

#ifdef MMPF_CHIPSET_CATEGORY_QCT_MSM8926
#define MMPF_DISABLE_CAMERA_FULL_SCREEN
#endif

// OMX option
#if defined(MMPF_CHIPSET_VENDOR_TI) || defined(MMPF_CHIPSET_VENDOR_XXX) || \
        defined(MMPF_CHIPSET_VENDOR_MTK) || defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8909)
#define MMPF_OMX_CALL_FILLTHISBUFFER_AT_MMPF_CONTEXT
#endif

// Decoder Output Option
//    color format of video renderer input (color format of video decoder output)
//    this definition is used at SW video rendereer

#if defined(MMPF_CHIPSET_VENDOR_QCT)
#if defined(MMPF_CHIPSET_CATEGORY_QCT_FUSION3) || defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8960)
#define MMPF_DECODER_OUTPUTFORMAT_YUV420PTILED  // NV12T, I project, MSM8660, Fusion3(APQ8064)
#else
// #define MMPF_DECODER_OUTPUT_FORMAT_NV21           // NV21, U0
#define MMPF_DECODER_OUTPUT_FORMAT_NV12  // MSM8974
#endif
#elif defined(MMPF_CHIPSET_VENDOR_TI)
#define MMPF_DECODER_OUTPUT_FORMAT_NV12  // NV12, Default, TI
#elif defined(MMPF_CHIPSET_VENDOR_NVIDIA)
#define MMPF_DECODER_OUTPUT_FORMAT_YUV420P  // YUV420P, NVIDIA, MTK
#elif defined(MMPF_CHIPSET_VENDOR_MTK)
#define MMPF_DECODER_OUTPUT_FORMAT_YUV420P  // NV12, MTK
#elif defined(MMPF_CHIPSET_VENDOR_XXX)
#define MMPF_DECODER_OUTPUT_FORMAT_YUV420P  // NV12, XXX
#else
// #define MMPF_DECODER_OUTPUTFORMAT_YUV420PTILED    // NV12T, I project, MSM8660
#define MMPF_DECODER_OUTPUT_FORMAT_NV21  // NV21, U0
// #define MMPF_DECODER_OUTPUT_FORMAT_NV12           // NV12, Default, TI
// #define MMPF_DECODER_OUTPUT_FORMAT_YUV420P        // YUV420P, NVIDIA
#endif

// Display color format Option
//    this definition is used at SW video renderer

#if defined(MMPF_CHIPSET_VENDOR_QCT)
// #define MMPF_VIDEORENDERER_COLORFORMAT_RGB888         // QCT
#define MMPF_VIDEORENDERER_COLORFORMAT_YUV420  // MSM8974
#elif defined(MMPF_CHIPSET_VENDOR_MTK)
#define MMPF_VIDEORENDERER_COLORFORMAT_RGB888  // MTK
#elif defined(MMPF_CHIPSET_VENDOR_TI)
#define MMPF_VIDEORENDERER_COLORFORMAT_RGB888  // NV12, Default, TI
#elif defined(MMPF_CHIPSET_VENDOR_NVIDIA)
#define MMPF_VIDEORENDERER_COLORFORMAT_RGB888  // YUV420P, NVIDIA
#else
#define MMPF_VIDEORENDERER_COLORFORMAT_RGB888  // NV12T, I project, MSM8660
#endif

// Camera output color format
#if defined(MMPF_CHIPSET_VENDOR_MTK)
#define MMPF_CAMERA_OUTPUT_FORMAT_NV21
#else
#define MMPF_CAMERA_OUTPUT_FORMAT_NV12
#endif

// align
#define MMPF_BUFFER_ALIGN128(a)         (((a) + 127) >> 7 << 7)
#define MMPF_BUFFER_ALIGN64(a)          (((a) + 64) >> 6 << 6)
#define MMPF_BUFFER_ALIGN32(a)          (((a) + 31) >> 5 << 5)
#define MMPF_BUFFER_ALIGN16(a)          (((a) + 15) >> 4 << 4)

// --align macro.
#define MMPF_MEDIA_ALIGN(__sz, __align) (((__sz) + (__align - 1)) & (~(__align - 1)))

// Camera output memory align

#if defined(MMPF_CHIPSET_VENDOR_QCT)
#if defined(MMPF_CHIPSET_CATEGORY_DEFAULT) || defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8x26_W) ||      \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8926) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8994) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8992) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8909) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8996) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8916) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8952) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8937) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8976) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8917) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8940) ||                                              \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8998) || defined(MMPF_CHIPSET_CATEGORY_QCT_SDM845) || \
        defined(MMPF_CHIPSET_CATEGORY_QCT_SDM450) || defined(MMPF_CHIPSET_CATEGORY_QCT_SM8150) ||  \
        defined(MMPF_CHIPSET_CATEGORY_QCT_SM6150) || defined(MMPF_CHIPSET_CATEGORY_QCT_SM8250) ||  \
        defined(MMPF_CHIPSET_CATEGORY_QCT_SM7250) || defined(MMPF_CHIPSET_CATEGORY_QCT_SM6115)
// MSM8974, MSM8x26 JB & Kitkat
#define MMPF_CAMERA_OUTPUT_ALIGN(a)             (((a) + 127) >> 7 << 7)
// MSM8974, MSM8x26 JB & Kitkat
#define MMPF_CAMERA_OUTPUT_HALF_ALIGN(a)        (((a) + 63) >> 6 << 6)
// MSM8974, MSM8x26 JB & Kitkat - In Doc, height align is 64byte.
// But actual height align is 32bytes
#define MMPF_CAMERA_OUTPUT_HEIGHT_ALIGN(a)      (((a) + 31) >> 5 << 5)
// MSM8974, MSM8x26 JB & Kitkat
#define MMPF_CAMERA_OUTPUT_HEIGHT_HALF_ALIGN(a) (((a) + 15) >> 4 << 4)

#define MMPF_CAMERA_OUTPUT_UV_OFFSET(a)         (a)
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_FUSION3) || defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8960)
#define MMPF_CAMERA_OUTPUT_ALIGN(a)             (a)  // APQ8064 kitkat
#define MMPF_CAMERA_OUTPUT_HALF_ALIGN(a)        (a)  // APQ8064 kitkat

#define MMPF_CAMERA_OUTPUT_HEIGHT_ALIGN(a)      (a)  // APQ8064 kitkat
#define MMPF_CAMERA_OUTPUT_HEIGHT_HALF_ALIGN(a) (a)  // APQ8064 kitkat
// APQ8064 kitkat - 2k airvineined
#define MMPF_CAMERA_OUTPUT_UV_OFFSET(a)         (((a) + 2047) >> 11 << 11)
#else
#define MMPF_CAMERA_OUTPUT_ALIGN(a)             (a)
#define MMPF_CAMERA_OUTPUT_HALF_ALIGN(a)        (a)

#define MMPF_CAMERA_OUTPUT_HEIGHT_ALIGN(a)      (a)
#define MMPF_CAMERA_OUTPUT_HEIGHT_HALF_ALIGN(a) (a)

#define MMPF_CAMERA_OUTPUT_UV_OFFSET(a)         (a)
#endif
#elif defined(MMPF_CHIPSET_VENDOR_MTK)
#define MMPF_CAMERA_OUTPUT_ALIGN(a)             (((a) + 31) >> 5 << 5)
#define MMPF_CAMERA_OUTPUT_HALF_ALIGN(a)        (((a) + 16) >> 4 << 4)

#define MMPF_CAMERA_OUTPUT_HEIGHT_ALIGN(a)      (a)
#define MMPF_CAMERA_OUTPUT_HEIGHT_HALF_ALIGN(a) (a)

#define MMPF_CAMERA_OUTPUT_UV_OFFSET(a)         (a)
#elif defined(MMPF_CHIPSET_VENDOR_XXX)
#define MMPF_CAMERA_OUTPUT_ALIGN(a)             (a)
#define MMPF_CAMERA_OUTPUT_HALF_ALIGN(a)        (a)

#define MMPF_CAMERA_OUTPUT_HEIGHT_ALIGN(a)      (a)
#define MMPF_CAMERA_OUTPUT_HEIGHT_HALF_ALIGN(a) (a)

#define MMPF_CAMERA_OUTPUT_UV_OFFSET(a)         (a)
#else
#define MMPF_CAMERA_OUTPUT_ALIGN(a)      (a)
#define MMPF_CAMERA_OUTPUT_HALF_ALIGN(a) (a)

#define MMPF_CAMERA_OUTPUT_UV_OFFSET(a)  (a)
#endif

// Encoder input memory align

#if defined(MMPF_CHIPSET_VENDOR_QCT)
#if defined(__MMPF_ANDROID_4_4__)
#define MMPF_VIDEOENCODER_INPUT_ALIGN(a)        (a)  // MSM8974, MSM8x26 Kitkat
#define MMPF_VIDEOENCODER_INPUT_HALF_ALIGN(a)   (a)  // MSM8974, MSM8x26 Kitkat

#define MMPF_VIDEOENCODER_INPUT_HEIGHT_ALIGN(a) (a)  // MSM8974, MSM8x26 Kitkat
#else
// MSM8974 JB
#define MMPF_VIDEOENCODER_INPUT_ALIGN(a)        (((a) + 127) >> 7 << 7)
#define MMPF_VIDEOENCODER_INPUT_HALF_ALIGN(a)   (((a) + 63) >> 6 << 6)

#define MMPF_VIDEOENCODER_INPUT_HEIGHT_ALIGN(a) (((a) + 31) >> 5 << 5)
#endif
#elif defined(MMPF_CHIPSET_VENDOR_XXX)
#define MMPF_VIDEOENCODER_INPUT_ALIGN(a)        (((a) + 31) >> 5 << 5)
#define MMPF_VIDEOENCODER_INPUT_HALF_ALIGN(a)   (((a) + 15) >> 4 << 4)

#define MMPF_VIDEOENCODER_INPUT_HEIGHT_ALIGN(a) (((a) + 31) >> 5 << 5)
#elif defined(MMPF_CHIPSET_VENDOR_MTK)
#define MMPF_VIDEOENCODER_INPUT_ALIGN(a)        (a)
#define MMPF_VIDEOENCODER_INPUT_HALF_ALIGN(a)   (a)
#define MMPF_VIDEOENCODER_INPUT_HEIGHT_ALIGN(a) (a)
#else
#define MMPF_VIDEOENCODER_INPUT_ALIGN(a) (a)
#endif

// Video Codec Buffer Size

#define MMPF_VIDEOCODEC_USE_MAXBUFFER
#define MMPF_VIDEOCODEC_MAXBUFFER_WIDTH  640  // VGA
#define MMPF_VIDEOCODEC_MAXBUFFER_HEIGHT 480  // VGA

// SPS/PPS
#if defined(MMPF_CHIPSET_VENDOR_QCT)
#if defined(MMPF_CHIPSET_CATEGORY_QCT_FUSION3)
#define MMPF_SPS_PPS_QCT_MSM8960_APQ8064
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8x26_W)
#define MMPF_SPS_PPS_QCT_MSM8X26_W
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8960)
#define MMPF_SPS_PPS_QCT_MSM8960
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8909)
#define MMPF_SPS_PPS_QCT_MSM8909
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8937)
#define MMPF_SPS_PPS_QCT_MSM8937
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8917)
#define MMPF_SPS_PPS_QCT_MSM8917
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8940)
#define MMPF_SPS_PPS_QCT_MSM8940
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_SDM450)
#define MMPF_SPS_PPS_QCT_SDM450
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_SM8150)
#define MMPF_SPS_PPS_QCT_SM8150
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_SM6150)
#define MMPF_SPS_PPS_QCT_SM6150
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_SM8250)
#define MMPF_SPS_PPS_QCT_SM8250
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_SM7250)
#define MMPF_SPS_PPS_QCT_SM7250
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_SM6115)
#define MMPF_SPS_PPS_QCT_SM6115
#else
#define MMPF_SPS_PPS_QCT_DEFAULT
#endif
#elif defined(MMPF_CHIPSET_VENDOR_XXX)
#define MMPF_SPS_PPS_XXX_GH14
#elif defined(MMPF_CHIPSET_VENDOR_MTK)
#if defined(MMPF_CHIPSET_CATEGORY_MTK_MT6753)
#define MMPF_SPS_PPS_MTK_MT6753
#elif defined(MMPF_CHIPSET_CATEGORY_MTK_MT6883)
#define MMPF_SPS_PPS_MTK_MT6883  // Todo : need to verification at mt6883 chipset
#else
#define MMPF_SPS_PPS_MTK
#endif
#else
// #error SPS/PPS is not defined!!!
#endif

// H.265 VPS/SPS/PPS
#if defined(MMPF_CHIPSET_VENDOR_QCT)
#define MMPF_VPS_SPS_PPS_QCT_DEFAULT
#endif

// Video H/W Codec
#if defined(MMPF_CHIPSET_VENDOR_QCT) || defined(MMPF_CHIPSET_VENDOR_)
#if defined(MMPF_CHIPSET_CATEGORY_QCT_FUSION3) || defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8960)
#define MMPF_VIDEOCODEC_QCT_OLD  // MSM8960, APQ8064
#else
#define MMPF_VIDEOCODEC_QCT_VENUS  // MSM8974
#endif
#endif

// Rorate option
//     enable this option if HW rotation is not available
//     default - on
//     QCT - Use Camera Rotation
#define USE_MMPF_ROTATION
#if defined(MMPF_CHIPSET_VENDOR_QCT)
#if defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8998) || defined(MMPF_CHIPSET_CATEGORY_QCT_SDM845) ||    \
        defined(MMPF_CHIPSET_CATEGORY_QCT_SM8150) || defined(MMPF_CHIPSET_CATEGORY_QCT_SM6150) || \
        defined(MMPF_CHIPSET_CATEGORY_QCT_SM8250) || defined(MMPF_CHIPSET_CATEGORY_QCT_SM7250) || \
        defined(MMPF_CHIPSET_CATEGORY_QCT_SM6115)
#define USE_OMX_ROTATION
#else
#define USE_CAMERA_ROTATION
#endif
#elif defined(MMPF_CHIPSET_VENDOR_MTK)
#define USE_OMX_ROTATION
#else
#define USE_CAMERA_ROTATION
#endif

// MMPF Rate Control
//     If we can't control the frame-rate of camera, then use USE_MMPF_FRAME_RATE_CONTROL
//     default - off
#if defined(MMPF_CHIPSET_CATEGORY_QCT_FUSION3) || defined(MMPF_CHIPSET_VENDOR_) || \
        defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8909)  // || defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8992)
#define USE_MMPF_FRAME_RATE_CONTROL
#endif

// call AudioSystem::setPhoneState
// call AudioSystem::setPhoneState in mmpf
// This definition is used for MPCS VoLTE//D1L project, other project doesn't use this definition
// default - off
// #define CALL_AUDIOSYSTEM_SETPHONE_STATE

// RTCP Ind format
//    Include Raw data of RTCP packet when mmpf send RTCP indication to client
//    default - off
// #define INCLUDE_RTCP_RAW_DATA_IN_RTCP_IND

// Support H.264 Recording
#define MMPF_USE_STAGEFRIGHT_MPEG4WRITER

// YUV Format for video renderer
#if defined(MMPF_CHIPSET_VENDOR_QCT)
#define UV_ALIGN(a) (((a) + 2047) >> 11 << 11)  // QCT
#else
#define UV_ALIGN(a) (a)  // NVIDIA, TI
#endif

#if defined(MMPF_CHIPSET_VENDOR_NVIDIA)
#define YUV_LINE_ALIGN(a) (a)  // NVIDIA,
#else
#define YUV_LINE_ALIGN(a) (((a) + 31) >> 5 << 5)  // QCT, TI(front camera only)
#endif

#if defined(MMPF_CHIPSET_VENDOR_NVIDIA) || defined(MMPF_CHIPSET_VENDOR_XXX)
#define YUV_HEIGHT_ALIGN(a) (((a) + 31) >> 5 << 5)  // NVIDIA
#else
#define YUV_HEIGHT_ALIGN(a) (a)  // QCT, TI
#endif

#if defined(MMPF_CHIPSET_VENDOR_NVIDIA)
#define YUV_STRIDE_CAMERA 6400  // nvidia, for qct and ti off this definition
#endif

// Master port setting for CP MMPF
//    - set MMPF qmi device id string
//    - defined by qmi_ril_client_get_master_port()
//      in vendor/qcom/proprietary/qcril/qcril_qmi/qcril_qmi_client.c

#if defined(MMPF_CHIPSET_CATEGORY_QCT_FUSION3)
#define MMPF_QMI_DEV_ID_STRING "rmnet_usb0"  // APQ8064(G)
#elif defined(MMPF_CHIPSET_CATEGORY_QCT_MSM8960)
#define MMPF_QMI_DEV_ID_STRING "rmnet0"  // MSM8960(Vu2)
#else
#define MMPF_QMI_DEV_ID_STRING "rmnet0"  // MSM8960(Vu2)
#endif

// Debugging Features
#ifdef MMPF_DEBUG_BUILD
// send 'tx side rtp encoder packet' to 'rx side rtp decoder' directly in MMPFRTPSession,
// and disable socket
// #define MMPF_DEBUG_LOOPBACK_TEST

// Force to Generate Jitter re-ordering case at TX side
// #define MMPF_DEBUG_JITTER_GEN_SIMULATION_REORDER
// Force to Generate Jitter delay case at TX side
// #define MMPF_DEBUG_JITTER_GEN_SIMULATION_DELAY
// Force to Generate Packet loss case at TX side
// #define MMPF_DEBUG_JITTER_GEN_SIMULATION_LOSS

// #define MMPF_DEBUG_HEAP                           // check heap

// #define MMPF_DEBUG_DUMP_CAMERA_OUTPUT             // YUV
// #define MMPF_DEBUG_DUMP_CAMERA_OUTPUT_SEQUENCE
// #define MMPF_DEBUG_DUMP_TX_VIDEO_BITSTREAM        // tx encoded bitstream
// #define MMPF_DEBUG_DUMP_VIDEO_DECODER_INPUT       // rx encoded bitstream
// #define MMPF_DEBUG_DUMP_VIDEO_DECODER_OUTPUT      // YUV
// #define MMPF_DEBUG_DUMP_VIDEO_RENDERER_RGB        // RGB565

#endif

// Enable/Disable MMPF Logs

// force to disable MMPFLOGD, This define is prior to SetConfigDB(MMPF_SETCONFIG_TRACE, ...)
// #define MMPF_FORCE_TO_DISABLE_LOGD
// force to disable MMPFLOGD_PACKET, This define is prior to SetConfigDB(MMPF_SETCONFIG_TRACE, ...)
// #define MMPF_FORCE_TO_DISABLE_PACKET_LOG

#endif  // MMPF_MMPFCONFIGURE_H_INCLUDED
