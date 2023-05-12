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
#include "IJniMedia.h"

using namespace android;
using namespace android::telephony::imsmedia;

__IMS_TRACE_TAG_USER_DECL__("JNI.MEDIA");

PUBLIC
JniMediaSessionThread::JniMediaSessionThread() :
        BaseServiceThread()
{
    IMS_TRACE_D("+JniMediaSessionThread", 0, 0, 0);
}

PUBLIC VIRTUAL JniMediaSessionThread::~JniMediaSessionThread()
{
    IMS_TRACE_D("~JniMediaSessionThread", 0, 0, 0);
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnOpenSession(IN ImsMediaMsgOpenConfigParam* pParam)
{
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("OnOpenSession type[%d]", pParam->m_eMediaType, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_OPEN_SESSION);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    objParcel.writeString16(android::String16(pParam->m_objLocalAddress.ToString().GetStr()));
    objParcel.writeInt32(pParam->m_nLocalPort);

    if (pParam->m_pConfig != NULL)
    {
        pParam->m_pConfig->writeToParcel(&objParcel);
    }

    SendData2Java(objParcel);
    delete pParam;
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnModifySession(IN ImsMediaMsgConfigParam* pParam)
{
    if (pParam == IMS_NULL || pParam->m_pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("OnModifySession type[%d]", pParam->m_eMediaType, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_MODIFY_SESSION);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    pParam->m_pConfig->writeToParcel(&objParcel);
    SendData2Java(objParcel);
    delete pParam;
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnCloseSession(IN ImsMediaMsgParamBase* pParam)
{
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("OnCloseSession type[%d]", pParam->m_eMediaType, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_CLOSE_SESSION);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    SendData2Java(objParcel);
    delete pParam;
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnAddConfig(IN ImsMediaMsgConfigParam* pParam)
{
    if (pParam == IMS_NULL || pParam->m_pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("OnAddConfig", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_ADD_CONFIG);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    pParam->m_pConfig->writeToParcel(&objParcel);
    SendData2Java(objParcel);
    delete pParam;
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnDeleteConfig(IN ImsMediaMsgConfigParam* pParam)
{
    if (pParam == IMS_NULL || pParam->m_pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("OnDeleteConfig", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_DELETE_CONFIG);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    pParam->m_pConfig->writeToParcel(&objParcel);
    SendData2Java(objParcel);
    delete pParam;
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnConfirmConfig(IN ImsMediaMsgConfigParam* pParam)
{
    if (pParam == IMS_NULL || pParam->m_pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("OnConfirmConfig", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_CONFIRM_CONFIG);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    pParam->m_pConfig->writeToParcel(&objParcel);
    SendData2Java(objParcel);
    delete pParam;
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnSendDtmf(IN ImsMediaMsgDtmfParam* pParam)
{
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("OnSendDtmf", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_SEND_DTMF);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    objParcel.writeByte(pParam->m_dtmfCode);
    objParcel.writeInt32(pParam->m_nDuration);
    SendData2Java(objParcel);
    delete pParam;
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnSetMediaQualityThreshold(
        IN ImsMediaMsgSetMediaQualityParam* pParam)
{
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("OnSetMediaQualityThreshold - type[%d]", pParam->m_eMediaType, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_SET_MEDIA_QUALITY);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    pParam->m_objMediaQualityThreshold.writeToParcel(&objParcel);
    SendData2Java(objParcel);
    delete pParam;
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL JniMediaSessionThread::OnRequestQos(IN ImsMediaMsgQosParam* pParam)
{
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("OnRequestQos() - type[%d]", pParam->m_eMediaType, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_QOS);
    objParcel.writeInt32((IMS_UINT32)ConvertToSessionType(pParam->m_eMediaType));
    objParcel.writeString16(android::String16(pParam->m_objIpAddress.ToString().GetStr()));
    objParcel.writeInt32(pParam->m_nPort);
    SendData2Java(objParcel);
    delete pParam;
    return IMS_TRUE;
}

PUBLIC
void JniMediaSessionThread::OnSetPreviewSurface()
{
    IMS_TRACE_D("OnSetPreviewSurface", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_SET_PREVIEW_SURFACE);
    objParcel.writeInt32((IMS_UINT32)SESSION_TYPE_VIDEO);
    SendData2Java(objParcel);
}

PUBLIC
void JniMediaSessionThread::OnSetDisplaySurface()
{
    IMS_TRACE_D("OnSetDisplaySurface", 0, 0, 0);
    Parcel objParcel;
    objParcel.writeInt32(IJniMedia::REQUEST_SET_DISPLAY_SURFACE);
    objParcel.writeInt32((IMS_UINT32)SESSION_TYPE_VIDEO);
    SendData2Java(objParcel);
}

PROTECTED VIRTUAL IMS_BOOL JniMediaSessionThread::IsThreadSwitchingRequired(
        IN IMS_SINT32 nMsg) const
{
    (void)nMsg;
    return IMS_FALSE;
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
