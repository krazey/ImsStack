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

#include "ISessionDescriptor.h"
#include "ServiceTimer.h"

#include "Configuration.h"
#include "ServicePhoneInfo.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceNetwork.h"
#include "ServiceEvent.h"
#include "ServiceSystemTime.h"
#include "ServiceUtil.h"
#include "IJniMedia.h"
#include "IMediaSessionListener.h"
#include "MediaManager.h"
#include "audio/AudioMediaSession.h"
#include "audio/AudioProfileUtil.h"

#include <AudioConfig.h>
using namespace android::telephony::imsmedia;

static const IMS_UINT32 IMS_MEDIA_TIMER_MARGIN = 1000;

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
AudioMediaSession::AudioMediaSession(IN IMS_SINT32 nSlotId) :
        BaseSession(nSlotId),
        m_pConfig(IMS_NULL),
        m_objMediaQualityThreshold(MediaQualityThreshold()),
        m_objLocalAddress(IpAddress::IPv6NONE),
        m_nLocalPort(0),
        m_nNetworkToneTimer(0),
        m_nRtpInactivityTimer(0),
        m_bAnbrEnabled(IMS_FALSE),
        m_piNetworkToneWaitTimer(IMS_NULL)
{
    IMS_TRACE_I("+AudioMediaSession() - state[%d]", m_nState, 0, 0);

    m_pRtpConfig = new AudioConfig();
}

PUBLIC
VIRTUAL AudioMediaSession::~AudioMediaSession()
{
    IMS_TRACE_I("~AudioMediaSession() - state[%d]", m_nState, 0, 0);

    if (m_pRtpConfig)
    {
        delete m_pRtpConfig;
    }
}

PUBLIC VIRTUAL void AudioMediaSession::Timer_TimerExpired(IN ITimer* piTimer)
{
    if ((m_piNetworkToneWaitTimer != IMS_NULL) && (m_piNetworkToneWaitTimer == piTimer))
    {
        IMS_TRACE_D("Timer_TimerExpired", 0, 0, 0);

        NetworkToneTimerExpired();
    }
}

PRIVATE void AudioMediaSession::NetworkToneTimerExpired()
{
    IMS_TRACE_D("NetworkToneTimerExpired : networktone time[%d]",
            GetInactivityTimer(NETWORK_TONE_INACTIVITY), 0, 0);

    if (GetInactivityTimer(NETWORK_TONE_INACTIVITY) > 0)
    {
        SetNetworkToneTimer(0);

        if (m_piMediaSessionListener != IMS_NULL)
        {
            m_piMediaSessionListener->MediaSession_NotifyToClient(
                    REPORT_DATA_RECEIVE_STARTED, MEDIA_TYPE_AUDIO, MEDIA_PROTOCOL_RTP);
            m_piMediaSessionListener->MediaSession_NotifyToClient(
                    REPORT_NW_TONE_RTP_RECEIVE_STARTED, MEDIA_TYPE_AUDIO, MEDIA_PROTOCOL_RTP);
        }
    }
}

PRIVATE
IMS_RESULT AudioMediaSession::StartTimer(IN IMS_SINT32 nDuration)
{
    IMS_TRACE_D("StartTimer : duration[%d]", nDuration, 0, 0);

    if (m_piNetworkToneWaitTimer == IMS_NULL)
    {
        m_piNetworkToneWaitTimer = TimerService::GetTimerService()->CreateTimer();

        if (m_piNetworkToneWaitTimer == IMS_NULL)
        {
            return IMS_FAILURE;
        }

        m_piNetworkToneWaitTimer->SetTimer(nDuration + IMS_MEDIA_TIMER_MARGIN, this);
    }

    return IMS_SUCCESS;
}

PRIVATE
void AudioMediaSession::StopTimer()
{
    if (m_piNetworkToneWaitTimer == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("StopTimer", 0, 0, 0);

    m_piNetworkToneWaitTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piNetworkToneWaitTimer);
    m_piNetworkToneWaitTimer = IMS_NULL;
}

