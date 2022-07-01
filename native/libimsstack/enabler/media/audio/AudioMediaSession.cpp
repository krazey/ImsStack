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

// == INCLUDES =============================================================
#include <AudioConfig.h>
#include "ISessionDescriptor.h"
#include "Configuration.h"
#include "ServicePhoneInfo.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceNetwork.h"
#include "ServiceEvent.h"
#include "ServiceSystemTime.h"
#include "ServiceUtil.h"
#include "audio/AudioMediaSession.h"
#include "MediaManager.h"
#include "IMMedia.h"

using namespace android::telephony::imsmedia;

// == DEFINES =============================================================
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
    IMS_TRACE_I("+AudioMediaSession()", 0, 0, 0);
}

PUBLIC
VIRTUAL AudioMediaSession::~AudioMediaSession()
{
    IMS_TRACE_I("~AudioMediaSession()", 0, 0, 0);
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
IMS_BOOL AudioMediaSession::UpdateRtpConfig(
        IN AudioProfile* pSrcProfile, IN AudioProfile* pDestProfile, IN AudioProfile* pNegoProfile)
{
    if (pSrcProfile == IMS_NULL || pDestProfile == IMS_NULL || pNegoProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pNegoProfile->lstPayload.GetSize() == 0 || pDestProfile->lstPayload.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    // Get Negotiated Payload from negotiated Payload index...
    AudioProfile::Payload* pDestPayload;
    AudioProfile::Payload* pNegoPayload;

    IMS_TRACE_D("UpdateRtpConfig() - nNegotiated nDestPIndex[%d], nSrcIndex[%d]",
            pDestProfile->nNegotiatedPayloadIndex, pSrcProfile->nNegotiatedPayloadIndex, 0);

    if (pNegoProfile->nNegotiatedPayloadIndex < 0)
    {
        pNegoPayload = pNegoProfile->lstPayload.GetAt(0);
    }
    else
    {
        pNegoPayload = pNegoProfile->lstPayload.GetAt(pNegoProfile->nNegotiatedPayloadIndex);
    }

    if (pDestProfile->nNegotiatedPayloadIndex < 0)
    {
        pDestPayload = pDestProfile->lstPayload.GetAt(0);
    }
    else
    {
        pDestPayload = pDestProfile->lstPayload.GetAt(pDestProfile->nNegotiatedPayloadIndex);
    }

    if (pNegoPayload == IMS_NULL || pDestPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Setting the network properties
    UpdateLocalEndPoint(pNegoProfile);

    if (pSrcProfile->nNegotiatedPayloadIndex < 0)
    {
        m_objAudioConfig.setTxPayloadTypeNumber((int32_t)pNegoPayload->objRtpMap.nPayloadNum);
    }
    else
    {
        AudioProfile::Payload* pSrcPayload =
                pSrcProfile->lstPayload.GetAt(pSrcProfile->nNegotiatedPayloadIndex);
        if (pSrcPayload == IMS_NULL)
        {
            return IMS_FALSE;
        }

        m_objAudioConfig.setTxPayloadTypeNumber((int32_t)pSrcPayload->objRtpMap.nPayloadNum);
    }

    // remote network parameters
    m_objAudioConfig.setRemoteAddress(
            android::String8(pDestProfile->objIpAddr.ToString().GetStr()));
    m_objAudioConfig.setRemotePort(pDestProfile->nDataPort);
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
    IMS_TRACE_D("UpdateRtpConfig() - Dscp[%d], , MediaDirection[%d]", m_objAudioConfig.getDscp(),
            m_objAudioConfig.getMediaDirection(), 0);

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

        if (pSrcProfile->nNegotiatedPayloadIndex >= 0)
        {
            AudioProfile::Payload* pSrcPayload =
                    pSrcProfile->lstPayload.GetAt(pSrcProfile->nNegotiatedPayloadIndex);

            if (pSrcPayload != IMS_NULL && pSrcPayload->pFmtp != IMS_NULL)
            {
                m_objAudioConfig.setDtxEnabled(
                        ((AudioProfile::AmrFmtp*)pSrcPayload->pFmtp)->bSCREnable);
            }
        }

        AmrParams* pAmrParams = new AmrParams();
        IMS_SINT32 maxModeSet = -1;
        if (pNegoPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB"))
        {
            maxModeSet = AudioProfileConfigurer::GetLargestModesetInFmtp("AMR-WB", pNegoPayload);
        }
        else
        {
            maxModeSet = AudioProfileConfigurer::GetLargestModesetInFmtp("AMR", pNegoPayload);
        }

        if (maxModeSet == -1)
        {
            maxModeSet = 0;
        }

        pAmrParams->setAmrMode((int32_t) static_cast<AmrParams::AmrMode>(maxModeSet));

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

        // tx side
        pEvsParams->setUseHeaderFullOnlyOnTx((IMS_BOOL)pFmtp->nHfOnly);

        // rx side
        pEvsParams->setUseHeaderFullOnlyOnRx((IMS_BOOL)pDestFmtp->nHfOnly);

        IMS_SINT32 maxModeSet =
                AudioProfileConfigurer::GetLargestModesetInFmtp("EVS", pNegoPayload);
        if (maxModeSet == -1)
        {
            maxModeSet = 0;
        }
        // evs primary mode conversion
        if (pFmtp->nEvsModeSwitch == 0)
        {  // evs primary mode
            maxModeSet += EvsParams::EVS_MODE_9;
        }
        pEvsParams->setEvsMode((int32_t) static_cast<EvsParams::EvsMode>(maxModeSet));

        // update bandwidth
        IMS_SINT32 nEvsBandwidth = pFmtp->nBwList;
        // exception : evs AMR-WB IO mode
        if (pFmtp->nEvsModeSwitch == 1)
        {
            nEvsBandwidth = EvsParams::EVS_WIDE_BAND;
        }
        pEvsParams->setEvsBandwidth(nEvsBandwidth);

        m_objAudioConfig.setEvsParams(*pEvsParams);
        m_objAudioConfig.setCodecModeRequest((int8_t)pFmtp->bSendCmr);

        delete pEvsParams;

        EvsParams objEvsParams = m_objAudioConfig.getEvsParams();
        IMS_TRACE_D("UpdateRtpConfig() - EvsMode[%d], ChannelAwareMode[%d]",
                objEvsParams.getEvsMode(), objEvsParams.getChannelAwareMode(), 0);
        IMS_TRACE_D("UpdateRtpConfig() - UseHeaderFullOnlyOnTx[%d], setUseHeaderFullOnlyOnRx[%d]",
                objEvsParams.getUseHeaderFullOnlyOnTx(), objEvsParams.getUseHeaderFullOnlyOnRx(),
                0);
        IMS_TRACE_D("UpdateRtpConfig() - EVS nBandwidth[0x%08x], CodecModeRequest[0x%08x]",
                objEvsParams.getEvsBandwidth(), m_objAudioConfig.getCodecModeRequest(), 0);
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
        IMS_TRACE_E(0, "UpdateRtpConfig() - Invalid", 0, 0, 0);
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
            m_objAudioConfig.setDtmfPayloadTypeNumber(pPayload->objRtpMap.nPayloadNum);
            m_objAudioConfig.setDtmfsamplingRateKHz(pPayload->objRtpMap.nSamplingRate / 1000);
            break;
        }
    }

    IMS_TRACE_D("UpdateRtpConfig() - DtmfPayloadTypeNumber[%d], DtmfsamplingRateKHz[%d]",
            m_objAudioConfig.getDtmfPayloadTypeNumber(), m_objAudioConfig.getDtmfsamplingRateKHz(),
            0);

    // jitter options // NEXT_ITEM
    /*
        m_objAudioConfig.sessionParams.jitterBufferParams.minJitterBufferSizeMillis =
                m_pConfig->GetJitterBufferMinSize();
        m_objAudioConfig.sessionParams.jitterBufferParams.maxJitterBufferSizeMillis =
                m_pConfig->GetJitterBufferMaxSize();
        m_objAudioConfig.sessionParams.jitterBufferParams.jitterBufferAdjustTimerMillis = 80;
        m_objAudioConfig.sessionParams.jitterBufferParams.bufferStepSizeMillis = 1;

        IMS_TRACE_I("UpdateRtpConfig() - set JB min[%d], max[%d]",
                m_objAudioConfig.sessionParams.jitterBufferParams.minJitterBufferSizeMillis,
                m_objAudioConfig.sessionParams.jitterBufferParams.maxJitterBufferSizeMillis, 0);
        IMS_TRACE_I("UpdateRtpConfig() - set JB th[%d],step[%d]",
                m_objAudioConfig.sessionParams.jitterBufferParams.jitterBufferAdjustTimerMillis,
                m_objAudioConfig.sessionParams.jitterBufferParams.bufferStepSizeMillis, 0);
    */
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioMediaSession::IsDirectionHold()
{
    IMS_UINT32 nDirection = m_objAudioConfig.getMediaDirection();
    IMS_TRACE_D("IsDirectionHold() - m_objAudioConfig direction[%d]", nDirection, 0, 0);
    return (nDirection == (IMS_UINT32)RtpConfig::MEDIA_DIRECTION_SEND_RECEIVE) ? IMS_FALSE
                                                                               : IMS_TRUE;
}

PUBLIC
void AudioMediaSession::HoldRtpConfig()
{
    m_objAudioConfig.setMediaDirection((int32_t)RtpConfig::MEDIA_DIRECTION_RECEIVE_ONLY);
}

PUBLIC
IMS_BOOL AudioMediaSession::UpdateMediaQualityThreshold(
        IN IMS_BOOL bIsHold, IN AudioProfile* audioProfile)
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
        if (audioProfile && audioProfile->nBandwidthRs == 0 && audioProfile->nBandwidthRr == 0)
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
IMS_BOOL AudioMediaSession::UpdateLocalEndPoint(IN AudioProfile* pNegoProfile)
{
    if (pNegoProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_objLocalAddress = pNegoProfile->objIpAddr;
    m_nLocalPort = pNegoProfile->nDataPort;

    IMS_TRACE_D("UpdateLocalEndPoint() - LocalIP[%s], LocalPort[%d]",
            m_objLocalAddress.ToString().GetStr(), m_nLocalPort, 0);
    // modem id??
    return IMS_TRUE;
}

PUBLIC
void AudioMediaSession::UpdateLocalEndPoint(IN IPAddress objLocalAddr, IN IMS_UINT32 nPort)
{
    m_objLocalAddress = objLocalAddr;
    m_nLocalPort = nPort;

    IMS_TRACE_D("UpdateLocalEndPoint() - LocalIP[%s], LocalPort[%d]",
            m_objLocalAddress.ToString().GetStr(), m_nLocalPort, 0);
}

PUBLIC
IMS_BOOL AudioMediaSession::Open()
{
    IMS_TRACE_I("Open()", 0, 0, 0);
    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgOpenConfigParam* pParam = new ImsMediaMsgOpenConfigParam();
        pParam->m_eMediaType = MEDIA_TYPE_AUDIO;
        pParam->m_objLocalAddress = m_objLocalAddress;
        pParam->m_nLocalPort = m_nLocalPort;
        pParam->m_objAudioConfig = m_objAudioConfig;
        m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_OPEN_SESSION, pParam);
    }
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioMediaSession::Modify()
{
    IMS_TRACE_I("Modify()", 0, 0, 0);
    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam();
        pParam->m_eMediaType = MEDIA_TYPE_AUDIO;
        pParam->m_objAudioConfig = m_objAudioConfig;
        m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_MODIFY_SESSION, pParam);
    }
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioMediaSession::Add()
{
    IMS_TRACE_I("Add()", 0, 0, 0);
    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam();
        pParam->m_eMediaType = MEDIA_TYPE_AUDIO;
        pParam->m_objAudioConfig = m_objAudioConfig;
        m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_ADD_CONFIG, pParam);
    }
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioMediaSession::Delete()
{
    IMS_TRACE_I("AudioMediaSession::Delete()", 0, 0, 0);
    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam();
        pParam->m_eMediaType = MEDIA_TYPE_AUDIO;
        pParam->m_objAudioConfig = m_objAudioConfig;
        m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_DELETE_CONFIG, pParam);
    }
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioMediaSession::Confirm()
{
    IMS_TRACE_I("Confirm()", 0, 0, 0);
    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgConfigParam* pParam = new ImsMediaMsgConfigParam();
        pParam->m_eMediaType = MEDIA_TYPE_AUDIO;
        pParam->m_objAudioConfig = m_objAudioConfig;
        m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_CONFIRM_CONFIG, pParam);
    }
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioMediaSession::Close()
{
    IMS_TRACE_I("Close()", 0, 0, 0);
    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgParamBase* pParam = new ImsMediaMsgParamBase();
        pParam->m_eMediaType = MEDIA_TYPE_AUDIO;
        m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_CLOSE_SESSION, pParam);
    }
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioMediaSession::SendDtmf(IN IMS_CHAR cDtmfCode, IN IMS_SINT32 nDuration)
{
    IMS_TRACE_I("SendDtmf() - cDtmfCode[%d], nDuration[%d]", cDtmfCode, nDuration, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgDtmfParam* pParam = new ImsMediaMsgDtmfParam();
        pParam->m_eMediaType = MEDIA_TYPE_AUDIO;
        pParam->m_dtmfCode = cDtmfCode;
        pParam->m_nDuration = nDuration;
        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_SEND_DTMF, pParam);
    }
    return bResult;
}

PUBLIC
IMS_BOOL AudioMediaSession::SetMediaQuality()
{
    IMS_TRACE_I("SetMediaQuality()", 0, 0, 0);
    IMS_BOOL bResult = IMS_FALSE;

    if (m_piMediaSessionListener != IMS_NULL)
    {
        ImsMediaMsgSetMediaQualityParam* pParam = new ImsMediaMsgSetMediaQualityParam();
        pParam->m_eMediaType = MEDIA_TYPE_AUDIO;
        pParam->m_objMediaQualityThreshold = m_objMediaQualityThreshold;

        bResult = m_piMediaSessionListener->MediaSession_SendMsgToMediaManager(
                IMMedia::REQUEST_SET_MEDIA_QUALITY, pParam);
    }
    return bResult;
}

PRIVATE
void AudioMediaSession::SendEventToUi(IMS_SINT32 nEvent, IMS_SINT32 nResult)
{
    IMS_TRACE_I("SendEventToUi() - nEvent[%d], nResult[%d]", nEvent, 0, 0);

    if (nEvent != -1 && m_piMediaSessionListener != IMS_NULL)
    {
        m_piMediaSessionListener->MediaSession_SendEventToUi(nEvent, nResult);
    }
}
