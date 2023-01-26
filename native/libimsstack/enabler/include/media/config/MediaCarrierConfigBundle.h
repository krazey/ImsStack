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

#ifndef MEDIA_CARRIER_CONFIG_BUNDLE_H_
#define MEDIA_CARRIER_CONFIG_BUNDLE_H_

#include "ImsTypeDef.h"
#include "AString.h"
#include "ImsVector.h"
#include "CarrierConfig.h"

// Public VoLTE
struct MediaAudioCodecCapabilityPayloadTypesBundle
{
public:
    MediaAudioCodecCapabilityPayloadTypesBundle() :
            // KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE
            objEvsPayloadType(IMSVector<IMS_SINT32>()),     // KEY_EVS_PAYLOAD_TYPE_INT_ARRAY
            objAmrwbPayloadType(IMSVector<IMS_SINT32>()),   // KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY
            objAmrnbPayloadType(IMSVector<IMS_SINT32>()),   // KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY
            objDtmfwbPayloadType(IMSVector<IMS_SINT32>()),  // KEY_DTMFWB_PAYLOAD_TYPE_INT_ARRAY
            objDtmfnbPayloadType(IMSVector<IMS_SINT32>())   // KEY_DTMFNB_PAYLOAD_TYPE_INT_ARRAY
    {
    }

    MediaAudioCodecCapabilityPayloadTypesBundle(
            IN const MediaAudioCodecCapabilityPayloadTypesBundle&) = delete;
    MediaAudioCodecCapabilityPayloadTypesBundle& operator=(
            IN const MediaAudioCodecCapabilityPayloadTypesBundle&) = delete;

public:
    IMSVector<IMS_SINT32> objEvsPayloadType;
    IMSVector<IMS_SINT32> objAmrwbPayloadType;
    IMSVector<IMS_SINT32> objAmrnbPayloadType;
    IMSVector<IMS_SINT32> objDtmfwbPayloadType;
    IMSVector<IMS_SINT32> objDtmfnbPayloadType;
};

struct MediaAmrnbPayloadDescriptionBundle
{
public:
    MediaAmrnbPayloadDescriptionBundle() :
            // KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE
            nAmrnbCodecAttributePayloadFormat(CarrierConfig::ImsVoice::AMR_BANDWIDTH_EFFICIENT),
            // KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT
            objAmrnbCodecAttributeModeset(IMSVector<IMS_SINT32>())
    // KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY
    {
    }

    MediaAmrnbPayloadDescriptionBundle(IN const MediaAmrnbPayloadDescriptionBundle&) = delete;
    MediaAmrnbPayloadDescriptionBundle& operator=(
            IN const MediaAmrnbPayloadDescriptionBundle&) = delete;

public:
    IMS_SINT32 nAmrnbCodecAttributePayloadFormat;
    IMSVector<IMS_SINT32> objAmrnbCodecAttributeModeset;
};

struct MediaAmrwbPayloadDescriptionBundle
{
public:
    MediaAmrwbPayloadDescriptionBundle() :
            // KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE
            nAmrwbCodecAttributePayloadFormat(CarrierConfig::ImsVoice::AMR_BANDWIDTH_EFFICIENT),
            // KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT
            objAmrwbCodecAttributeModeset(IMSVector<IMS_SINT32>())
    // KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY
    {
    }

    MediaAmrwbPayloadDescriptionBundle(IN const MediaAmrwbPayloadDescriptionBundle&) = delete;
    MediaAmrwbPayloadDescriptionBundle& operator=(
            IN const MediaAmrwbPayloadDescriptionBundle&) = delete;

public:
    IMS_SINT32 nAmrwbCodecAttributePayloadFormat;
    IMSVector<IMS_SINT32> objAmrwbCodecAttributeModeset;
};

struct MediaEvsPayloadDescriptionBundle
{
public:
    MediaEvsPayloadDescriptionBundle() :
            // KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE
            nEvsCodecAttributeModeSwitch(0),  // KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT
            nEvsCodecAttributeBandwidth(0),   // KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT
            objEvsCodecAttributeBitrate(IMSVector<IMS_SINT32>()),
            // KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY
            nEvsCodecAttributeChAwRecv(0),         // KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT
            nEvsCodecAttributeHfOnly(0),           // KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT
            bEvsCodecAttributeDtx(IMS_FALSE),      // KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL
            bEvsCodecAttributeDtxRecv(IMS_FALSE),  // KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL
            nEvsCodecAttributeChannels(0),         // KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT
            nEvsCodecAttributeCmr(0),              // KEY_EVS_CODEC_ATTRIBUTE_CMR_INT
            nCodecAttributeModeChangePeriod(1),    // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT
            nCodecAttributeModeChangeCapability(
                    1),                           // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT
            nCodecAttributeModeChangeNeighbor(0)  // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT
    {
    }