PUBLIC
void AudioMediaSession::SetNegoId(IMS_UINTP nNegoId)
{
    m_listNegoId.Append(nNegoId);
}

PUBLIC
IMS_BOOL AudioMediaSession::IsSameNegoId(IMS_UINTP nNegoId)
{
    IMS_BOOL bRet = IMS_FALSE;

    // check nego id
    for (IMS_UINT32 i = 0; i < m_listNegoId.GetSize(); i++)
    {
        if (nNegoId == m_listNegoId.GetAt(i))
        {
            bRet = IMS_TRUE;
            break;
        }
    }
    return bRet;
}

PUBLIC
void AudioMediaSession::SetConfig(IN AudioConfiguration* pConfig)
{
    m_pConfig = pConfig;
}

PUBLIC
IMS_BOOL AudioMediaSession::UpdateRtpConfig(IN const IMS_UINT32 nAccessNetwork,
        IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
        IN AudioProfile* pNegoProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegoProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateRtpConfig() - invalid profile", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pNegoProfile->lstPayload.GetSize() == 0 || pPeerProfile->lstPayload.GetSize() == 0)
    {
        IMS_TRACE_E(0, "UpdateRtpConfig() - no payload to update", 0, 0, 0);
        return IMS_FALSE;
    }

    // Get Negotiated Payload from negotiated Payload index...
    AudioProfile::Payload* pPeerPayload;
    AudioProfile::Payload* pNegoPayload;

    IMS_TRACE_D("UpdateRtpConfig() - nNegotiated nDestPIndex[%d], nSrcIndex[%d]",
            pPeerProfile->nNegotiatedPayloadIndex, pLocalProfile->nNegotiatedPayloadIndex, 0);

    pNegoPayload = pNegoProfile->lstPayload.GetAt(0);

    if (pPeerProfile->nNegotiatedPayloadIndex < 0)
    {
        pPeerPayload = pPeerProfile->lstPayload.GetAt(0);
    }
    else
    {
        pPeerPayload = pPeerProfile->lstPayload.GetAt(pPeerProfile->nNegotiatedPayloadIndex);
    }

    if (pNegoPayload == IMS_NULL || pPeerPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AudioConfig objAudioConfig;

    // Setting the network properties
    SetLocalEndPoint(pNegoProfile->objIpAddress, pNegoProfile->nDataPort);
    objAudioConfig.setAccessNetwork(nAccessNetwork);
    objAudioConfig.setTxPayloadTypeNumber(pNegoPayload->objRtpMap.nPayloadNum);

    // remote network parameters
    objAudioConfig.setRemoteAddress(
            android::String8(pPeerProfile->objIpAddress.ToString().GetStr()));
    objAudioConfig.setRemotePort(pPeerProfile->nDataPort);
    objAudioConfig.setDscp(m_pConfig->GetRtpDscp());

    objAudioConfig.setRxPayloadTypeNumber(pPeerPayload->objRtpMap.nPayloadNum);

    IMS_SINT32 nAudioDirection;

    switch (pNegoProfile->eDirection)
    {
        case MEDIA_DIRECTION_RECEIVE:
            nAudioDirection = RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY;
            break;
        case MEDIA_DIRECTION_SEND_RECEIVE:
            nAudioDirection = RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE;
            break;
        case MEDIA_DIRECTION_SEND:
        case MEDIA_DIRECTION_INACTIVE:
            nAudioDirection = RtpConfig::MEDIA_DIRECTION_INACTIVE;
            break;
        default:
            nAudioDirection = RtpConfig::MEDIA_DIRECTION_NO_FLOW;
            break;
    }

    objAudioConfig.setMediaDirection((int32_t)nAudioDirection);

    IMS_TRACE_D("UpdateRtpConfig() - MediaDirection[%d], TxPayload[%d], RxPayload[%d]",
            objAudioConfig.getMediaDirection(), objAudioConfig.getTxPayloadTypeNumber(),
            objAudioConfig.getRxPayloadTypeNumber());
    IMS_TRACE_D("UpdateRtpConfig() - RemoteAddress[%s], RemotePort[%d]",
            objAudioConfig.getRemoteAddress().c_str(), objAudioConfig.getRemotePort(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - Dscp[%d], AccessNetwork[%d]", u_char(objAudioConfig.getDscp()),
            objAudioConfig.getAccessNetwork(), 0);

    // RTCP
    RtcpConfig objRtcpConfig;
    objRtcpConfig.setCanonicalName(android::String8("Canonical_Name"));  // TODO_MEDIA
    objRtcpConfig.setTransmitPort(pPeerProfile->nControlPort);

    if (pNegoProfile->nBandwidthRs == 0 && pNegoProfile->nBandwidthRr == 0)
    {
        objRtcpConfig.setIntervalSec(0);
    }
    else
    {
        objRtcpConfig.setIntervalSec(pNegoProfile->nRtcpInterval);
    }

    // RTCP-XR
    IMS_UINT32 nRtcpXrBlocks = 0;
    if (pNegoProfile->objRtcpXrAttr.bSupportVoipMetrics)
    {
        nRtcpXrBlocks += RtcpConfig::FLAG_RTCPXR_VOIP_METRICS_REPORT_BLOCK;
    }
    if (pNegoProfile->objRtcpXrAttr.bSupportStatisticMetrics)
    {
        nRtcpXrBlocks += RtcpConfig::FLAG_RTCPXR_STATISTICS_SUMMARY_REPORT_BLOCK;
    }
    if (pNegoProfile->objRtcpXrAttr.bSupportPacketLossRle)
    {
        nRtcpXrBlocks += RtcpConfig::FLAG_RTCPXR_LOSS_RLE_REPORT_BLOCK;
    }
    if (pNegoProfile->objRtcpXrAttr.bSupportPacketDuplicatedRle)
    {
        nRtcpXrBlocks += RtcpConfig::FLAG_RTCPXR_DUPLICATE_RLE_REPORT_BLOCK;
    }

    objRtcpConfig.setRtcpXrBlockTypes(nRtcpXrBlocks);
    objAudioConfig.setRtcpConfig(objRtcpConfig);

    IMS_TRACE_D("UpdateRtpConfig() - RTCP TransmitPort[%d], IntervalSec[%d], RtcpXrBlockTypes[%d]",
            objRtcpConfig.getTransmitPort(), objRtcpConfig.getIntervalSec(),
            objRtcpConfig.getRtcpXrBlockTypes());

    // Setting the codec properties
    if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
            pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
    {
        AudioProfile::AmrFmtp* pFmtp =
                REINTERPRET_CAST(AudioProfile::AmrFmtp*, pNegoPayload->pFmtp);
        if (pFmtp == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            objAudioConfig.setCodecType((int32_t)AudioConfig::CODEC_AMR_WB);
        }
        else
        {
            objAudioConfig.setCodecType((int32_t)AudioConfig::CODEC_AMR);
        }

        objAudioConfig.setSamplingRateKHz((int8_t)(pNegoPayload->objRtpMap.nSamplingRate / 1000));
        objAudioConfig.setPtimeMillis((int8_t)pNegoProfile->nPtime);
        objAudioConfig.setMaxPtimeMillis((int32_t)pNegoProfile->nMaxPtime);
        // AMR DTX on/off by source codec
        objAudioConfig.setDtxEnabled(IMS_FALSE);

        if (pLocalProfile->nNegotiatedPayloadIndex >= 0)
        {
            AudioProfile::Payload* pLocalPayload =
                    pLocalProfile->lstPayload.GetAt(pLocalProfile->nNegotiatedPayloadIndex);

            if (pLocalPayload != IMS_NULL && pLocalPayload->pFmtp != IMS_NULL)
            {
                objAudioConfig.setDtxEnabled(
                        (REINTERPRET_CAST(AudioProfile::AmrFmtp*, pLocalPayload->pFmtp)->bDtx));
            }
        }

        AmrParams* pAmrParams = new AmrParams();
        pAmrParams->setAmrMode((int32_t)AudioProfileUtil::GetModesetList(
                pNegoPayload->objRtpMap.strPayloadType, pNegoPayload));

        // AMR padding mode
        pAmrParams->setOctetAligned(pFmtp->nOctetAlign);
        pAmrParams->setMaxRedundancyMillis(0);  // TODO::MEDIA insert real value
        objAudioConfig.setAmrParams(*pAmrParams);
        delete pAmrParams;

        AmrParams objAmrParams = objAudioConfig.getAmrParams();
        IMS_TRACE_D("UpdateRtpConfig() - AmrMode[%d], OctetAligned[%d], MaxRedundancyMillis[%d]",
                objAmrParams.getAmrMode(), objAmrParams.getOctetAligned(),
                objAmrParams.getMaxRedundancyMillis());
    }
    else if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
    {
        AudioProfile::EvsFmtp* pFmtp =
                REINTERPRET_CAST(AudioProfile::EvsFmtp*, pNegoPayload->pFmtp);
        AudioProfile::EvsFmtp* pDestFmtp =
                REINTERPRET_CAST(AudioProfile::EvsFmtp*, pPeerPayload->pFmtp);
        if (pFmtp == IMS_NULL)
        {
            return IMS_FALSE;
        }
        objAudioConfig.setCodecType((int32_t)AudioConfig::CODEC_EVS);
        objAudioConfig.setSamplingRateKHz((int8_t)(pNegoPayload->objRtpMap.nSamplingRate / 1000));
        objAudioConfig.setPtimeMillis((int8_t)pNegoProfile->nPtime);
        objAudioConfig.setMaxPtimeMillis((int32_t)pNegoProfile->nMaxPtime);
        // AMR DTX on/off by source codec
        objAudioConfig.setDtxEnabled(pFmtp->bDtx);

        EvsParams* pEvsParams = new EvsParams();
        pEvsParams->setChannelAwareMode((int8_t)pFmtp->nChAwRecv);

        // TODO Media : use the Dest HFOnly
        pEvsParams->setUseHeaderFullOnly((IMS_BOOL)pDestFmtp->nHfOnly);

        IMS_SINT32 modeSet = AudioProfileUtil::GetModesetList("EVS", pNegoPayload);

        // evs primary mode conversion
        if (pFmtp->nEvsModeSwitch == 0)
        {  // evs primary mode
            modeSet = modeSet << 9;
        }
        pEvsParams->setEvsMode((int32_t)modeSet);

        // update bandwidth
        IMS_SINT32 nEvsBandwidth = pFmtp->nBwList;
        // exception : evs AMR-WB IO mode
        if (pFmtp->nEvsModeSwitch == 1)
        {
            nEvsBandwidth = EvsParams::EVS_WIDE_BAND;
        }
        pEvsParams->setEvsBandwidth(nEvsBandwidth);
        pEvsParams->setCodecModeRequest((int8_t)pFmtp->bSendCmr);

        objAudioConfig.setEvsParams(*pEvsParams);

        delete pEvsParams;

        EvsParams objEvsParams = objAudioConfig.getEvsParams();
        IMS_TRACE_D("UpdateRtpConfig() - EvsMode[%d], ChannelAwareMode[%d]",
                objEvsParams.getEvsMode(), objEvsParams.getChannelAwareMode(), 0);
        IMS_TRACE_D("UpdateRtpConfig() - UseHeaderFullOnly[%d]",
                objEvsParams.getUseHeaderFullOnly(), 0, 0);
        IMS_TRACE_D("UpdateRtpConfig() - EVS nBandwidth[0x%08x], CodecModeRequest[0x%08x]",
                objEvsParams.getEvsBandwidth(), objEvsParams.getCodecModeRequest(), 0);
    }
    else if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMU") ||
            pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMA"))
    {
        if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMU"))
        {
            objAudioConfig.setCodecType((int32_t)AudioConfig::CODEC_PCMA);
        }
        else
        {
            objAudioConfig.setCodecType((int32_t)AudioConfig::CODEC_PCMU);
        }
        objAudioConfig.setSamplingRateKHz((int8_t)(pNegoPayload->objRtpMap.nSamplingRate / 1000));
        objAudioConfig.setPtimeMillis((int8_t)pNegoProfile->nPtime);
        objAudioConfig.setMaxPtimeMillis((int32_t)pNegoProfile->nMaxPtime);
    }
    else
    {
        IMS_TRACE_E(0, "UpdateRtpConfig() - Invalid - state[%d]", m_nState, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("UpdateRtpConfig() - CodecType[%d], SamplingRate[%d]",
            objAudioConfig.getCodecType(), objAudioConfig.getSamplingRateKHz() * 1000, 0);
    IMS_TRACE_D("UpdateRtpConfig() - PtimeMillis[%d], MaxPtimeMillis[%d], DtxEnabled[%d]",
            objAudioConfig.getPtimeMillis(), objAudioConfig.getMaxPtimeMillis(),
            objAudioConfig.getDtxEnabled());

    // Setting the DTMF properties
    for (IMS_UINT32 i = 0; i < pNegoProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* pPayload = pNegoProfile->lstPayload.GetAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        if (pPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
        {
            objAudioConfig.setTxDtmfPayloadTypeNumber(pPayload->objRtpMap.nPayloadNum);
            objAudioConfig.setRxDtmfPayloadTypeNumber(pPayload->objRtpMap.nPayloadNum);
            objAudioConfig.setDtmfsamplingRateKHz(pPayload->objRtpMap.nSamplingRate / 1000);
            break;
        }
    }

    AnbrMode objAnbr;
    objAnbr.setDefaultAnbrMode();

    if (m_pRtpConfig->getAnbrMode().getAnbrUplinkCodecMode() != 0)
    {
        objAnbr.setAnbrUplinkCodecMode(m_pRtpConfig->getAnbrMode().getAnbrUplinkCodecMode());
    }
    if (m_pRtpConfig->getAnbrMode().getAnbrDownlinkCodecMode() != 0)
    {
        objAnbr.setAnbrDownlinkCodecMode(m_pRtpConfig->getAnbrMode().getAnbrDownlinkCodecMode());
    }
    objAudioConfig.setAnbrMode(objAnbr);

    IMS_TRACE_D("UpdateRtpConfig() - DtmfTxPayloadTypeNumber[%d],"
                "DtmfRxPayloadTypeNumber[%d],DtmfsamplingRateKHz[%d] ",
            objAudioConfig.getTxDtmfPayloadTypeNumber(),
            objAudioConfig.getRxDtmfPayloadTypeNumber(), objAudioConfig.getDtmfsamplingRateKHz());
    IMS_TRACE_D("UpdateRtpConfig() - Anbr UL CodecMode[%d], Anbr DL CodecMode[%d]",
            objAudioConfig.getAnbrMode().getAnbrUplinkCodecMode(),
            objAudioConfig.getAnbrMode().getAnbrDownlinkCodecMode(), 0);

    if (m_pRtpConfig != IMS_NULL)
    {
        AudioConfig* pPrevAudioConfig = REINTERPRET_CAST(AudioConfig*, m_pRtpConfig);

        if (objAudioConfig == *pPrevAudioConfig)
        {
            return IMS_FALSE;
        }

        delete m_pRtpConfig;
    }

    m_pRtpConfig = new AudioConfig(objAudioConfig);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioMediaSession::UpdateAccessNetwork(IMS_UINT32 nAccessNetwork)
{
    if (m_pRtpConfig->getAccessNetwork() != nAccessNetwork)
    {
        m_pRtpConfig->setAccessNetwork(nAccessNetwork);
        IMS_TRACE_D("UpdateAccessNetwork() - accessNetwork[%d]", m_pRtpConfig->getAccessNetwork(),
                0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL AudioMediaSession::UpdateMediaQualityThreshold(
        IN IMS_BOOL bActiveSession, IN IMS_BOOL bEnableRtcp)
{
    IMS_SINT32 nRtpInactivity = 0;

    if (GetInactivityTimer(NETWORK_TONE_INACTIVITY) > 0)
    {
        nRtpInactivity = GetInactivityTimer(NETWORK_TONE_INACTIVITY);
    }
    else
    {
        nRtpInactivity = (bActiveSession) ? m_pConfig->GetRtpInactivityTimerMillis() : 0;
        m_nRtpInactivityTimer = nRtpInactivity;
    }

    m_objMediaQualityThreshold.setRtpInactivityTimerMillis(std::vector<int32_t>{nRtpInactivity});
    m_objMediaQualityThreshold.setRtcpInactivityTimerMillis(
            (bEnableRtcp) ? m_pConfig->GetRtcpInactivityTimerMillis() : 0);

    IMS_TRACE_D("UpdateMediaQualityThreshold() - bActiveSession[%d], RtpInactivity[%d], "
                "RtcpInactivity[%d]",
            bActiveSession,
            (m_objMediaQualityThreshold.getRtpInactivityTimerMillis().empty())
                    ? -1
                    : m_objMediaQualityThreshold.getRtpInactivityTimerMillis().front(),
            m_objMediaQualityThreshold.getRtcpInactivityTimerMillis());

    return IMS_TRUE;
}

PUBLIC
void AudioMediaSession::SetLocalEndPoint(IN const IpAddress& objLocalAddr, IN IMS_UINT32 nPort)
{
    m_objLocalAddress = objLocalAddr;
    m_nLocalPort = nPort;

    IMS_TRACE_D("UpdateLocalEndPoint() - LocalIP[%s], LocalPort[%d]",
            m_objLocalAddress.ToString().GetStr(), m_nLocalPort, 0);
}

PUBLIC
IMS_BOOL AudioMediaSession::GetEnabledRtcp()
{
    return (m_pRtpConfig->getRtcpConfig().getIntervalSec() > 0);
}

PUBLIC
IMS_BOOL AudioMediaSession::Open()
{
    IMS_TRACE_I("Open() - state[%d]", m_nState, 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgOpenConfigParam* pParam = new ImsMediaMsgOpenConfigParam(MEDIA_TYPE_AUDIO);
        pParam->m_objLocalAddress = m_objLocalAddress;
        pParam->m_nLocalPort = m_nLocalPort;
        pParam->m_pConfig = new AudioConfig(REINTERPRET_CAST(AudioConfig*, m_pRtpConfig));
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_OPEN_SESSION, pParam);

        if (bResult == IMS_TRUE)
        {
            m_nState = STATE_IDLE;
        }
    }

    return bResult;
}

PUBLIC
IMS_BOOL AudioMediaSession::Modify()
{
    IMS_TRACE_I("Modify() - state[%d]", m_nState, 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_AUDIO);
        pParam->m_pConfig = new AudioConfig(REINTERPRET_CAST(AudioConfig*, m_pRtpConfig));

        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_MODIFY_SESSION, pParam);

        if (bResult == IMS_TRUE)
        {
            if (MEDIA_DIRECTION_IS_AUDIO_HOLD(GetDirection()))
            {
                m_nState = STATE_PAUSED;
            }
            else
            {
                m_nState = STATE_LIVE;
            }
        }
    }

    return bResult;
}

PUBLIC
IMS_BOOL AudioMediaSession::Add()
{
    IMS_TRACE_I("Add() - state[%d]", m_nState, 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_AUDIO);
        pParam->m_pConfig = new AudioConfig(REINTERPRET_CAST(AudioConfig*, m_pRtpConfig));
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_ADD_CONFIG, pParam);

        if (bResult == IMS_TRUE)
        {
            m_nState = STATE_LIVE;
        }
    }

    return bResult;
}

PUBLIC
IMS_BOOL AudioMediaSession::Delete()
{
    IMS_TRACE_I("Delete() - state[%d]", m_nState, 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_AUDIO);
        pParam->m_pConfig = new AudioConfig(REINTERPRET_CAST(AudioConfig*, m_pRtpConfig));
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_DELETE_CONFIG, pParam);

        if (bResult == IMS_TRUE)
        {
            m_nState = STATE_NONE;
        }
    }

    return bResult;
}

PUBLIC
IMS_BOOL AudioMediaSession::Confirm()
{
    IMS_TRACE_I("Confirm() - state[%d]", m_nState, 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_AUDIO);
        pParam->m_pConfig = new AudioConfig(REINTERPRET_CAST(AudioConfig*, m_pRtpConfig));
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_CONFIRM_CONFIG, pParam);

        if (bResult == IMS_TRUE)
        {
            m_nState = STATE_LIVE;
        }
    }

    return bResult;
}

