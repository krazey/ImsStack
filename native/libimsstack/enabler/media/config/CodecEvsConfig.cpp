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

#include "ServiceTrace.h"
#include "config/CodecEvsConfig.h"

__IMS_TRACE_TAG_MEDIA__;

const IMS_BOOL CodecEvsConfig::DEFAULT_DTX_RECV = IMS_TRUE;
const IMS_SINT32 CodecEvsConfig::DEFAULT_HF_ONLY = 0;
const IMS_SINT32 CodecEvsConfig::DEFAULT_EVS_MODESWITCH = 0;  // EVS_OPERATIONAL_MODE_PRIMARY
const IMS_SINT32 CodecEvsConfig::DEFAULT_BR = CodecEvsConfig::EVS_PRIMARY_MODE_BITRATE_24_4_KBPS;
const IMS_SINT32 CodecEvsConfig::DEFAULT_BR_LIST = 1 << DEFAULT_BR;
const IMS_SINT32 CodecEvsConfig::DEFAULT_BW_LIST = CodecEvsConfig::EVS_ENCODED_BW_TYPE_NB_WB_SWB;
const IMS_SINT32 CodecEvsConfig::DEFAULT_CMR = 0;
const IMS_SINT32 CodecEvsConfig::DEFAULT_CH_AW_RECV = 0;
const IMS_SINT32 CodecEvsConfig::CMR_NOT_PRESENT = -2;
const IMS_SINT32 CodecEvsConfig::NOT_DEFINED = -2;

PUBLIC
CodecEvsConfig::CodecEvsConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
        CodecAudioConfig(nType, nPayloadTypeNum),
        m_bVisibleDtx(IMS_FALSE),
        m_bVisibleHfOnly(IMS_FALSE),
        m_bVisibleCmr(IMS_FALSE),
        m_bVisibleEvsModeSwitch(IMS_FALSE),
        m_bVisibleChannelAwMode(IMS_FALSE),
        m_bVisibleAmrIOModeSet(IMS_FALSE),
        m_bVisibleChAwRecv(IMS_FALSE),
        m_bDtxRecv(DEFAULT_DTX_RECV),
        m_nHfOnly(DEFAULT_HF_ONLY),
        m_nEvsModeSwitch(DEFAULT_EVS_MODESWITCH),
        m_nBrList(DEFAULT_BR_LIST),
        m_nBwList(DEFAULT_BW_LIST),
        m_nCmr(DEFAULT_CMR),
        m_nAmrIOModeSetList(DEFAULT_MODESET_AMRWB),
        m_nDefaultAmrIOModeSetList(DEFAULT_MODESET_AMRWB),
        m_nChAwRecv(DEFAULT_CH_AW_RECV)
{
    IMS_TRACE_I("+CodecEvsConfig - Type[%d]", nType, 0, 0);
}

