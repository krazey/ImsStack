/*
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

#ifndef MEDIA_CARRIER_CONFIG_BUNDLE_H_
#define MEDIA_CARRIER_CONFIG_BUNDLE_H_

#include "AString.h"
#include "CarrierConfig.h"
#include "ImsTypeDef.h"
#include "ImsVector.h"

/**
 * @brief Defines the payload types for various audio codec capabilities.
 */
struct AudioCodecCapabilityPayloadTypesBundle
{
public:
    AudioCodecCapabilityPayloadTypesBundle() :
            // KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE
            objEvsPayloadType(ImsVector<IMS_SINT32>()),     // KEY_EVS_PAYLOAD_TYPE_INT_ARRAY
            objAmrwbPayloadType(ImsVector<IMS_SINT32>()),   // KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY
            objAmrnbPayloadType(ImsVector<IMS_SINT32>()),   // KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY
            objDtmfwbPayloadType(ImsVector<IMS_SINT32>()),  // KEY_DTMFWB_PAYLOAD_TYPE_INT_ARRAY
            objDtmfnbPayloadType(ImsVector<IMS_SINT32>())   // KEY_DTMFNB_PAYLOAD_TYPE_INT_ARRAY
    {
    }

    AudioCodecCapabilityPayloadTypesBundle(
            IN const AudioCodecCapabilityPayloadTypesBundle&) = delete;
    AudioCodecCapabilityPayloadTypesBundle& operator=(
            IN const AudioCodecCapabilityPayloadTypesBundle&) = delete;

public:
    ImsVector<IMS_SINT32> objEvsPayloadType;
    ImsVector<IMS_SINT32> objAmrwbPayloadType;
    ImsVector<IMS_SINT32> objAmrnbPayloadType;
    ImsVector<IMS_SINT32> objDtmfwbPayloadType;
    ImsVector<IMS_SINT32> objDtmfnbPayloadType;
};

/**
 * @brief Defines the payload description for the AMR-NB (Narrowband) codec.
 */
struct AmrnbPayloadDescriptionBundle
{
public:
    AmrnbPayloadDescriptionBundle() :
            // KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE
            nAmrnbCodecAttributePayloadFormat(CarrierConfig::ImsVoice::AMR_BANDWIDTH_EFFICIENT),
            // KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT
            objAmrnbCodecAttributeModeset(ImsVector<IMS_SINT32>())
    // KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY
    {
    }

    AmrnbPayloadDescriptionBundle(IN const AmrnbPayloadDescriptionBundle&) = delete;
    AmrnbPayloadDescriptionBundle& operator=(IN const AmrnbPayloadDescriptionBundle&) = delete;

public:
    IMS_SINT32 nAmrnbCodecAttributePayloadFormat;
    ImsVector<IMS_SINT32> objAmrnbCodecAttributeModeset;
};

/**
 * @brief Defines the payload description for the AMR-WB (Wideband) codec.
 */
struct AmrwbPayloadDescriptionBundle
{
public:
    AmrwbPayloadDescriptionBundle() :
            // KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE
            nAmrwbCodecAttributePayloadFormat(CarrierConfig::ImsVoice::AMR_BANDWIDTH_EFFICIENT),
            // KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT
            objAmrwbCodecAttributeModeset(ImsVector<IMS_SINT32>())
    // KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY
    {
    }

    AmrwbPayloadDescriptionBundle(IN const AmrwbPayloadDescriptionBundle&) = delete;
    AmrwbPayloadDescriptionBundle& operator=(IN const AmrwbPayloadDescriptionBundle&) = delete;

public:
    IMS_SINT32 nAmrwbCodecAttributePayloadFormat;
    ImsVector<IMS_SINT32> objAmrwbCodecAttributeModeset;
};

/**
 * @brief Defines the payload description for the EVS (Enhanced Voice Services) codec.
 */
struct EvsPayloadDescriptionBundle
{
public:
    // TODO : the default values will be updated later
    EvsPayloadDescriptionBundle() :
            // KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE
            nEvsCodecAttributeModeSwitch(0),  // KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT
            nEvsCodecAttributeBandwidth(0),   // KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT
            objEvsCodecAttributeBitrate(ImsVector<IMS_SINT32>()),
            // KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY
            nEvsCodecAttributeChAwRecv(0),        // KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT
            nEvsCodecAttributeHfOnly(0),          // KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT
            bEvsCodecAttributeDtx(IMS_TRUE),      // KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL
            bEvsCodecAttributeDtxRecv(IMS_TRUE),  // KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL
            nEvsCodecAttributeChannels(1),        // KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT
            nEvsCodecAttributeCmr(0),             // KEY_EVS_CODEC_ATTRIBUTE_CMR_INT
            nCodecAttributeModeChangePeriod(1),   // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT
            nCodecAttributeModeChangeCapability(
                    1),                           // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT
            nCodecAttributeModeChangeNeighbor(0)  // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT
    {
    }