    MediaEvsPayloadDescriptionBundle(IN const MediaEvsPayloadDescriptionBundle&) = delete;
    MediaEvsPayloadDescriptionBundle& operator=(
            IN const MediaEvsPayloadDescriptionBundle&) = delete;

public:
    IMS_SINT32 nEvsCodecAttributeModeSwitch;            // KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT
    IMS_SINT32 nEvsCodecAttributeBandwidth;             // KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT
    IMSVector<IMS_SINT32> objEvsCodecAttributeBitrate;  // KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY
    IMS_SINT32 nEvsCodecAttributeChAwRecv;              // KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT
    IMS_SINT32 nEvsCodecAttributeHfOnly;                // KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT
    IMS_BOOL bEvsCodecAttributeDtx;                     // KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL
    IMS_BOOL bEvsCodecAttributeDtxRecv;                 // KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL
    IMS_SINT32 nEvsCodecAttributeChannels;              // KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT
    IMS_SINT32 nEvsCodecAttributeCmr;                   // KEY_EVS_CODEC_ATTRIBUTE_CMR_INT
    IMS_SINT32 nCodecAttributeModeChangePeriod;  // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT
    IMS_SINT32
            nCodecAttributeModeChangeCapability;   // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT
    IMS_SINT32 nCodecAttributeModeChangeNeighbor;  // KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT
};

// AOSP RTT
struct MediaTextCodecCapabilityPayloadTypesBundle
{
public:
    MediaTextCodecCapabilityPayloadTypesBundle() :
            // KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE
            m_nT140PayloadType(111),  // KEY_T140_PAYLOAD_TYPE_INT
            m_nRedPayloadType(112)    // KEY_RED_PAYLOAD_TYPE_INT
    {
    }

    MediaTextCodecCapabilityPayloadTypesBundle(
            IN const MediaTextCodecCapabilityPayloadTypesBundle&) = delete;
    MediaTextCodecCapabilityPayloadTypesBundle& operator=(
            IN const MediaTextCodecCapabilityPayloadTypesBundle&) = delete;

public:
    IMS_SINT32 m_nT140PayloadType;
    IMS_SINT32 m_nRedPayloadType;
};

// AOSP VT
struct MediaVideoCodecCapabilityPayloadTypesBundle
{
public:
    MediaVideoCodecCapabilityPayloadTypesBundle() :
            // KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE
            objAvcPayloadType(IMSVector<IMS_SINT32>())  // KEY_H264_PAYLOAD_TYPE_INT_ARRAY
    {
    }

    MediaVideoCodecCapabilityPayloadTypesBundle(
            IN const MediaVideoCodecCapabilityPayloadTypesBundle&) = delete;
    MediaVideoCodecCapabilityPayloadTypesBundle& operator=(
            IN const MediaVideoCodecCapabilityPayloadTypesBundle&) = delete;

public:
    IMSVector<IMS_SINT32> objAvcPayloadType;
};

struct MediaH264PayloadDescriptionBundle
{
public:
    MediaH264PayloadDescriptionBundle() :
            // KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE
            nVideoCodecAttributePacketizationMode(1),
            // KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT
            nVideoCodecAttributeFrameRate(15),  // KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT
            objVideoCodecAttributeResolution(IMSVector<IMS_SINT32>()),
            // KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY
            strH264VideoCodecAttributeProfileLevelId(AString::ConstEmpty())
    // KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING
    {
    }

    MediaH264PayloadDescriptionBundle(IN const MediaH264PayloadDescriptionBundle&) = delete;
    MediaH264PayloadDescriptionBundle& operator=(
            IN const MediaH264PayloadDescriptionBundle&) = delete;

public:
    IMS_SINT32 nVideoCodecAttributePacketizationMode;
    IMS_SINT32 nVideoCodecAttributeFrameRate;
    IMSVector<IMS_SINT32> objVideoCodecAttributeResolution;
    AString strH264VideoCodecAttributeProfileLevelId;
};

#endif