PUBLIC VIRTUAL CodecEvsConfig::~CodecEvsConfig()
{
    IMS_TRACE_I("~CodecEvsConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecEvsConfig::Create(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("Create - EvsCodecConfig", 0, 0, 0);

    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piBuffer is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    ICarrierConfig* piCcBundle =
            piCc->GetBundle(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - EVS codec description is invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strPayloadTypeNumber;
    strPayloadTypeNumber.SetNumber(m_nPayloadType);
    ICarrierConfig* piCcSubBundle = piCcBundle->GetBundle(strPayloadTypeNumber.GetStr());

    if (piCcSubBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - EVS Codec SubBundle is invalid", 0, 0, 0);
        CreateDefaultEvsCodec();
    }
    else
    {
        SetChannel(piCcSubBundle->GetInt(
                CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT, DEFAULT_CHANNEL));
        m_bVisibleDtx = piCc->GetBoolean(
                CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_VISIBLE_DTX_BOOL, IMS_FALSE);
        SetDtx(piCcSubBundle->GetBoolean(CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL,
                CodecAudioConfig::DEFAULT_DTX));
        m_bDtxRecv = piCcSubBundle->GetBoolean(
                CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL, DEFAULT_DTX_RECV);
        if (!GetDtx())
        {
            m_bVisibleDtx = IMS_TRUE;
        }
        IMS_TRACE_D("Create - VisibleDtx[%d], Dtx[%d], DtxRecv[%d], ", m_bVisibleDtx, GetDtx(),
                m_bDtxRecv);

        m_nHfOnly = piCcSubBundle->GetInt(
                CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT, NOT_DEFINED);
        if (m_nHfOnly != NOT_DEFINED)
        {
            m_bVisibleHfOnly = IMS_TRUE;
        }
        else
        {
            m_nHfOnly = DEFAULT_HF_ONLY;
            m_bVisibleHfOnly = IMS_FALSE;
        }
        IMS_TRACE_D("Create - HfOnly[%d], VisibleHfOnly[%d]", m_nHfOnly, m_bVisibleHfOnly, 0);

        m_nEvsModeSwitch = piCcSubBundle->GetInt(
                CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT, NOT_DEFINED);
        if (m_nEvsModeSwitch != NOT_DEFINED)
        {
            m_bVisibleEvsModeSwitch = IMS_TRUE;
        }
        else
        {
            m_nEvsModeSwitch = DEFAULT_EVS_MODESWITCH;
            m_bVisibleEvsModeSwitch = IMS_FALSE;
        }
        IMS_TRACE_D("Create - EvsModeSwitch[%d], VisibleEvsModeSwitch[%d]", m_nEvsModeSwitch,
                m_bVisibleEvsModeSwitch, 0);

        ImsVector<IMS_SINT32> objBitrateList = piCcSubBundle->GetIntArray(
                CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY);

        IMS_SINT32 nBrStart = DEFAULT_BR;
        IMS_SINT32 nBrEnd = DEFAULT_BR;

        if (!objBitrateList.IsEmpty())
        {
            nBrStart = objBitrateList.GetAt(0);
            if (objBitrateList.GetSize() > 1)
            {
                nBrEnd = objBitrateList.GetAt(1);
            }
        }

        m_nBrList = ConvertEvsBitrateToList(nBrStart, nBrEnd);
        m_nBwList = CheckEvsBandwidthWithBitrate(
                piCcSubBundle->GetInt(
                        CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT,
                        DEFAULT_BW_LIST),
                m_nBrList);

        m_nCmr = piCcSubBundle->GetInt(
                CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CMR_INT, NOT_DEFINED);
        if (m_nCmr != NOT_DEFINED)
        {
            m_bVisibleCmr = IMS_TRUE;
        }
        else
        {
            m_nCmr = DEFAULT_CMR;
            m_bVisibleCmr = IMS_FALSE;
        }

        m_nChAwRecv = piCcSubBundle->GetInt(
                CarrierConfig::ImsVoice::KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT, NOT_DEFINED);
        if (m_nChAwRecv != NOT_DEFINED)
        {
            m_bVisibleChAwRecv = IMS_TRUE;
        }
        else
        {
            m_nChAwRecv = DEFAULT_CH_AW_RECV;
            m_bVisibleChAwRecv = IMS_FALSE;
        }

        IMS_SINT32 nModeChangeCapability = piCcSubBundle->GetInt(
                CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
                NOT_DEFINED);
        if (nModeChangeCapability != NOT_DEFINED)
        {
            SetModeChangeCapability(nModeChangeCapability);
            SetVisibleModeChangeCapability(IMS_TRUE);

            IMS_BOOL bVisibleFlag = piCc->GetBoolean(
                    CarrierConfig::ImsVoice::
                            KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_CAPABILITY_BOOL,
                    IMS_TRUE);

            if (!bVisibleFlag)
            {
                SetVisibleModeChangeCapability(IMS_FALSE);
            }
        }
        else
        {
            SetModeChangeCapability(CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);
            SetVisibleModeChangeCapability(IMS_FALSE);
        }

        IMS_SINT32 nModeChangePeriod = piCcSubBundle->GetInt(
                CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT, NOT_DEFINED);
        if (nModeChangePeriod != NOT_DEFINED)
        {
            SetModeChangePeriod(nModeChangePeriod);
            SetVisibleModeChangePeriod(IMS_TRUE);

            IMS_BOOL bVisibleFlag = piCc->GetBoolean(
                    CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_PERIOD_BOOL,
                    IMS_TRUE);

            if (!bVisibleFlag)
            {
                SetVisibleModeChangePeriod(IMS_FALSE);
            }
        }
        else
        {
            SetModeChangePeriod(CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);
            SetVisibleModeChangePeriod(IMS_FALSE);
        }

        IMS_SINT32 nModeChangeNeighbor = piCcSubBundle->GetInt(
                CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT, NOT_DEFINED);
        if (nModeChangeNeighbor != NOT_DEFINED)
        {
            SetModeChangeNeighbor(nModeChangeNeighbor);
            SetVisibleModeChangeNeighbor(IMS_TRUE);

            IMS_BOOL bVisibleFlag = piCc->GetBoolean(
                    CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_NEIGHBOR_BOOL,
                    IMS_TRUE);

            if (!bVisibleFlag)
            {
                SetVisibleModeChangeNeighbor(IMS_FALSE);
            }
        }
        else
        {
            SetModeChangeNeighbor(CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);
            SetVisibleModeChangeNeighbor(IMS_FALSE);
        }

        // From the internal Asset.
        m_bVisibleAmrIOModeSet = piCc->GetBoolean(
                CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_AMRWBIO_MODESET_BOOL,
                IMS_FALSE);
        SetVisibleModeSet(m_bVisibleAmrIOModeSet);

        m_nAmrIOModeSetList = static_cast<IMS_UINT32>(
                piCc->GetInt(CarrierConfig::ImsVoice::KEY_EVS_AMRWB_IO_MODE_SET_INT,
                        CodecAudioConfig::DEFAULT_MODESET_AMRWB));
        SetModeSetList(m_nAmrIOModeSetList);

        ImsVector<IMS_SINT32> objCodecDefaultModeset = piCc->GetIntArray(
                CarrierConfig::ImsVoice::KEY_AUDIO_AMRWB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY);
        IMS_SINT32 nDefaultModeSetList = ConvertModeSetList(objCodecDefaultModeset);
        SetDefaultModeSetList(nDefaultModeSetList);
        IMS_TRACE_D("Create - DefaultAmrWBModeSetList[%d]", nDefaultModeSetList, 0, 0);
        // TODO(b/414484057) : need to change the default value like the amrcodec and check the
        // display asset for the AMR-IO mode again
        piCcSubBundle->ReleaseBundle();
    }

    piCcBundle->ReleaseBundle();

    ToDebugString();
    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecEvsConfig::ToDebugString() const
{
    CodecAudioConfig::ToDebugString();

    IMS_TRACE_D("VisibleDtx[%d], Dtx[%d], DtxRecv[%d], ", m_bVisibleDtx, GetDtx(), m_bDtxRecv);
    IMS_TRACE_D("HfOnly[%d], EvsModeSwitch[%d], BitrateList[0x%04x]", m_nHfOnly, m_nEvsModeSwitch,
            m_nBrList);
    IMS_TRACE_D("BandwidthList[%d], Cmr[%d], ChAwRecv[%d]", m_nBwList, m_nCmr, m_nChAwRecv);
}

PRIVATE
IMS_SINT32 CodecEvsConfig::ConvertEvsBitrateToList(IN IMS_SINT32 nBrStart, IN IMS_SINT32 nBrEnd)
{
    IMS_SINT32 nBitrateSet = 0;

    IMS_TRACE_D("ConvertEvsBitrateToList - Bitrate range:[%d]~[%d]", nBrStart, nBrEnd, 0);

    if (nBrStart < EVS_PRIMARY_MODE_BITRATE_5_9_KBPS ||
            nBrEnd < EVS_PRIMARY_MODE_BITRATE_5_9_KBPS ||
            nBrStart > EVS_PRIMARY_MODE_BITRATE_128_0_KBPS ||
            nBrEnd > EVS_PRIMARY_MODE_BITRATE_128_0_KBPS)
    {
        nBitrateSet = DEFAULT_BR_LIST;
    }
    else
    {
        for (IMS_SINT32 nBitrate = nBrStart; nBitrate <= nBrEnd; nBitrate++)
        {
            IMS_TRACE_D("ConvertEvsBitrateToList - Bitrate[%d] ", nBitrate, 0, 0);
            nBitrateSet = (nBitrateSet | (1 << nBitrate));
        }
    }

    return nBitrateSet;
}

PRIVATE
IMS_SINT32 CodecEvsConfig::CheckEvsBandwidthWithBitrate(
        IN IMS_SINT32 nBwList, IN IMS_SINT32 nBrList)
{
    IMS_TRACE_D("CheckEvsBandwidthWithBitrate - BandwidthList[%d] BitrateList[%d]", nBwList,
            nBrList, 0);

    if ((nBrList & 0xFF8) == IMS_FALSE)
    {
        switch (nBwList)
        {
            case EVS_ENCODED_BW_TYPE_NB:  // FALL-THROUGH
            case EVS_ENCODED_BW_TYPE_WB:  // FALL-THROUGH
            case EVS_ENCODED_BW_TYPE_NB_WB:
                IMS_TRACE_D("CheckEvsBandwidthWithBitrate - no change", 0, 0, 0);
                break;
            case EVS_ENCODED_BW_TYPE_SWB:  // FALL-THROUGH
            case EVS_ENCODED_BW_TYPE_FB:
                IMS_TRACE_D("CheckEvsBandwidthWithBitrate - br and bw mismatched", 0, 0, 0);
                /** TODO Media - check to need to convert to WB or return error */
                break;
            case EVS_ENCODED_BW_TYPE_NB_WB_SWB:  // FALL-THROUGH
            case EVS_ENCODED_BW_TYPE_NB_WB_SWB_FB:
                nBwList = EVS_ENCODED_BW_TYPE_NB_WB;
                break;
        }
        IMS_TRACE_D("CheckEvsBandwidthWithBitrate - changed BandwidthList[%d]", nBwList, 0, 0);
    }
    else if ((nBrList & 0xFE0) == IMS_FALSE)
    {
        switch (nBwList)
        {
            case EVS_ENCODED_BW_TYPE_NB:         // FALL-THROUGH
            case EVS_ENCODED_BW_TYPE_WB:         // FALL-THROUGH
            case EVS_ENCODED_BW_TYPE_NB_WB:      // FALL-THROUGH
            case EVS_ENCODED_BW_TYPE_SWB:        // FALL-THROUGH
            case EVS_ENCODED_BW_TYPE_NB_WB_SWB:  // FALL-THROUGH
                IMS_TRACE_D("CheckEvsBandwidthWithBitrate - no change", 0, 0, 0);
                break;
            case EVS_ENCODED_BW_TYPE_FB:
                IMS_TRACE_D("CheckEvsBandwidthWithBitrate - br and bw mismatched", 0, 0, 0);
                /** TODO Media - check to need to convert to WB or return error */
                break;
            case EVS_ENCODED_BW_TYPE_NB_WB_SWB_FB:
                nBwList = EVS_ENCODED_BW_TYPE_NB_WB_SWB;
                break;
        }
        IMS_TRACE_D("CheckEvsBandwidthWithBitrate - changed BandwidthList[%d]", nBwList, 0, 0);
    }

    switch (nBwList)
    {
        case EVS_ENCODED_BW_TYPE_NB:
            nBwList = 1 << EVS_ENCODED_BW_TYPE_NB;
            break;
        case EVS_ENCODED_BW_TYPE_WB:
            nBwList = 1 << EVS_ENCODED_BW_TYPE_WB;
            break;
        case EVS_ENCODED_BW_TYPE_SWB:
            nBwList = 1 << EVS_ENCODED_BW_TYPE_SWB;
            break;
        case EVS_ENCODED_BW_TYPE_FB:
            nBwList = 1 << EVS_ENCODED_BW_TYPE_FB;
            break;
        case EVS_ENCODED_BW_TYPE_NB_WB:
            nBwList = (1 << EVS_ENCODED_BW_TYPE_NB) | (1 << EVS_ENCODED_BW_TYPE_WB);
            break;
        case EVS_ENCODED_BW_TYPE_NB_WB_SWB:
            nBwList = (1 << EVS_ENCODED_BW_TYPE_NB) | (1 << EVS_ENCODED_BW_TYPE_WB) |
                    (1 << EVS_ENCODED_BW_TYPE_SWB);
            break;
        case EVS_ENCODED_BW_TYPE_NB_WB_SWB_FB:
            nBwList = (1 << EVS_ENCODED_BW_TYPE_NB) | (1 << EVS_ENCODED_BW_TYPE_WB) |
                    (1 << EVS_ENCODED_BW_TYPE_SWB) | (1 << EVS_ENCODED_BW_TYPE_FB);
            break;
    }

    IMS_TRACE_D("CheckEvsBandwidthWithBitrate - new changed BandwidthList[0x%04x]", nBwList, 0, 0);

    return nBwList;
}

PRIVATE
IMS_SINT32 CodecEvsConfig::GetEvsBitrateFromList(IN IMS_SINT32 nBitrateList)
{
    if (nBitrateList == 0)
    {
        return EVS_PRIMARY_MODE_BITRATE_24_4_KBPS;
    }

    for (IMS_SINT32 nFindBitrate = EVS_PRIMARY_MODE_BITRATE_MAX; nFindBitrate >= 0; nFindBitrate--)
    {
        if (nBitrateList & (1 << nFindBitrate))
        {
            return nFindBitrate;
        }
    }

    return EVS_PRIMARY_MODE_BITRATE_24_4_KBPS;
}

PUBLIC
IMS_BOOL CodecEvsConfig::GetVisibleDtx() const
{
    return m_bVisibleDtx;
}

PUBLIC
IMS_BOOL CodecEvsConfig::GetDtxRecv() const
{
    return m_bDtxRecv;
}

PUBLIC
IMS_BOOL CodecEvsConfig::GetVisibleHfOnly() const
{
    return m_bVisibleHfOnly;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetHfOnly() const
{
    return m_nHfOnly;
}

PUBLIC
IMS_BOOL CodecEvsConfig::GetVisibleEvsModeSwitch() const
{
    return m_bVisibleEvsModeSwitch;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetEvsModeSwitch() const
{
    return m_nEvsModeSwitch;
}

PUBLIC
IMS_UINT32 CodecEvsConfig::GetBrList() const
{
    return static_cast<IMS_UINT32>(m_nBrList);
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetBr() const
{
    return GetEvsBitrateFromList(m_nBrList);
}

PUBLIC
IMS_UINT32 CodecEvsConfig::GetBwList() const
{
    return static_cast<IMS_UINT32>(m_nBwList);
}

PUBLIC
IMS_BOOL CodecEvsConfig::GetVisibleCmr() const
{
    return m_bVisibleCmr;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetCmr() const
{
    return m_nCmr;
}

PUBLIC
IMS_BOOL CodecEvsConfig::GetVisibleChAwareRecv() const
{
    return m_bVisibleChAwRecv;
}

PUBLIC
IMS_SINT32 CodecEvsConfig::GetChAwareRecv() const
{
    return m_nChAwRecv;
}

PUBLIC VIRTUAL void CodecEvsConfig::CreateDefaultEvsCodec()
{
    IMS_TRACE_D(
            "CreateDefaultEvsCodec: codec[%d], payloadTypeNumber[%d]", m_nCodec, m_nPayloadType, 0);
    SetChannel(DEFAULT_CHANNEL);
    m_bVisibleDtx = IMS_FALSE;
    SetDtx(CodecAudioConfig::DEFAULT_DTX);
    m_bDtxRecv = DEFAULT_DTX_RECV;

    m_nHfOnly = DEFAULT_HF_ONLY;
    m_bVisibleHfOnly = IMS_FALSE;

    m_nEvsModeSwitch = DEFAULT_EVS_MODESWITCH;
    m_bVisibleEvsModeSwitch = IMS_FALSE;

    m_nBrList = ConvertEvsBitrateToList(DEFAULT_BR, DEFAULT_BR);
    m_nBwList = CheckEvsBandwidthWithBitrate(DEFAULT_BW_LIST, m_nBrList);

    m_nCmr = DEFAULT_CMR;
    m_bVisibleChAwRecv = IMS_FALSE;

    m_nChAwRecv = DEFAULT_CH_AW_RECV;
    m_bVisibleChAwRecv = IMS_FALSE;

    SetModeSetList(CodecAudioConfig::FULL_MODESET_AMRWB);

    SetModeChangeCapability(CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);
    SetVisibleModeChangeCapability(IMS_FALSE);

    SetModeChangePeriod(CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);
    SetVisibleModeChangePeriod(IMS_FALSE);

    SetModeChangeNeighbor(CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);
    SetVisibleModeChangeNeighbor(IMS_FALSE);

    m_bVisibleAmrIOModeSet = IMS_FALSE;
    m_nAmrIOModeSetList = CodecAudioConfig::DEFAULT_MODESET_AMRWB;
    SetDefaultModeSetList(CodecAudioConfig::FULL_MODESET_AMRWB);
}