    EvsPayloadDescriptionBundle(IN const EvsPayloadDescriptionBundle&) = delete;
    EvsPayloadDescriptionBundle& operator=(IN const EvsPayloadDescriptionBundle&) = delete;

public:
    IMS_SINT32 nEvsCodecAttributeModeSwitch;            // KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT
    IMS_SINT32 nEvsCodecAttributeBandwidth;             // KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT
    ImsVector<IMS_SINT32> objEvsCodecAttributeBitrate;  // KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY
    IMS_SINT32 nEvsCodecAttributeChAwRecv;              // KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT
    IMS_SINT32 nEvsCodecAttributeHfOnly;                // KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT
    IMS_BOOL bEvsCodecAttributeDtx;                     // KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL
    IMS_BOOL bEvsCodecAttributeDtxRecv;                 // KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL
    IMS_SINT32 nEvsCodecAttributeChannels;              // KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT
    IMS_SINT32 nEvsCodecAttributeCmr;                   // KEY_EVS_CODEC_ATTRIBUTE_CMR_INT
    IMS_SINT32 nCodecAttributeModeChangePeriod;  // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT
    IMS_SINT32
    nCodecAttributeModeChangeCapability;           // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT
    IMS_SINT32 nCodecAttributeModeChangeNeighbor;  // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT
};

/**
 * @brief Defines the payload types for text codec capabilities, used for Real-Time Text (RTT).
 */
struct TextCodecCapabilityPayloadTypesBundle
{
public:
    TextCodecCapabilityPayloadTypesBundle() :
            // KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE
            m_nT140PayloadType(111),  // KEY_T140_PAYLOAD_TYPE_INT
            m_nRedPayloadType(112)    // KEY_RED_PAYLOAD_TYPE_INT
    {
    }

    TextCodecCapabilityPayloadTypesBundle(IN const TextCodecCapabilityPayloadTypesBundle&) = delete;
    TextCodecCapabilityPayloadTypesBundle& operator=(
            IN const TextCodecCapabilityPayloadTypesBundle&) = delete;

public:
    IMS_SINT32 m_nT140PayloadType;
    IMS_SINT32 m_nRedPayloadType;
};

/**
 * @brief Defines the payload types for video codec capabilities, used for Video Telephony (VT).
 */
struct VideoCodecCapabilityPayloadTypesBundle
{
public:
    VideoCodecCapabilityPayloadTypesBundle() :
            // KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE
            objAvcPayloadType(ImsVector<IMS_SINT32>())  // KEY_H264_PAYLOAD_TYPE_INT_ARRAY
    {
    }

    VideoCodecCapabilityPayloadTypesBundle(
            IN const VideoCodecCapabilityPayloadTypesBundle&) = delete;
    VideoCodecCapabilityPayloadTypesBundle& operator=(
            IN const VideoCodecCapabilityPayloadTypesBundle&) = delete;

public:
    ImsVector<IMS_SINT32> objAvcPayloadType;
};

/**
 * @brief Defines the payload description for the H.264 (AVC) video codec.
 */
struct H264PayloadDescriptionBundle
{
public:
    H264PayloadDescriptionBundle() :
            // KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE
            nVideoCodecAttributePacketizationMode(1),
            // KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT
            nVideoCodecAttributeFrameRate(15),  // KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT
            objVideoCodecAttributeResolution(ImsVector<IMS_SINT32>()),
            // KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY
            strH264VideoCodecAttributeProfileLevelId(AString::ConstEmpty())
    // KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING
    {
    }

    H264PayloadDescriptionBundle(IN const H264PayloadDescriptionBundle&) = delete;
    H264PayloadDescriptionBundle& operator=(IN const H264PayloadDescriptionBundle&) = delete;

public:
    IMS_SINT32 nVideoCodecAttributePacketizationMode;
    IMS_SINT32 nVideoCodecAttributeFrameRate;
    ImsVector<IMS_SINT32> objVideoCodecAttributeResolution;
    AString strH264VideoCodecAttributeProfileLevelId;
};

#endif
