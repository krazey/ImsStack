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

#define IMS_STL_USE
#include <gui/Surface.h>
#include <utils/String8.h>

#include "ServiceTrace.h"
#include "ImsProcess.h"

#include "JniConnectorFactory.h"
#include "JniMediaSession.h"
#include "JniMediaSessionThread.h"
#include "SurfaceManager.h"

__IMS_TRACE_TAG_USER_DECL__("JNI.MEDIA");

extern int IMSInterface_GetSurface(const android::String8& str8Class,
        const android::String8& str8SurfaceName, long& nSurfaceObject);

JniMediaSession::JniMediaSession(IN CBServiceNoti pfnNotifier, IN IMS_SINT32 nSlotId,
        IN IMS_SINTP nCallKey, IN IMS_SINTP nNativeObject) :
        m_pThread(IMS_NULL),
        m_strThreadName(AString::ConstNull()),
        m_nSlotId(nSlotId),
        m_piMediaManager(IMS_NULL),
        m_nCallKey(nCallKey)
{
    IMS_TRACE_D("+JniMediaSession SlotId[%d], nCallKey[%d]", m_nSlotId, nCallKey, 0);

    Initialize(pfnNotifier, nNativeObject);
    m_piMediaManager = JniConnectorFactory::GetInstance()
                               ->GetMediaSessionConnector(m_nSlotId)
                               ->GetEnablerService();

    SetJniMediaSessionThread();
}

JniMediaSession::~JniMediaSession()
{
    IMS_TRACE_D("~JniMediaSession", 0, 0, 0);

    if (m_pThread != IMS_NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_strThreadName);
        m_pThread = IMS_NULL;
    }
    SetJniMediaSessionThread();
}

PUBLIC VIRTUAL int JniMediaSession::SendData(const android::Parcel& objParcel)
{
    int nMsg = objParcel.readInt32();

    if (IsThreadSwitchingRequired(nMsg))
    {
        SendDataUsingEnablerThread(objParcel, m_nSlotId);
    }
    else
    {
        HandleMessage(nMsg, objParcel);
    }
    return IMS_TRUE;
}

PUBLIC
void JniMediaSession::Initialize(IN CBServiceNoti pfnNotifier, IN IMS_SINTP nNativeObject)
{
    if (pfnNotifier == IMS_NULL)
    {
        return;
    }
    m_strThreadName.Sprintf("JniMediaSessionThread_%08" PFLS_x, reinterpret_cast<IMS_SINTP>(this));

    IMS_TRACE_D("Initialize()", 0, 0, 0);
    auto fnEntry = []() -> BaseThread*
    {
        return new JniMediaSessionThread();
    };
    ImsProcess::GetInstance()->LoadThread(m_strThreadName, fnEntry);
    m_pThread = (JniMediaSessionThread*)(ImsProcess::GetInstance()->GetThread(m_strThreadName));

    if (m_pThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMediaSession : can't create listener thread", 0, 0, 0);
        return;
    }
    IMS_TRACE_D("Initialize()", 0, 0, 0);
    m_pThread->SetSlotId(m_nSlotId);
    m_pThread->SetCallback(nNativeObject, pfnNotifier);
}

PUBLIC
void JniMediaSession::SetMtcCallId(IN IMS_SINTP nCallKey)
{
    m_nCallKey = nCallKey;
}

PUBLIC
JniMediaSessionThread* JniMediaSession::GetThread()
{
    return m_pThread;
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
        IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    IMS_TRACE_D("HandleMessage() MSG[%d]", nMsg, 0, 0);

    if (m_piMediaManager == IMS_NULL)
    {
        m_piMediaManager = JniConnectorFactory::GetInstance()
                                   ->GetMediaSessionConnector(m_nSlotId)
                                   ->GetEnablerService();
    }

    if (m_piMediaManager == IMS_NULL)
    {
        IMS_TRACE_E(0, "HandleMessage() m_piMediaManager is not exist", 0, 0, 0);
        return;
    }

    IMS_BOOL bNeedConfig = IMS_FALSE;
    switch (nMsg)
    {
        case IMMedia::RESPONSE_OPEN_SESSION:
            bNeedConfig = IMS_FALSE;
            OnResponses(nMsg, bNeedConfig, objParcel);
            break;
        case IMMedia::RESPONSE_MODIFY_SESSION:
        case IMMedia::RESPONSE_ADD_CONFIG:
        case IMMedia::RESPONSE_CONFIRM_CONFIG:
        case IMMedia::NOTIFY_FIRST_PACKET:
            bNeedConfig = IMS_TRUE;
            OnResponses(nMsg, bNeedConfig, objParcel);
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
        case IMMedia::NOTIFY_MEDIA_QUALITY_CHANGE:
            OnNofityMediaQualityChange(nMsg, objParcel);
            break;
        case IMMedia::RESPONSE_SESSION_CHANGED:
            OnResponseSessionChanged(nMsg, objParcel);
            break;
        case IMMedia::NOTIFY_HEADER_EXTENSION:
            OnNofityHeaderExtension(nMsg, objParcel);
            break;
        case IMMedia::NOTIFY_QOS_INFO:
            OnNotifyQosInfo(nMsg, objParcel);
            break;
        case IMMedia::NOTIFY_MEDIA_DETACH:
            OnNotifyMediaDetach(nMsg);
            break;
        case IMMedia::SETSURFACE_CMD:
            OnCmdSetSurface(nMsg, objParcel);
            break;
        case IMMedia::SELECT_CAMERA_CMD:
            OnCmdSelectCamera(nMsg, objParcel);
            break;
        case IMMedia::CHANGE_CAMERA_ZOOM_CMD:
            OnCmdChangeCameraZoom(nMsg, objParcel);
            break;
        case IMMedia::CHANGE_ORIENTATION_CMD:
            OnCmdOrientationChanged(nMsg, objParcel);
            break;
        default:
            IMS_TRACE_E(0, "HandleMessage() unhandled message", 0, 0, 0);
            break;
    }
}

