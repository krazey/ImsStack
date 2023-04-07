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

#include <utils/String8.h>

#include "ServiceTrace.h"
#include "ImsProcess.h"

#include "JniEnablerConnector.h"
#include "JniMediaSession.h"
#include "JniMediaSessionThread.h"

#include <AudioConfig.h>
#include <TextConfig.h>

using namespace android::telephony::imsmedia;

__IMS_TRACE_TAG_USER_DECL__("JNI.MEDIA");

JniMediaSession::JniMediaSession(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId,
        IN IMS_SINTP nCallKey, IN IMS_SINTP nNativeObject) :
        BaseService(nSlotId),
        m_pThread(IMS_NULL),
        m_strThreadName(AString::ConstNull()),
        m_nCallKey(nCallKey)
{
    IMS_TRACE_D("+JniMediaSession SlotId[%d], nCallKey[%d]", nSlotId, nCallKey, 0);

    Initialize(pfnSendDataToJava, nNativeObject);
    JniEnablerConnector::GetInstance().SetJniEnabler(
            nSlotId, EnablerType::MEDIA_SESSION, DYNAMIC_CAST(IJniEnabler*, this), m_nCallKey);
}

JniMediaSession::~JniMediaSession()
{
    IMS_TRACE_D("~JniMediaSession", 0, 0, 0);

    JniEnablerConnector::GetInstance().SetJniEnabler(
            GetSlotId(), EnablerType::MEDIA_SESSION, IMS_NULL, m_nCallKey);

    if (m_pThread != IMS_NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_strThreadName);
        m_pThread = IMS_NULL;
    }
}

PUBLIC VIRTUAL int JniMediaSession::SendData(const Parcel& objParcel)
{
    int nMsg = objParcel.readInt32();

    if (IsThreadSwitchingRequired(nMsg))
    {
        SendDataUsingEnablerThread(objParcel);
    }
    else
    {
        HandleMessage(nMsg, objParcel);
    }
    return IMS_TRUE;
}