PUBLIC
IMS_BOOL AudioMediaSession::Close()
{
    IMS_TRACE_I("Close() - state[%d]", m_nState, 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgParamBase* pParam = new ImsMediaMsgParamBase(MEDIA_TYPE_AUDIO);
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_CLOSE_SESSION, pParam);

        if (bResult == IMS_TRUE)
        {
            m_nState = STATE_NONE;
        }
    }

    return bResult;
}

PUBLIC
IMS_BOOL AudioMediaSession::SendDtmf(IN IMS_CHAR cDtmfCode)
{
    if (m_pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("SendDtmf() - state[%d], cDtmfCode[%d], nDuration[%d]", m_nState, cDtmfCode,
            m_pConfig->GetDTMFDuration());
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgDtmfParam* pParam = new ImsMediaMsgDtmfParam();
        pParam->m_dtmfCode = cDtmfCode;
        pParam->m_nDuration = m_pConfig->GetDTMFDuration();
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_SEND_DTMF, pParam);
    }

    return bResult;
}

PUBLIC
IMS_BOOL AudioMediaSession::UpdateAnbrEnabledConfig(IN IMS_BOOL anbrEnabled)
{
    IMS_TRACE_I(
            "UpdateAnbrEnabledConfig() - state[%d], anbr enabled[%d]", m_nState, anbrEnabled, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        m_bAnbrEnabled = anbrEnabled;

        ImsMediaMsgAnbrNegotiationParam* pParam = new ImsMediaMsgAnbrNegotiationParam();
        pParam->m_bAnbrNegotiationType = anbrEnabled;

        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_UPDATE_ANBR_ENABLED_CONFIG, pParam);
    }

    return bResult;
}

