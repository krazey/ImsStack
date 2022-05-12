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

#include "ServiceTrace.h"
#include "config/CodecEvsConfig.h"

__IMS_TRACE_TAG_USER_DECL__("MED.CONF");

/*

Remarks

*/
PUBLIC
CodecEvsConfig::CodecEvsConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_) :
        CodecConfig(nType_, nPayloadTypeNum_),
        m_nChannel(DEFAULT_CHANNEL),
        m_bDtx(DEFAULT_DTX),
        m_bDtxRecv(DEFAULT_DTX_RECV),
        m_nHfOnly(DEFAULT_HF_ONLY),
        m_nEvsModeSwitch(DEFAULT_EVS_MODESWITCH),
        m_nBrList(DEFAULT_BR_LIST),
        m_nBwList(DEFAULT_BW_LIST),
        m_nCmr(DEFAULT_CMR),
        m_nChAwRecv(DEFAULT_CH_AW_RECV),
        m_nModeSetList(DEFAULT_AMRWB_IO_MODESET)
{
    IMS_TRACE_D("+CodecEvsConfig Type[%d]", nType_, 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL CodecEvsConfig::~CodecEvsConfig()
{
    IMS_TRACE_D("~CodecEvsConfig", 0, 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL CodecEvsConfig::Create(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piBuffer is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    ICarrierConfig* piCcBundle =
            piCc->GetBundle(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piCcBundle is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strPayloadTypeNumber;
    strPayloadTypeNumber.SetNumber(m_nPayloadType);
    ICarrierConfig* piCcSubBundle = piCcBundle->GetBundle(strPayloadTypeNumber.GetStr());

    if (piCcSubBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piCcSubBundle is NULL", 0, 0, 0);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;
        return IMS_FALSE;
    }

    m_nChannel = piCcBundle->GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT);
    m_bDtx = piCcBundle->GetBoolean(
            CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL, IMS_TRUE);
    m_bDtxRecv = piCcBundle->GetBoolean(
            CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL, IMS_TRUE);
    m_nHfOnly = piCcBundle->GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT);
    m_nEvsModeSwitch =
            piCcBundle->GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT);
    IMSVector<IMS_SINT32> objBitrateList = piCcBundle->GetIntArray(
            CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY);

    IMS_SINT32 nBrStart = DEFAULT_BR_LIST;
    IMS_SINT32 nBrEnd = DEFAULT_BR_LIST;

    if (!objBitrateList.IsEmpty())
    {
        nBrStart = objBitrateList.GetAt(0);

        if (objBitrateList.GetSize() > 1)
        {
            nBrEnd = objBitrateList.GetAt(1);
        }
    }

    m_nBrList = ConvertEvsBitrateToList(nBrStart, nBrEnd);
    m_nBwList = piCcBundle->GetInt(
            CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT, DEFAULT_BW_LIST);
    m_nCmr = piCcBundle->GetInt(
            CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CMR_INT, CMR_NOT_PRESENT);
    m_nChAwRecv =
            piCcBundle->GetInt(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT);
    // TODO_MEDIA
    //    m_nModeSetList = piCcBundle->GetString(
    //            CarrierConfig::ImsVoice::KEY_EVS_AMRWB_IO_MODE_SET_INT);
    piCcSubBundle->ReleaseBundle();
    piCcSubBundle = IMS_NULL;

    piCcBundle->ReleaseBundle();
    piCcBundle = IMS_NULL;

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL void CodecEvsConfig::ToDebugString() const
{
    CodecConfig::ToDebugString();

    IMS_TRACE_D("nChannel(%d), bDtx(%d), bDtxRecv(%d)", m_nChannel, m_bDtx, m_bDtxRecv);
    IMS_TRACE_D("nHfOnly(%d), nEvsModeSwitch(%d)", m_nHfOnly, m_nEvsModeSwitch, 0);
    IMS_TRACE_D("nBrList(0x%04x), nBwList(%d)", m_nBrList, m_nBwList, 0);
    IMS_TRACE_D("nCmr(%d), nChAwRecv(%d), nModeSetList(%d)", m_nCmr, m_nChAwRecv, m_nModeSetList);
}

PRIVATE
IMS_UINT32 CodecEvsConfig::ConvertEvsBitrateToList(
        IN IMS_SINT32 nBrStart, IN IMS_SINT32 nBrEnd) const
{
    IMS_SINT32 nBitrate = 0;
    IMS_UINT32 nBitrateSet = 0;

    IMS_TRACE_D("ConvertEvsBitrateToList nBrStart(%d) nBrEnd(%d)", nBrStart, nBrEnd, 0);

    if (nBrStart < EVS_PRIMARY_MODE_BITRATE_5_9_KBPS ||
            nBrEnd < EVS_PRIMARY_MODE_BITRATE_5_9_KBPS ||
            nBrStart > EVS_PRIMARY_MODE_BITRATE_128_0_KBPS ||
            nBrEnd > EVS_PRIMARY_MODE_BITRATE_128_0_KBPS)
    {
        nBrStart = DEFAULT_BR_LIST;
        nBrEnd = DEFAULT_BR_LIST;
    }

    for (IMS_UINT32 i = nBrStart; i <= nBrEnd; i++)
    {
        nBitrate = i;

        IMS_TRACE_D("ConvertEvsBitrateToList nBitrate (%d) ", nBitrate, 0, 0);
        nBitrateSet = (nBitrateSet | (1 << nBitrate));
    }

    return (IMS_UINT32)nBitrateSet;
}

PRIVATE
IMS_SINT32 CodecEvsConfig::GetEvsBandwidthFromList(IN IMS_UINT32 nBandwidthList) const
{
    if (nBandwidthList == 0)
    {
        return EVS_BANDWIDTH_SWB;
    }

    for (IMS_SINT32 nFindBandwidth = EVS_BANDWIDTH_FB; nFindBandwidth >= 0; nFindBandwidth--)
    {
        if (nBandwidthList & (1 << nFindBandwidth))
        {
            return nFindBandwidth;
        }
    }

    return EVS_BANDWIDTH_SWB;
}

PRIVATE
IMS_SINT32 CodecEvsConfig::GetEvsBitrateFromList(IN IMS_UINT32 nBitrateList) const
{
    if (nBitrateList == 0)
    {
        return EVS_PRIMARY_MODE_BITRATE_24_4_KBPS;
    }

    for (IMS_SINT32 nFindBitrate = EVS_PRIMARY_MODE_BITRATE_24_4_KBPS; nFindBitrate >= 0;
            nFindBitrate--)
    {
        if (nBitrateList & (1 << nFindBitrate))
        {
            return nFindBitrate;
        }
    }

    return EVS_PRIMARY_MODE_BITRATE_24_4_KBPS;
}

PRIVATE
IMS_SINT32 CodecEvsConfig::GetAmrIoModeSetFromList(IN IMS_UINT32 nAmrIoModeSet) const
{
    if (nAmrIoModeSet == 0)
    {
        return DEFAULT_AMRWB_IO_MODESET;
    }

    for (IMS_SINT32 nFindModeSet = DEFAULT_AMRWB_IO_MODESET; nFindModeSet >= 0; nFindModeSet--)
    {
        if (nAmrIoModeSet & (1 << nFindModeSet))
        {
            return nFindModeSet;
        }
    }

    return DEFAULT_AMRWB_IO_MODESET;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetChannel() const
{
    return m_nChannel;
}

PUBLIC
IMS_BOOL CodecEvsConfig::GetDtx() const
{
    return m_bDtx;
}

PUBLIC
IMS_BOOL CodecEvsConfig::GetDtxRecv() const
{
    return m_bDtxRecv;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetHfOnly() const
{
    return m_nHfOnly;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetEvsModeSwitch() const
{
    return m_nEvsModeSwitch;
}

PUBLIC
IMS_UINT32 CodecEvsConfig::GetBrList() const
{
    return m_nBrList;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetBr() const
{
    return GetEvsBitrateFromList(m_nBrList);
}

PUBLIC
IMS_UINT32 CodecEvsConfig::GetBwList() const
{
    return m_nBwList;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetBw() const
{
    return GetEvsBandwidthFromList(m_nBwList);
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetCmr() const
{
    return m_nCmr;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetChAwareRecv() const
{
    return m_nChAwRecv;
}

PUBLIC
IMS_UINT32 CodecEvsConfig::GetModeSetList() const
{
    return m_nModeSetList;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetModeSet() const
{
    IMS_SINT32 nModeSet;

    if (m_nModeSetList == 0)
    {
        return DEFAULT_AMRWB_IO_MODESET;
    }

    for (nModeSet = DEFAULT_AMRWB_IO_MODESET; nModeSet >= 0; nModeSet--)
    {
        if (m_nModeSetList & (1 << nModeSet))
        {
            return nModeSet;
        }
    }
    return 0;
}
