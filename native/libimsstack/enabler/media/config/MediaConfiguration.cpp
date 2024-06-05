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
#include "config/ImsCodec.h"
#include "config/CodecConfig.h"
#include "config/CodecConfigFactory.h"
#include "config/MediaConfiguration.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
MediaConfiguration::MediaConfiguration(MEDIA_CONTENT_TYPE eSessionType) :
        m_eSessionType(eSessionType),
        m_nPortRtp(DEFAULT_RTP_PORT),
        m_nPortRtpEnd(DEFAULT_RTP_PORT_END),
        m_nPortRtcp(DEFAULT_RTCP_PORT),
        m_nRtcpLiveInterval(DEFAULT_RTCP_INVERVAL_LIVE),
        m_nRtcpInterval(DEFAULT_RTCP_INVERVAL),
        m_nAsBandwidthKbps(DEFAULT_AS),
        m_nRsBandwidthBps(DEFAULT_RS),
        m_nRrBandwidthBps(DEFAULT_RR),
        m_nRtpInactivityTimerMillis(DEFAULT_RTP_INACTIVITY),
        m_nRtcpInactivityTimerMillis(DEFAULT_RTCP_INACTIVITY)
{
    IMS_TRACE_I("+MediaConfiguration eSessionType[%d]", eSessionType, 0, 0);
}