PUBLIC
IMS_BOOL AudioMediaSession::NotifyAnbrReceived(
        IN IMS_UINT32 anbrMediaType, IN IMS_UINT32 anbrDirection, IN IMS_UINT32 anbrBitrate)
{
    IMS_TRACE_I("NotifyAnbrReceived() - state[%d], anbr media type[%d], direction[%d]", m_nState,
            anbrMediaType, anbrDirection);
    IMS_BOOL bResult = IMS_FALSE;

    if (!m_bAnbrEnabled)
    {
        IMS_TRACE_D("NotifyAnbrReceived() - Anbr is disabled", 0, 0, 0);
        return bResult;
    }

    if (m_piMediaSessionListener != IMS_NULL && m_pRtpConfig != IMS_NULL)
    {
        AudioConfig* pAudioConfig = REINTERPRET_CAST(AudioConfig*, m_pRtpConfig);
        IMS_SINT32 codecMode =
                ConvertBitrateToCodecMode(anbrBitrate, (IMS_UINT32)pAudioConfig->getCodecType());
        AnbrMode anbrMode;

        if (anbrDirection == MEDIA_DIRECTION_ANBR::DIRECTION_UPLINK)
        {
            anbrMode.setAnbrUplinkCodecMode(1 << codecMode);

            if (m_pRtpConfig->getAnbrMode().getAnbrDownlinkCodecMode() != 0)
            {
                anbrMode.setAnbrDownlinkCodecMode(
                        m_pRtpConfig->getAnbrMode().getAnbrDownlinkCodecMode());
            }
        }
        else if (anbrDirection == MEDIA_DIRECTION_ANBR::DIRECTION_DOWNLINK)
        {
            if (m_pRtpConfig->getAnbrMode().getAnbrUplinkCodecMode() != 0)
            {
                anbrMode.setAnbrUplinkCodecMode(
                        m_pRtpConfig->getAnbrMode().getAnbrUplinkCodecMode());
            }

            anbrMode.setAnbrDownlinkCodecMode(1 << codecMode);
        }
        else
        {
            IMS_TRACE_D("NotifyAnbrReceived() - Invalid direction[%d]", anbrDirection, 0, 0);
            return bResult;
        }

        SetAnbrMode(anbrMode);

        bResult = Modify();
    }

    return bResult;
}

