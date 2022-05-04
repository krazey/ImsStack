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

#include <binder/Parcel.h>
#include "JniMtcCall.h"
#include "JniMediaSessionThread.h"
#include "JniMtcUtils.h"
#include "ServiceTrace.h"

using namespace android;
using namespace android::telephony::imsmedia;

__IMS_TRACE_TAG_USER_DECL__("JNI.MEDIA");

PUBLIC
JniMediaSessionThread::JniMediaSessionThread() :
        BaseServiceThread(),
        m_nSlotId(0)
{
    IMS_TRACE_D("+JniMediaSessionThread", 0, 0, 0);
}

PUBLIC VIRTUAL JniMediaSessionThread::~JniMediaSessionThread()
{
    IMS_TRACE_D("~JniMediaSessionThread", 0, 0, 0);
}

PUBLIC
void JniMediaSessionThread::SetSlotId(IN IMS_SINT32 nSlotId)
{
    m_nSlotId = nSlotId;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnOpenSession(IN ImsMediaMsgOpenConfigParam* pParam)
{
    IMS_TRACE_D("OnOpenSession", 0, 0, 0);

    Parcel objParcel;

    objParcel.writeInt32(IMMedia::REQUEST_OPEN_SESSION);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    objParcel.writeString16(android::String16(pParam->m_objLocalAddress.ToString().GetStr()));
    objParcel.writeInt32(pParam->m_nLocalPort);
    // pParam->m_objAudioConfig.writeToParcel(&objParcel);

    delete pParam;

    SendData2Java(objParcel, IMS_TRUE);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnModifySession(IN ImsMediaMsgConfigParam* pParam)
{
    IMS_TRACE_D("OnModifySession", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IMMedia::REQUEST_MODIFY_SESSION);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    pParam->m_objAudioConfig.writeToParcel(&objParcel);

    delete pParam;

    SendData2Java(objParcel, IMS_TRUE);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnCloseSession(IN ImsMediaMsgParamBase* pParam)
{
    IMS_TRACE_D("OnCloseSession", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IMMedia::REQUEST_CLOSE_SESSION);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));

    delete pParam;

    SendData2Java(objParcel, IMS_TRUE);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnAddConfig(IN ImsMediaMsgConfigParam* pParam)
{
    IMS_TRACE_D("OnAddConfig", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IMMedia::REQUEST_ADD_CONFIG);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    pParam->m_objAudioConfig.writeToParcel(&objParcel);

    delete pParam;

    SendData2Java(objParcel, IMS_TRUE);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnDeleteConfig(IN ImsMediaMsgConfigParam* pParam)
{
    IMS_TRACE_D("OnDeleteConfig", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IMMedia::REQUEST_DELETE_CONFIG);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    pParam->m_objAudioConfig.writeToParcel(&objParcel);

    delete pParam;

    SendData2Java(objParcel, IMS_TRUE);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnConfirmConfig(IN ImsMediaMsgConfigParam* pParam)
{
    IMS_TRACE_D("OnConfirmConfig", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IMMedia::REQUEST_CONFIRM_CONFIG);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    pParam->m_objAudioConfig.writeToParcel(&objParcel);

    delete pParam;

    SendData2Java(objParcel, IMS_TRUE);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnSendDtmf(IN ImsMediaMsgDtmfParam* pParam)
{
    IMS_TRACE_D("OnSendDtmf", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IMMedia::REQUEST_SEND_DTMF);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    objParcel.writeByte(pParam->m_dtmfCode);
    objParcel.writeInt32(pParam->m_nDuration);

    delete pParam;

    SendData2Java(objParcel, IMS_TRUE);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnSetMediaQualityThreshold(
        IN ImsMediaMsgSetMediaQualityParam* pParam)
{
    IMS_TRACE_D("OnSetMediaQualityThreshold", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IMMedia::REQUEST_SET_MEDIA_QUALITY);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    pParam->m_objMediaQualityThreshold.writeToParcel(&objParcel);

    delete pParam;

    SendData2Java(objParcel, IMS_TRUE);

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL JniMediaSessionThread::IsThreadSwitchingRequired(
        IN IMS_SINT32 nMsg) const
{
    switch (nMsg)
    {
        default:
        case IMMedia::REQUEST_OPEN_SESSION:
            return IMS_TRUE;
            break;
    }
    return IMS_TRUE;
}

PRIVATE
SessionType JniMediaSessionThread::ConvertToSessionType(IN MEDIA_CONTENT_TYPE eMediaType)
{
    switch (eMediaType)
    {
        case MEDIA_TYPE_VIDEO:
            return SESSION_TYPE_VIDEO;
        case MEDIA_TYPE_TEXT:
            return SESSION_TYPE_RTT;
        case MEDIA_TYPE_AUDIO:
        default:
            return SESSION_TYPE_AUDIO;
    }
}