PRIVATE
void JniMediaSession::SetJniMediaSessionThread()
{
    if (m_piMediaManager != IMS_NULL)
    {
        m_piMediaManager->SetJniMediaSessionThread(m_nCallKey, m_pThread);
    }
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
        IN IMS_SINT32 nMsg, IN IMS_BOOL bNeedConfig, IN const android::Parcel& objParcel)
{
    ImsMediaResponseConfigParam* pParam = new ImsMediaResponseConfigParam();

    pParam->m_eMediaType = ConvertToMediaType((SessionType)objParcel.readInt32());
    IMS_TRACE_D("OnResponses() m_eMediaType[%d], bNeedConfig[%d], m_nCallKey[%d]",
            pParam->m_eMediaType, bNeedConfig, m_nCallKey);

    if (bNeedConfig == IMS_TRUE)
    {
        pParam->m_objAudioConfig.readFromParcel(&objParcel);
    }
    if (nMsg != IMMedia::NOTIFY_FIRST_PACKET)
    {
        pParam->m_eResult = (RtpError)(objParcel.readInt32());
    }

    m_piMediaManager->OnResponse(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::OnNofityMediaInactitivy(
        IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    ImsMediaNotifyInactivityParam* pParam = new ImsMediaNotifyInactivityParam();

    pParam->m_eMediaType = ConvertToMediaType((SessionType)objParcel.readInt32());
    pParam->m_eMediaProtocolType = (MEDIA_TRANSPORT_PROTOCOL)(objParcel.readInt32());

    m_piMediaManager->OnResponse(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::OnNofityPacketLosses(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    ImsMediaNotifyPacketParam* pParam = new ImsMediaNotifyPacketParam();

    pParam->m_eMediaType = ConvertToMediaType((SessionType)objParcel.readInt32());
    pParam->m_nResponse = objParcel.readInt32();

    m_piMediaManager->OnResponse(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::OnNofityMediaQualityChange(
        IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    (void)nMsg;
    (void)objParcel;
    /*
        ImsMediaNotifyQualityParam* pParam = new ImsMediaNotifyQualityParam();

        pParam->m_eMediaType = ConvertToMediaType((SessionType)objParcel.readInt32());
        // to do later :: CallQuality -> Param

        m_piMediaManager->OnResponse(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
    */
}

PRIVATE
void JniMediaSession::OnResponseSessionChanged(
        IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    (void)nMsg;
    (void)objParcel;
    /*
        ImsMediaSessionChangedParam* pParam = new ImsMediaSessionChangedParam();

        pParam->m_eMediaType = ConvertToMediaType((SessionType)objParcel.readInt32());

        m_piMediaManager->OnResponse(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
    */ // NEXT_ITEM :: OnSessionChanged
}

PRIVATE
void JniMediaSession::OnNofityHeaderExtension(
        IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    (void)nMsg;
    (void)objParcel;
    /*
        ImsMediaHeaderExtensionParam* pParam = new ImsMediaHeaderExtensionParam();

        pParam->m_eMediaType = ConvertToMediaType((SessionType)objParcel.readInt32());

        m_piMediaManager->OnResponse(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
    */ // NEXT_ITEM :: OnNofityHeaderExtension
}

PRIVATE
void JniMediaSession::OnNotifyQosInfo(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    ImsMediaNotifyQosParam* pParam = new ImsMediaNotifyQosParam();

    pParam->m_eMediaType = ConvertToMediaType((SessionType)objParcel.readInt32());
    AString strIpAddress;
    ConvertString(objParcel.readString16(), strIpAddress);
    pParam->m_objIpAddr = IPAddress(strIpAddress);
    pParam->m_nPort = objParcel.readInt32();
    pParam->m_bResult = (IMS_BOOL)objParcel.readBool();

    m_piMediaManager->OnResponse(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::OnNotifyMediaDetach(IN IMS_SINT32 nMsg)
{
    m_piMediaManager->OnResponse(nMsg, m_nCallKey, IMS_NULL);
}

PRIVATE
void JniMediaSession::OnCmdSetSurface(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    ImsMediaVideoParam* pParam = new ImsMediaVideoParam();
    pParam->nValue = objParcel.readInt32();
    m_piMediaManager->OnVideoMessage(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::OnCmdSelectCamera(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    ImsMediaVideoParam* pParam = new ImsMediaVideoParam();
    pParam->nValue = objParcel.readInt32();
    m_piMediaManager->OnVideoMessage(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::OnCmdChangeCameraZoom(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    ImsMediaVideoParam* pParam = new ImsMediaVideoParam();
    pParam->nValue = objParcel.readInt32();
    m_piMediaManager->OnVideoMessage(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::OnCmdOrientationChanged(
        IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    ImsMediaVideoParam* pParam = new ImsMediaVideoParam();
    pParam->nValue = objParcel.readInt32();
    m_piMediaManager->OnVideoMessage(nMsg, m_nCallKey, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void JniMediaSession::ConvertString(IN const android::String16& strSource, OUT AString& strDest)
{
    android::String8 str8(strSource);
    strDest = str8.string();
}