PRIVATE
IMS_SINT32 AudioMediaSession::ConvertBitrateToCodecMode(IMS_UINT32 bitrate, IMS_UINT32 codecType)
{
    IMS_SINT32 convertedCodecMode = -1;

    switch (codecType)
    {
        case CodecType_ANBR::CODEC_AMR:
        case CodecType_ANBR::CODEC_AMR_WB:
            if (bitrate >= 0 && bitrate <= 12)
            {
                convertedCodecMode = 10;
            }
            else if (bitrate <= 16)
            {
                convertedCodecMode = 12;
            }
            else if (bitrate <= 24)
            {
                convertedCodecMode = 13;
            }
            // TODO: need to add for EVS 24.4kbps
            break;
        case CodecType_ANBR::CODEC_EVS:
            if (bitrate >= 0 && bitrate <= 10)
            {
                convertedCodecMode = 9;  // 5.9kbps
            }
            else if (bitrate <= 12)
            {
                convertedCodecMode = 10;  // 7.2kbps
            }
            else if (bitrate <= 16)
            {
                convertedCodecMode = 12;  // 9.6kbps
            }
            else if (bitrate <= 20)
            {
                convertedCodecMode = 13;  // 13.2kbps
            }
            else if (bitrate <= 24)
            {
                convertedCodecMode = 14;  // 16.4kbps
            }
            else if (bitrate <= 28)
            {
                convertedCodecMode = 15;  // 24.4kbps
            }
            else
            {
                convertedCodecMode = 13;  // 13.2kbps
            }
            // TODO: need to add for EVS 24.4kbps
            break;
        default:
            IMS_TRACE_D("Enter default - Error", 0, 0, 0);
            break;
    }
    IMS_TRACE_D(
            "ConvertBitrateToCodecMode() - converted codec mode [%d]", convertedCodecMode, 0, 0);
    return convertedCodecMode;
}

