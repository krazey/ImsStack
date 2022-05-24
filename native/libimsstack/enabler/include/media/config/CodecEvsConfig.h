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

#ifndef _CODEC_EVS_CONFIG_H_
#define _CODEC_EVS_CONFIG_H_

#include "AString.h"
#include "config/CodecConfig.h"

class CodecEvsConfig : public CodecConfig
{
private:
    // EVS FrameType
    enum
    {
        EVS_PRIMARY_MODE_BITRATE_5_9_KBPS = 0,
        EVS_PRIMARY_MODE_BITRATE_7_2_KBPS = 1,
        EVS_PRIMARY_MODE_BITRATE_8_0_KBPS = 2,
        EVS_PRIMARY_MODE_BITRATE_9_6_KBPS = 3,
        EVS_PRIMARY_MODE_BITRATE_13_2_KBPS = 4,
        EVS_PRIMARY_MODE_BITRATE_16_4_KBPS = 5,
        EVS_PRIMARY_MODE_BITRATE_24_4_KBPS = 6,
        EVS_PRIMARY_MODE_BITRATE_32_0_KBPS = 7,
        EVS_PRIMARY_MODE_BITRATE_48_0_KBPS = 8,
        EVS_PRIMARY_MODE_BITRATE_64_0_KBPS = 9,
        EVS_PRIMARY_MODE_BITRATE_96_0_KBPS = 10,
        EVS_PRIMARY_MODE_BITRATE_128_0_KBPS = 11,
        EVS_PRIMARY_MODE_BITRATE_MAX = EVS_PRIMARY_MODE_BITRATE_128_0_KBPS
    };

public:
    enum
    {
        EVS_ENCODED_BW_TYPE_NB = 0,
        EVS_ENCODED_BW_TYPE_WB = 1,
        EVS_ENCODED_BW_TYPE_SWB = 2,
        EVS_ENCODED_BW_TYPE_FB = 3,
        EVS_ENCODED_BW_TYPE_NB_WB = 4,
        EVS_ENCODED_BW_TYPE_NB_WB_SWB = 5,
        EVS_ENCODED_BW_TYPE_NB_WB_SWB_FB = 6,
        EVS_ENCODED_BW_TYPE_WB_SWB = 7,
        EVS_ENCODED_BW_TYPE_WB_SWB_FB = 8
    };
    enum
    {
        EVS_BANDWIDTH_NB = 0,
        EVS_BANDWIDTH_WB = 1,
        EVS_BANDWIDTH_SWB = 2,
        EVS_BANDWIDTH_FB = 3,
        EVS_BANDWIDTH_MAX = EVS_BANDWIDTH_FB
    };

public:
    static const IMS_SINT32 DEFAULT_CHANNEL = 1;  // 1 == mono
    static const IMS_BOOL DEFAULT_DTX = IMS_TRUE;
    static const IMS_BOOL DEFAULT_DTX_RECV = IMS_TRUE;
    static const IMS_SINT32 DEFAULT_HF_ONLY = 0;
    static const IMS_SINT32 DEFAULT_EVS_MODESWITCH = 0;
    static const IMS_SINT32 DEFAULT_BR = EVS_PRIMARY_MODE_BITRATE_24_4_KBPS;
    static const IMS_SINT32 DEFAULT_BR_LIST = 1 << DEFAULT_BR;
    static const IMS_SINT32 DEFAULT_BW_LIST = EVS_ENCODED_BW_TYPE_NB_WB_SWB;
    static const IMS_SINT32 DEFAULT_CMR = 0;
    static const IMS_SINT32 DEFAULT_CH_AW_RECV = 0;
    static const IMS_SINT32 DEFAULT_AMRWB_IO_MODESET = 8;
    static const IMS_SINT32 CMR_NOT_PRESENT = -2;

public:
    CodecEvsConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_);
    virtual ~CodecEvsConfig();

public:
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    virtual void ToDebugString() const;

private:
    IMS_UINT32 ConvertEvsBitrateToList(IN IMS_SINT32 nBrStart, IN IMS_SINT32 nBrEnd) const;
    IMS_UINT32 ConvertEvsBandwidthToList(IN AString strBandwidth) const;
    IMS_SINT32 GetEvsBandwidthFromList(IN IMS_UINT32 nBandwidthList) const;
    IMS_SINT32 GetEvsBitrateFromList(IN IMS_UINT32 nBitrateList) const;
    IMS_SINT32 GetAmrIoModeSetFromList(IN IMS_UINT32 nAmrIoModeSet) const;

public:
    IMS_SINT32 GetChannel() const;
    IMS_BOOL GetDtx() const;
    IMS_BOOL GetDtxRecv() const;
    IMS_SINT32 GetHfOnly() const;
    IMS_SINT32 GetEvsModeSwitch() const;
    IMS_UINT32 GetBrList() const;
    IMS_SINT32 GetBr() const;
    IMS_UINT32 GetBwList() const;
    IMS_SINT32 GetBw() const;
    IMS_SINT32 GetCmr() const;
    IMS_SINT32 GetChAwareRecv() const;
    IMS_UINT32 GetModeSetList() const;
    IMS_SINT32 GetModeSet() const;

private:
    IMS_SINT32 m_nChannel;

    IMS_BOOL m_bDtx;              // 1(default) is turn on DTX
    IMS_BOOL m_bDtxRecv;          // 1(default) is turn on DTX
    IMS_SINT32 m_nHfOnly;         // 0(default) is compact and hf format used,
                                  //  other is only hf format used
    IMS_SINT32 m_nEvsModeSwitch;  // 0(default) is "primary mode start"

    IMS_UINT32 m_nBrList;  // EVS primary mode bitrate range (kbps)
    IMS_UINT32 m_nBwList;  // bw has a value from the set:
                           // nb, wb, swb, fb, nb-wb, nb-swb, and nb-fb. nb, wb, swb, fb
    IMS_SINT32 m_nCmr;
    IMS_SINT32 m_nChAwRecv;  // -1 is channel aware mode disable,
                             // 0(default) is not used at the start of the session,
                             // but it'll be changed using CMR or RTCP app.
    // AMR-WB IO parameter
    IMS_UINT32 m_nModeSetList;
};
#endif  // _CODEC_EVS_CONFIG_H_