PUBLIC
void JniMediaSession::Initialize(
        IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINTP nNativeObject)
{
    if (pfnSendDataToJava == IMS_NULL)
    {
        return;
    }

    m_strThreadName.Sprintf("JniMediaSessionThread_%08" PFLS_x, reinterpret_cast<IMS_SINTP>(this));

    IMS_TRACE_D("Initialize()", 0, 0, 0);
    auto fnEntry = []() -> BaseThread*
    {
        return new JniMediaSessionThread();
    };

    ImsProcess::GetInstance()->LoadThread(m_strThreadName, fnEntry, GetSlotId());
    m_pThread = reinterpret_cast<JniMediaSessionThread*>(
            ImsProcess::GetInstance()->GetThread(m_strThreadName));

    if (m_pThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMediaSession : can't create listener thread", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("Initialize()", 0, 0, 0);
    m_pThread->SetCallback(nNativeObject, pfnSendDataToJava);
}

PUBLIC
void JniMediaSession::SetMtcCallId(IN IMS_SINTP nCallKey)
{
    m_nCallKey = nCallKey;
}

PUBLIC VIRTUAL void JniMediaSession::NotifyNativeEnablerSet()
{
    // TODO: send pending messages if needed.
}

PUBLIC
IJniEnablerThread* JniMediaSession::GetJniThread() const
{
    return DYNAMIC_CAST(IJniEnablerThread*, m_pThread);
}

PUBLIC GLOBAL IMS_BOOL JniMediaSession::IsMediaMessage(IN IMS_SINT32 nMsg)
{
    if (nMsg >= IMMedia::MEDIA_MESSAGE_IND_IDX_START && nMsg <= IMMedia::MEDIA_MESSAGE_IND_IDX_END)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL JniMediaSession::IsThreadSwitchingRequired(IN IMS_SINT32 nMsg) const
{
    switch (nMsg)
    {
        // media jni to do
        default:
            return IMS_TRUE;
    }
}

PROTECTED VIRTUAL void JniMediaSession::HandleMessage(
        IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    IMS_TRACE_D("HandleMessage() MSG[%d]", nMsg, 0, 0);

    if (GetMediaManager() == IMS_NULL)
    {
        IMS_TRACE_E(0, "HandleMessage() IJniMediaManager is not exist", 0, 0, 0);
        return;
    }

    switch (nMsg)
    {
        case IMMedia::RESPONSE_OPEN_SESSION:
            OnResponses(nMsg, IMS_FALSE, objParcel);
            break;
        case IMMedia::RESPONSE_MODIFY_SESSION:
        case IMMedia::RESPONSE_ADD_CONFIG:
        case IMMedia::RESPONSE_CONFIRM_CONFIG:
        case IMMedia::NOTIFY_FIRST_PACKET:
            OnResponses(nMsg, IMS_TRUE, objParcel);
            break;
        case IMMedia::NOTIFY_MEDIA_INACTIVITY:
            OnNofityMediaInactitivy(nMsg, objParcel);
            break;
        case IMMedia::NOTIFY_PACKET_LOSS:
            OnNofityPacketLosses(nMsg, objParcel);
            break;
        case IMMedia::NOTIFY_JITTER:
            // TODO : implementation
            break;
        case IMMedia::NOTIFY_CALL_QUALITY_CHANGE:
            OnNofityCallQualityChange(nMsg, objParcel);
            break;
        case IMMedia::NOTIFY_HEADER_EXTENSION:
            OnNofityHeaderExtension(nMsg, objParcel);
            break;
        case IMMedia::NOTIFY_MEDIA_DETACH:
            OnNotifyMediaDetach(nMsg);
            break;
        case IMMedia::NOTIFY_QOS_INFO:
            OnNotifyQosInfo(nMsg, objParcel);
            break;
        case IMMedia::SEND_DTMF:
            OnSendDtmf(nMsg, objParcel);
            break;
        case IMMedia::SETSURFACE_CMD:
        case IMMedia::SELECT_CAMERA_CMD:
        case IMMedia::CHANGE_CAMERA_ZOOM_CMD:
        case IMMedia::CHANGE_ORIENTATION_CMD:
            OnVideoMessage(nMsg, objParcel);
            break;
        default:
            IMS_TRACE_E(0, "HandleMessage() unhandled message", 0, 0, 0);
            break;
    }
}

PRIVATE
IJniMediaManager* JniMediaSession::GetMediaManager()
{
    return DYNAMIC_CAST(IJniMediaManager*,
            JniEnablerConnector::GetInstance().GetNativeEnabler(
                    GetSlotId(), EnablerType::MEDIA_SESSION));
}

PRIVATE
MEDIA_CONTENT_TYPE JniMediaSession::ConvertToMediaType(IN SessionType eSessiontype)
{
    switch (eSessiontype)
    {
        case SESSION_TYPE_VIDEO:
            return MEDIA_TYPE_VIDEO;
        case SESSION_TYPE_RTT:
            return MEDIA_TYPE_TEXT;
        case SESSION_TYPE_AUDIO:
        default:
            return MEDIA_TYPE_AUDIO;
    }
}

PRIVATE
void JniMediaSession::OnResponses(
        IN IMS_SINT32 nMsg, IN IMS_BOOL bNeedConfig, IN const Parcel& objParcel)
{
    MEDIA_CONTENT_TYPE eMediaType = ConvertToMediaType((SessionType)objParcel.readInt32());

    ImsMediaResponseConfigParam* pParam = new ImsMediaResponseConfigParam();
    pParam->m_eMediaType = eMediaType;

    IMS_TRACE_D("OnResponses() - eMediaType[%d], bNeedConfig[%d], nCallKey[%d]",
            pParam->m_eMediaType, bNeedConfig, m_nCallKey);

    if (bNeedConfig == IMS_TRUE)
    {
        if (pParam->m_eMediaType == MEDIA_TYPE_AUDIO)
        {
            AudioConfig* pConfig = new AudioConfig();
            pConfig->readFromParcel(&objParcel);
            pParam->m_pConfig = pConfig;
        }
        else if (pParam->m_eMediaType == MEDIA_TYPE_VIDEO)
        {
            VideoConfig* pConfig = new VideoConfig();
            pConfig->readFromParcel(&objParcel);
            pParam->m_pConfig = pConfig;
        }
        else if (pParam->m_eMediaType == MEDIA_TYPE_TEXT)
        {
            TextConfig* pConfig = new TextConfig();
            pConfig->readFromParcel(&objParcel);
            pParam->m_pConfig = pConfig;
        }
    }

    if (nMsg != IMMedia::NOTIFY_FIRST_PACKET)
    {
        pParam->m_eResult = objParcel.readInt32();
    }

    GetMediaManager()->SendMessage(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::OnNofityMediaInactitivy(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    MEDIA_CONTENT_TYPE eMediaType =
            ConvertToMediaType(static_cast<SessionType>(objParcel.readInt32()));

    if (eMediaType == MEDIA_TYPE_AUDIO)
    {
        ImsMediaNotifyQualityStatusParam* pQualityParam = new ImsMediaNotifyQualityStatusParam();

        pQualityParam->m_eMediaType = eMediaType;
        pQualityParam->m_nRtpInactivityTimerMillis = objParcel.readInt32();
        pQualityParam->m_nRtcpInactivityTimerMillis = objParcel.readInt32();

        GetMediaManager()->SendMessage(
                nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pQualityParam));
    }
    else
    {
        ImsMediaNotifyInactivityParam* pInactivityParam = new ImsMediaNotifyInactivityParam();

        pInactivityParam->m_eMediaType = eMediaType;
        pInactivityParam->m_eMediaProtocolType = static_cast<ProtocolType>(objParcel.readInt32());

        GetMediaManager()->SendMessage(
                nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pInactivityParam));
    }
}

PRIVATE
void JniMediaSession::OnNofityPacketLosses(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    ImsMediaNotifyPacketParam* pParam = new ImsMediaNotifyPacketParam();

    pParam->m_eMediaType = ConvertToMediaType((SessionType)objParcel.readInt32());
    pParam->m_nResponse = objParcel.readInt32();

    GetMediaManager()->SendMessage(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::OnNofityCallQualityChange(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    (void)nMsg;
    (void)objParcel;
}

PRIVATE
void JniMediaSession::OnNofityHeaderExtension(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    (void)nMsg;
    (void)objParcel;
    /** TODO: add implementation later
        ImsMediaHeaderExtensionParam* pParam = new ImsMediaHeaderExtensionParam();

        pParam->m_eMediaType = ConvertToMediaType((SessionType)objParcel.readInt32());

        GetMediaManager()->SendMessage(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
    */
}

PRIVATE
void JniMediaSession::OnNotifyMediaDetach(IN IMS_SINT32 nMsg)
{
    GetMediaManager()->SendMessage(nMsg, m_nCallKey, IMS_NULL);
}

PRIVATE
void JniMediaSession::OnNotifyQosInfo(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    ImsMediaMsgQosParam* pParam =
            new ImsMediaMsgQosParam(ConvertToMediaType((SessionType)objParcel.readInt32()));

    AString strIpAddress;
    ConvertString(objParcel.readString16(), strIpAddress);
    pParam->m_objIpAddress = IpAddress(strIpAddress);
    pParam->m_nPort = objParcel.readInt32();
    pParam->m_bResult = (IMS_BOOL)objParcel.readBool();

    GetMediaManager()->SendMessage(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::OnSendDtmf(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    ImsMediaMsgDtmfParam* pParam = new ImsMediaMsgDtmfParam();

    pParam->m_dtmfCode = objParcel.readByte();
    pParam->m_nDuration = 0;

    GetMediaManager()->SendMessage(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

void JniMediaSession::OnVideoMessage(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    ImsMediaVideoParam* pParam = new ImsMediaVideoParam();
    pParam->nValue = objParcel.readInt32();
    GetMediaManager()->SendMessage(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::ConvertString(IN const String16& strSource, OUT AString& strDest)
{
    String8 str8(strSource);
    strDest = str8.string();
}
