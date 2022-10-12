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

#include <AudioConfig.h>

#include "ISessionDescriptor.h"
#include "Configuration.h"
#include "ServicePhoneInfo.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceNetwork.h"
#include "ServiceEvent.h"
#include "ServiceSystemTime.h"
#include "ServiceUtil.h"

#include "IMMedia.h"
#include "audio/AudioMediaSession.h"
#include "MediaManager.h"

using namespace android::telephony::imsmedia;

__IMS_TRACE_TAG_USER_DECL__("MED.AS");

PUBLIC
AudioMediaSession::AudioMediaSession(IN IMS_SINT32 nSlotId) :
        BaseSession(nSlotId),
        m_pConfig(IMS_NULL),
        m_objAudioConfig(AudioConfig()),
        m_objMediaQualityThreshold(MediaQualityThreshold()),
        m_objLocalAddress(IPAddress::IPv6NONE),
        m_nLocalPort(0)
{
    IMS_TRACE_I("+AudioMediaSession() - state[%d]", GetState(), 0, 0);
}

PUBLIC
VIRTUAL AudioMediaSession::~AudioMediaSession()
{
    IMS_TRACE_I("~AudioMediaSession() - state[%d]", GetState(), 0, 0);
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
IMS_BOOL AudioMediaSession::UpdateRtpConfig(IN AudioProfile* pLocalProfile,
        IN AudioProfile* pPeerProfile, IN AudioProfile* pNegoProfile)
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
    AudioProfile::Payload* pDestPayload;
    AudioProfile::Payload* pNegoPayload;

    IMS_TRACE_D("UpdateRtpConfig() - nNegotiated nDestPIndex[%d], nSrcIndex[%d]",
            pPeerProfile->nNegotiatedPayloadIndex, pLocalProfile->nNegotiatedPayloadIndex, 0);

    if (pNegoProfile->nNegotiatedPayloadIndex < 0)
    {
        pNegoPayload = pNegoProfile->lstPayload.GetAt(0);
    }
    else
    {
        pNegoPayload = pNegoProfile->lstPayload.GetAt(pNegoProfile->nNegotiatedPayloadIndex);
    }

    if (pPeerProfile->nNegotiatedPayloadIndex < 0)
    {
        pDestPayload = pPeerProfile->lstPayload.GetAt(0);
    }
    else
    {
        pDestPayload = pPeerProfile->lstPayload.GetAt(pPeerProfile->nNegotiatedPayloadIndex);
    }

    if (pNegoPayload == IMS_NULL || pDestPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Setting the network properties
    SetLocalEndPoint(pNegoProfile->objIpAddr, pNegoProfile->nDataPort);

    if (pLocalProfile->nNegotiatedPayloadIndex < 0)
    {
        m_objAudioConfig.setTxPayloadTypeNumber((int32_t)pNegoPayload->objRtpMap.nPayloadNum);
    }
    else
    {
        AudioProfile::Payload* pSrcPayload =
                pLocalProfile->lstPayload.GetAt(pLocalProfile->nNegotiatedPayloadIndex);
        if (pSrcPayload == IMS_NULL)
        {
            return IMS_FALSE;
        }

        m_objAudioConfig.setTxPayloadTypeNumber((int32_t)pSrcPayload->objRtpMap.nPayloadNum);
    }

    // remote network parameters
    m_objAudioConfig.setRemoteAddress(
            android::String8(pPeerProfile->objIpAddr.ToString().GetStr()));
    m_objAudioConfig.setRemotePort(pPeerProfile->nDataPort);
    m_objAudioConfig.setDscp(m_pConfig->GetRtpDscp());

    m_objAudioConfig.setRxPayloadTypeNumber(pDestPayload->objRtpMap.nPayloadNum);

    switch (pNegoProfile->eDirection)
    {
        default:
        case MEDIA_DIRECTION_INACTIVE:
        case MEDIA_DIRECTION_RECEIVE:
        case MEDIA_DIRECTION_SEND:
            m_objAudioConfig.setMediaDirection((int32_t)RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY);
            break;
        case MEDIA_DIRECTION_SEND_RECEIVE:
            m_objAudioConfig.setMediaDirection((int32_t)RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE);
            break;
    }

    IMS_TRACE_D("UpdateRtpConfig() - TxPayloadTypeNumber[%d], RxPayloadTypeNumber[%d]",
            m_objAudioConfig.getTxPayloadTypeNumber(), m_objAudioConfig.getRxPayloadTypeNumber(),
            0);
    IMS_TRACE_D("UpdateRtpConfig() - RemoteAddress[%s], RemotePort[%d]",
            m_objAudioConfig.getRemoteAddress().c_str(), m_objAudioConfig.getRemotePort(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - Dscp[%d], , MediaDirection[%d] , AccessNetwork[%d]",
            m_objAudioConfig.getDscp(), m_objAudioConfig.getMediaDirection(),
            m_objAudioConfig.getAccessNetwork());

    // RTCP
    RtcpConfig* pRtcpConfig = new RtcpConfig();
    pRtcpConfig->setCanonicalName(android::String8("Canonical_Name"));  // TODO_MEDIA
    pRtcpConfig->setTransmitPort(pNegoProfile->nControlPort);
    if (pNegoProfile->nBandwidthRs == 0 && pNegoProfile->nBandwidthRr == 0)
    {
        pRtcpConfig->setIntervalSec(0);
    }
    else
    {
        pRtcpConfig->setIntervalSec(pNegoProfile->nRtcpInterval);
    }

    // RTCP-XR
    IMS_UINT32 nRtcpXrBlocks = 0;
    if (pNegoProfile->objRtcpXrAttr.bSupportVoipMatircs)
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
    pRtcpConfig->setRtcpXrBlockTypes(nRtcpXrBlocks);
    m_objAudioConfig.setRtcpConfig(*pRtcpConfig);
    delete pRtcpConfig;

    RtcpConfig objRtcpConfig = m_objAudioConfig.getRtcpConfig();
    IMS_TRACE_D("UpdateRtpConfig() - RTCP CanonicalName[%s], RtcpXrBlockTypes[%d]",
            objRtcpConfig.getCanonicalName().c_str(), objRtcpConfig.getRtcpXrBlockTypes(), 0);
    IMS_TRACE_D("UpdateRtpConfig() - RTCP TransmitPort[%d], IntervalSec[%d]",
            objRtcpConfig.getTransmitPort(), objRtcpConfig.getIntervalSec(), 0);

    // Setting the codec properties
    if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") ||
            pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
    {
        AudioProfile::AmrFmtp* pFmtp =
                reinterpret_cast<AudioProfile::AmrFmtp*>(pNegoPayload->pFmtp);
        if (pFmtp == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            m_objAudioConfig.setCodecType((int32_t)AudioConfig::CODEC_AMR_WB);
        }
        else
        {
            m_objAudioConfig.setCodecType((int32_t)AudioConfig::CODEC_AMR);
        }
        m_objAudioConfig.setSamplingRateKHz((int8_t)(pNegoPayload->objRtpMap.nSamplingRate / 1000));
        m_objAudioConfig.setPtimeMillis((int8_t)pNegoProfile->nPtime);
        m_objAudioConfig.setMaxPtimeMillis((int32_t)pNegoProfile->nMaxPtime);
        // AMR DTX on/off by source codec
        m_objAudioConfig.setDtxEnabled(IMS_TRUE);

        if (pLocalProfile->nNegotiatedPayloadIndex >= 0)
        {
            AudioProfile::Payload* pSrcPayload =
                    pLocalProfile->lstPayload.GetAt(pLocalProfile->nNegotiatedPayloadIndex);

            if (pSrcPayload != IMS_NULL && pSrcPayload->pFmtp != IMS_NULL)
            {
                m_objAudioConfig.setDtxEnabled(
                        ((AudioProfile::AmrFmtp*)pSrcPayload->pFmtp)->bSCREnable);
            }
        }

        AmrParams* pAmrParams = new AmrParams();
        pAmrParams->setAmrMode((int32_t)AudioProfileUtil::GetModesetList(
                pNegoPayload->objRtpMap.strPayloadType, pNegoPayload));

        // AMR padding mode
        pAmrParams->setOctetAligned(pFmtp->nOctetAlign);
        pAmrParams->setMaxRedundancyMillis(0);  // TODO::MEDIA insert real value
        m_objAudioConfig.setAmrParams(*pAmrParams);
        delete pAmrParams;

        AmrParams objAmrParams = m_objAudioConfig.getAmrParams();
        IMS_TRACE_D("UpdateRtpConfig() - AmrMode[%d], OctetAligned[%d], MaxRedundancyMillis[%d]",
                objAmrParams.getAmrMode(), objAmrParams.getOctetAligned(),
                objAmrParams.getMaxRedundancyMillis());
    }
    else if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
    {
        AudioProfile::EvsFmtp* pFmtp =
                reinterpret_cast<AudioProfile::EvsFmtp*>(pNegoPayload->pFmtp);
        AudioProfile::EvsFmtp* pDestFmtp =
                reinterpret_cast<AudioProfile::EvsFmtp*>(pDestPayload->pFmtp);
        if (pFmtp == IMS_NULL)
        {
            return IMS_FALSE;
        }
        m_objAudioConfig.setCodecType((int32_t)AudioConfig::CODEC_EVS);
        m_objAudioConfig.setSamplingRateKHz((int8_t)(pNegoPayload->objRtpMap.nSamplingRate / 1000));
        m_objAudioConfig.setPtimeMillis((int8_t)pNegoProfile->nPtime);
        m_objAudioConfig.setMaxPtimeMillis((int32_t)pNegoProfile->nMaxPtime);
        // AMR DTX on/off by source codec
        m_objAudioConfig.setDtxEnabled((IMS_BOOL)pFmtp->nDtx);

        // TODO_MEDIA need to add DtxRecv
        // m_objAudioConfig.setDtxRecvEnabled((IMS_BOOL)pFmtp->nDtxRecv);

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

        m_objAudioConfig.setEvsParams(*pEvsParams);

        delete pEvsParams;

        EvsParams objEvsParams = m_objAudioConfig.getEvsParams();
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
            m_objAudioConfig.setCodecType((int32_t)AudioConfig::CODEC_PCMA);
        }
        else
        {
            m_objAudioConfig.setCodecType((int32_t)AudioConfig::CODEC_PCMU);
        }
        m_objAudioConfig.setSamplingRateKHz((int8_t)(pNegoPayload->objRtpMap.nSamplingRate / 1000));
        m_objAudioConfig.setPtimeMillis((int8_t)pNegoProfile->nPtime);
        m_objAudioConfig.setMaxPtimeMillis((int32_t)pNegoProfile->nMaxPtime);
    }
    else
    {
        IMS_TRACE_E(0, "UpdateRtpConfig() - Invalid - state[%d]", GetState(), 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("UpdateRtpConfig() - CodecType[%d], SamplingRate[%d]",
            m_objAudioConfig.getCodecType(), m_objAudioConfig.getSamplingRateKHz() * 1000, 0);
    IMS_TRACE_D("UpdateRtpConfig() - PtimeMillis[%d], MaxPtimeMillis[%d], DtxEnabled[%d]",
            m_objAudioConfig.getPtimeMillis(), m_objAudioConfig.getMaxPtimeMillis(),
            m_objAudioConfig.getDtxEnabled());

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
            m_objAudioConfig.setTxDtmfPayloadTypeNumber(pPayload->objRtpMap.nPayloadNum);
            m_objAudioConfig.setRxDtmfPayloadTypeNumber(pPayload->objRtpMap.nPayloadNum);
            m_objAudioConfig.setDtmfsamplingRateKHz(pPayload->objRtpMap.nSamplingRate / 1000);
            break;
        }
    }

    IMS_TRACE_D("UpdateRtpConfig() - DtmfTxPayloadTypeNumber[%d],"
                "DtmfRxPayloadTypeNumber[%d],DtmfsamplingRateKHz[%d] ",
            m_objAudioConfig.getTxDtmfPayloadTypeNumber(),
            m_objAudioConfig.getRxDtmfPayloadTypeNumber(),
            m_objAudioConfig.getDtmfsamplingRateKHz());

    return IMS_TRUE;
}

PUBLIC
MEDIA_DIRECTION AudioMediaSession::GetMediaDirection()
{
    IMS_UINT32 nDirection = m_objAudioConfig.getMediaDirection();

    switch (nDirection)
    {
        case RtpConfig::MEDIA_DIRECTION_NO_FLOW:
            return MEDIA_DIRECTION_INVALID;
        case RtpConfig::MEDIA_DIRECTION_SEND_ONLY:
            return MEDIA_DIRECTION_SEND;
        case RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY:
            return MEDIA_DIRECTION_RECEIVE;
        case RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE:
            return MEDIA_DIRECTION_SEND_RECEIVE;
        case RtpConfig::MEDIA_DIRECTION_INACTIVE:
            return MEDIA_DIRECTION_INACTIVE;
    }

    return MEDIA_DIRECTION_INVALID;
}

PUBLIC
void AudioMediaSession::SetMediaDirection(IN MEDIA_DIRECTION eDirection)
{
    IMS_TRACE_D("SetMediaDirection() - eDirection[%d]", eDirection, 0, 0);
    m_nPrevMediaDirection = GetMediaDirection();

    switch (eDirection)
    {
        case MEDIA_DIRECTION_INVALID:
            m_objAudioConfig.setMediaDirection(RtpConfig::MEDIA_DIRECTION_NO_FLOW);
            break;
        case MEDIA_DIRECTION_SEND:
            m_objAudioConfig.setMediaDirection(RtpConfig::MEDIA_DIRECTION_SEND_ONLY);
            break;
        case MEDIA_DIRECTION_RECEIVE:
            m_objAudioConfig.setMediaDirection(RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY);
            break;
        case MEDIA_DIRECTION_SEND_RECEIVE:
            m_objAudioConfig.setMediaDirection(RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE);
            break;
        case MEDIA_DIRECTION_INACTIVE:
            m_objAudioConfig.setMediaDirection(RtpConfig::MEDIA_DIRECTION_INACTIVE);
            break;
    }
}

PUBLIC
MEDIA_DIRECTION AudioMediaSession::GetPrevMediaDirection()
{
    return m_nPrevMediaDirection;
}

PUBLIC
void AudioMediaSession::UpdateAccessNetwork(IMS_UINT32 nAccessNetwork)
{
    m_objAudioConfig.setAccessNetwork(nAccessNetwork);
    IMS_TRACE_D(
            "UpdateAccessNetwork() - accessNetwork[%d]", m_objAudioConfig.getAccessNetwork(), 0, 0);
}

PUBLIC
IMS_BOOL AudioMediaSession::UpdateMediaQualityThreshold(
        IN IMS_BOOL bIsHold, IN IMS_BOOL bEnableRtcp)
{
    // TODO_MEDIA need to get real value when it's ready.
    if (bIsHold)
    {
        m_objMediaQualityThreshold.setRtpInactivityTimerMillis(0);
        m_objMediaQualityThreshold.setRtcpInactivityTimerMillis(
                m_pConfig->GetRtcpInactivityTimerMillis());
        m_objMediaQualityThreshold.setRtpPacketLossDurationMillis(0);
        m_objMediaQualityThreshold.setRtpPacketLossRate(0);
        m_objMediaQualityThreshold.setJitterDurationMillis(0);
        m_objMediaQualityThreshold.setRtpJitterMillis(0);
    }
    else
    {
        m_objMediaQualityThreshold.setRtpInactivityTimerMillis(
                m_pConfig->GetRtpInactivityTimerMillis());

        if (bEnableRtcp == IMS_FALSE)
        {
            m_objMediaQualityThreshold.setRtcpInactivityTimerMillis(0);
        }
        else
        {
            m_objMediaQualityThreshold.setRtcpInactivityTimerMillis(
                    m_pConfig->GetRtcpInactivityTimerMillis());
        }

        m_objMediaQualityThreshold.setRtpPacketLossDurationMillis(15000);
        m_objMediaQualityThreshold.setRtpPacketLossRate(30);
        m_objMediaQualityThreshold.setJitterDurationMillis(15000);
        m_objMediaQualityThreshold.setRtpJitterMillis(100);
    }

    IMS_TRACE_D("UpdateMediaQualityThreshold() - IsHold[%d], RtpInactivity[%d], RtcpInactivity[%d]",
            bIsHold, m_objMediaQualityThreshold.getRtpInactivityTimerMillis(),
            m_objMediaQualityThreshold.getRtcpInactivityTimerMillis());
    IMS_TRACE_D("UpdateMediaQualityThreshold() - PacketLossDurationMillis[%d], PacketLossRate[%d]",
            m_objMediaQualityThreshold.getRtpPacketLossDurationMillis(),
            m_objMediaQualityThreshold.getRtpPacketLossRate(), 0);
    IMS_TRACE_D("UpdateMediaQualityThreshold() - JitterDurationMillis[%d], JitterMillis[%d]",
            m_objMediaQualityThreshold.getJitterDurationMillis(),
            m_objMediaQualityThreshold.getRtpJitterMillis(), 0);

    return IMS_TRUE;
}

PUBLIC
void AudioMediaSession::SetLocalEndPoint(IN IPAddress objLocalAddr, IN IMS_UINT32 nPort)
{
    m_objLocalAddress = objLocalAddr;
    m_nLocalPort = nPort;

    IMS_TRACE_D("UpdateLocalEndPoint() - LocalIP[%s], LocalPort[%d]",
            m_objLocalAddress.ToString().GetStr(), m_nLocalPort, 0);
}

PUBLIC
IMS_BOOL AudioMediaSession::Open()
{
    IMS_TRACE_I("Open() - state[%d]", GetState(), 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgOpenConfigParam* pParam = new ImsMediaMsgOpenConfigParam(MEDIA_TYPE_AUDIO);
        pParam->m_objLocalAddress = m_objLocalAddress;
        pParam->m_nLocalPort = m_nLocalPort;
        pParam->m_pConfig = new AudioConfig(m_objAudioConfig);
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_OPEN_SESSION, pParam);

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
    IMS_TRACE_I("Modify() - state[%d]", GetState(), 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_AUDIO);
        pParam->m_pConfig = new AudioConfig(m_objAudioConfig);
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_MODIFY_SESSION, pParam);

        if (bResult == IMS_TRUE)
        {
            if (MEDIA_DIRECTION_IS_AUDIO_HOLD(GetMediaDirection()))
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
    IMS_TRACE_I("Add() - state[%d]", GetState(), 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_AUDIO);
        pParam->m_pConfig = new AudioConfig(m_objAudioConfig);
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_ADD_CONFIG, pParam);

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
    IMS_TRACE_I("AudioMediaSession::Delete() - state[%d]", GetState(), 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_AUDIO);
        pParam->m_pConfig = new AudioConfig(m_objAudioConfig);
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_DELETE_CONFIG, pParam);

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
    IMS_TRACE_I("Confirm() - state[%d]", GetState(), 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam(MEDIA_TYPE_AUDIO);
        pParam->m_pConfig = new AudioConfig(m_objAudioConfig);
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_CONFIRM_CONFIG, pParam);

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
    IMS_TRACE_I("Close() - state[%d]", GetState(), 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgParamBase* pParam = new ImsMediaMsgParamBase(MEDIA_TYPE_AUDIO);
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_CLOSE_SESSION, pParam);

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

    IMS_TRACE_I("SendDtmf() - cDtmfCode[%d], nDuration[%d]", cDtmfCode,
            m_pConfig->GetDTMFDuration(), 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgDtmfParam* pParam = new ImsMediaMsgDtmfParam();
        pParam->m_dtmfCode = cDtmfCode;
        pParam->m_nDuration = m_pConfig->GetDTMFDuration();
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_SEND_DTMF, pParam);
    }

    return bResult;
}

PUBLIC
IMS_BOOL AudioMediaSession::SetMediaQuality()
{
    IMS_TRACE_I("SetMediaQuality() - state[%d]", GetState(), 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgSetMediaQualityParam* pParam =
                new ImsMediaMsgSetMediaQualityParam(MEDIA_TYPE_AUDIO);
        pParam->m_objMediaQualityThreshold = m_objMediaQualityThreshold;
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_SET_MEDIA_QUALITY, pParam);
    }
    return bResult;
}