PUBLIC VIRTUAL MediaConfiguration::~MediaConfiguration()
{
    IMS_TRACE_I("~MediaConfiguration", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL MediaConfiguration::Create(IN ICarrierConfig* piCc)
{
    (void)piCc;
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL MediaConfiguration::Update(IN ICarrierConfig* piCc)
{
    (void)piCc;
    return IMS_TRUE;
}

PUBLIC VIRTUAL CodecConfig* MediaConfiguration::GetCodecConfig(IN IMS_UINT32 nCodec) const
{
    for (IMS_UINT32 i = 0; i < m_objCodecConfigs.GetSize(); i++)
    {
        CodecConfig* pCodecConfig = m_objCodecConfigs.GetAt(i);

        if (pCodecConfig->GetCodec() == nCodec)
        {
            return pCodecConfig;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL const ImsList<CodecConfig*>& MediaConfiguration::GetCodecConfigs() const
{
    return m_objCodecConfigs;
}

PROTECTED VIRTUAL IMS_BOOL MediaConfiguration::CreateCodecConfigs(IN ICarrierConfig* piCc)
{
    (void)piCc;
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_UINT32 MediaConfiguration::MakeEachCodecs(IN ICarrierConfig* piCc,
        IN IMS_UINT32 nCodec, IN IMS_UINT32 nCodecIndex,
        IN ImsVector<IMS_SINT32> objPayloadTypeArray)
{
    if (!GetCodecType(nCodec))
    {
        IMS_TRACE_E(0, "MakeEachCodecs fail, No codec name", 0, 0, 0);
        return nCodecIndex;
    }

    if (piCc == IMS_NULL || objPayloadTypeArray.GetSize() <= 0)
    {
        return nCodecIndex;
    }

    for (IMS_SINT32 nIndex = 0; nIndex < objPayloadTypeArray.GetSize(); nIndex++)
    {
        nCodecIndex = MakeCodec(piCc, nCodec, nCodecIndex, objPayloadTypeArray.GetAt(nIndex));
    }

    return nCodecIndex;
}

PROTECTED VIRTUAL IMS_UINT32 MediaConfiguration::MakeCodec(IN ICarrierConfig* piCc,
        IN IMS_UINT32 nCodec, IN IMS_UINT32 nCodecIndex, IN IMS_SINT32 nPayloadTypeNum)
{
    IMS_UINT32 nCodecType = GetCodecType(nCodec);
    CodecConfig* pCodecConfig = IMS_NULL;

    if (nCodecType == ImsCodec::AUDIO_MAX)
    {
        pCodecConfig = CodecConfigFactory::CreateAudioPayloadConfig(piCc, nCodec, nPayloadTypeNum);
    }
    else if (nCodecType == ImsCodec::VIDEO_MAX)
    {
        pCodecConfig = CodecConfigFactory::CreateVideoPayloadConfig(piCc, nCodec, nPayloadTypeNum);
    }
    else if (nCodecType == ImsCodec::TEXT_MAX)
    {
        pCodecConfig = CodecConfigFactory::CreateTextPayloadConfig(piCc, nCodec, nPayloadTypeNum);
    }

    if (pCodecConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "MakeCodec fail, Create failure", 0, 0, 0);
        return nCodecIndex;
    }

    if (m_objCodecConfigs.InsertAt(pCodecConfig, nCodecIndex))
    {
        nCodecIndex++;
        IMS_TRACE_D("MakeCodec Added PayloadNum[%d], CodecIndex[%d], nPayloadTypeNum[%d]",
                pCodecConfig->GetPayloadType(), nCodecIndex, nPayloadTypeNum);
    }
    else
    {
        IMS_TRACE_E(0, "MakeCodec fail, InsertAt PayloadNum[%d] nPayloadTypeNum[%d]",
                pCodecConfig->GetPayloadType(), nCodecIndex, nPayloadTypeNum);
        return nCodecIndex;
    }
    return nCodecIndex;
}

PROTECTED VIRTUAL void MediaConfiguration::ToDebugString() const
{
    IMS_TRACE_D("session_type[%d], m_nRtcpLiveInterval[%d], m_nRtcpInterval[%d]",
            (IMS_SINT32)m_eSessionType, m_nRtcpLiveInterval, m_nRtcpInterval);
    IMS_TRACE_D("m_nPortRtp[%d], m_nPortRtpEnd[%d], m_nPortRtcp[%d]", m_nPortRtp, m_nPortRtpEnd,
            m_nPortRtcp);
    IMS_TRACE_D("m_nAsBandwidthKbps[%d], m_nRsBandwidthBps[%d], m_nRrBandwidthBps[%d]",
            m_nAsBandwidthKbps, m_nRsBandwidthBps, m_nRrBandwidthBps);
    IMS_TRACE_D("m_nRtpInactivityTimerMillis[%d], m_nRtcpInactivityTimerMillis[%d]",
            m_nRtpInactivityTimerMillis, m_nRtcpInactivityTimerMillis, 0);
}

PROTECTED VIRTUAL void MediaConfiguration::ToDebugStringCodecs(IN CodecConfig* pCodecConfig) const
{
    if (pCodecConfig == IMS_NULL)
    {
        return;
    }

    switch (pCodecConfig->GetCodec())
    {
        case ImsCodec::AUDIO_AMR:
        case ImsCodec::AUDIO_AMR_WB:
        {
            CodecAmrConfig* pAMRConfig = IMS_NULL;
            pAMRConfig = reinterpret_cast<CodecAmrConfig*>(pCodecConfig);

            if (pAMRConfig)
            {
                pAMRConfig->ToDebugString();
            }
        }
        break;
        case ImsCodec::AUDIO_PCMA:
        case ImsCodec::AUDIO_PCMU:
        {
            CodecPcmConfig* pPCMConfig = IMS_NULL;
            pPCMConfig = reinterpret_cast<CodecPcmConfig*>(pCodecConfig);

            if (pPCMConfig)
            {
                pPCMConfig->ToDebugString();
            }
        }
        break;
        case ImsCodec::AUDIO_TELEPHONE_EVENT:
        case ImsCodec::AUDIO_TELEPHONE_EVENT_WB:
        {
            CodecTelephoneEventConfig* pTelephoneEventConfig = IMS_NULL;
            pTelephoneEventConfig = reinterpret_cast<CodecTelephoneEventConfig*>(pCodecConfig);

            if (pTelephoneEventConfig)
            {
                pTelephoneEventConfig->ToDebugString();
            }
        }
        break;
        case ImsCodec::AUDIO_EVS:
        {
            CodecEvsConfig* pEVSConfig = IMS_NULL;
            pEVSConfig = reinterpret_cast<CodecEvsConfig*>(pCodecConfig);

            if (pEVSConfig)
            {
                pEVSConfig->ToDebugString();
            }
        }
        break;
        case ImsCodec::VIDEO_AVC:
        {
            CodecAvcConfig* pAvcConfig = IMS_NULL;
            pAvcConfig = reinterpret_cast<CodecAvcConfig*>(pCodecConfig);

            if (pAvcConfig)
            {
                pAvcConfig->ToDebugString();
            }
        }
        break;
        case ImsCodec::VIDEO_HEVC:
        {
            CodecHevcConfig* pHevcConfig = IMS_NULL;
            pHevcConfig = reinterpret_cast<CodecHevcConfig*>(pCodecConfig);

            if (pHevcConfig)
            {
                pHevcConfig->ToDebugString();
            }
        }
        break;
        case ImsCodec::TEXT_T140:
        {
            CodecT140Config* pT140Config = IMS_NULL;
            pT140Config = reinterpret_cast<CodecT140Config*>(pCodecConfig);

            if (pT140Config)
            {
                pT140Config->ToDebugString();
            }
        }
        break;
        default:
            break;
    }
}

PROTECTED VIRTUAL void MediaConfiguration::Clear()
{
    for (IMS_UINT32 i = 0; i < m_objCodecConfigs.GetSize(); ++i)
    {
        CodecConfig* pCodecConfig = m_objCodecConfigs.GetAt(i);

        if (pCodecConfig != IMS_NULL)
        {
            delete pCodecConfig;
        }
    }

    m_objCodecConfigs.Clear();
}

PROTECTED
IMS_UINT32 MediaConfiguration::GetCodecType(IN IMS_UINT32 nCodec) const
{
    switch (nCodec)
    {
        case ImsCodec::AUDIO_AMR:
        case ImsCodec::AUDIO_AMR_WB:
        case ImsCodec::AUDIO_PCMA:
        case ImsCodec::AUDIO_PCMU:
        case ImsCodec::AUDIO_TELEPHONE_EVENT:
        case ImsCodec::AUDIO_TELEPHONE_EVENT_WB:
        case ImsCodec::AUDIO_EVS:
            return ImsCodec::AUDIO_MAX;
        case ImsCodec::VIDEO_AVC:
        case ImsCodec::VIDEO_HEVC:
            return ImsCodec::VIDEO_MAX;
        case ImsCodec::TEXT_T140:
        case ImsCodec::TEXT_RED:
            return ImsCodec::TEXT_MAX;
        default:
            return 0;
    }
}

PROTECTED
void MediaConfiguration::SetPorts(IN ICarrierConfig* piCc, IN const IMS_CHAR* pszKey)
{
    ImsVector<IMS_SINT32> objPortRtp = piCc->GetIntArray(pszKey);
    if (!objPortRtp.IsEmpty())
    {
        m_nPortRtp = objPortRtp.GetAt(0);
        m_nPortRtcp = m_nPortRtp + 1;

        if (objPortRtp.GetSize() > 1)
        {
            m_nPortRtpEnd = objPortRtp.GetAt(1);
        }
    }
}

PROTECTED
void MediaConfiguration::SetRtcpIntervals(IN ICarrierConfig* piCc, IN const IMS_CHAR* pszKey)
{
    ImsVector<IMS_SINT32> objRtcpInterval = piCc->GetIntArray(pszKey);
    if (!objRtcpInterval.IsEmpty())
    {
        m_nRtcpLiveInterval = objRtcpInterval.GetAt(0);

        if (objRtcpInterval.GetSize() > 1)
        {
            m_nRtcpInterval = objRtcpInterval.GetAt(1);
        }
    }
}

PUBLIC
MEDIA_CONTENT_TYPE MediaConfiguration::GetSessionType() const
{
    return m_eSessionType;
}

PUBLIC
IMS_SINT32 MediaConfiguration::GetPortRtp() const
{
    return m_nPortRtp;
}

PUBLIC
IMS_SINT32 MediaConfiguration::GetPortRtpEnd() const
{
    return m_nPortRtpEnd;
}

PUBLIC
IMS_SINT32 MediaConfiguration::GetPortRtcp() const
{
    return m_nPortRtcp;
}

PUBLIC
IMS_SINT32 MediaConfiguration::GetRtcpLiveInterval() const
{
    return m_nRtcpLiveInterval;
}

PUBLIC
IMS_SINT32 MediaConfiguration::GetRtcpInterval() const
{
    return m_nRtcpInterval;
}

PUBLIC
IMS_SINT32 MediaConfiguration::GetAsBandwidthKbps() const
{
    return m_nAsBandwidthKbps;
}

PUBLIC
IMS_SINT32 MediaConfiguration::GetRsBandwidthBps() const
{
    return m_nRsBandwidthBps;
}

PUBLIC
IMS_SINT32 MediaConfiguration::GetRrBandwidthBps() const
{
    return m_nRrBandwidthBps;
}

PUBLIC
IMS_SINT32 MediaConfiguration::GetRtpInactivityTimerMillis() const
{
    return m_nRtpInactivityTimerMillis;
}

PUBLIC
IMS_SINT32 MediaConfiguration::GetRtcpInactivityTimerMillis() const
{
    return m_nRtcpInactivityTimerMillis;
}