PUBLIC
IMS_BOOL AudioMediaSession::SetMediaQuality()
{
    IMS_TRACE_I("SetMediaQuality() - state[%d]", m_nState, 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgSetMediaQualityParam* pParam =
                new ImsMediaMsgSetMediaQualityParam(MEDIA_TYPE_AUDIO);
        pParam->m_objMediaQualityThreshold = m_objMediaQualityThreshold;
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IJniMedia::REQUEST_SET_MEDIA_QUALITY, pParam);
    }
    return bResult;
}

PUBLIC
void AudioMediaSession::SetNetworkToneTimer(IN IMS_UINT32 nTimer)
{
    m_nNetworkToneTimer = nTimer;

    if (nTimer > 0)
    {
        StartTimer(nTimer);
    }
    else
    {
        StopTimer();
    }
}

PUBLIC
IMS_SINT32 AudioMediaSession::GetInactivityTimer(IN InactivitytimerType eType)
{
    switch (eType)
    {
        case RTP_INACTIVITY:
            return m_nRtpInactivityTimer;
        case RTCP_INACTIVITY:
            return m_objMediaQualityThreshold.getRtcpInactivityTimerMillis();
        case NETWORK_TONE_INACTIVITY:
            return m_nNetworkToneTimer;
        default:
            return -1;
    }
}
