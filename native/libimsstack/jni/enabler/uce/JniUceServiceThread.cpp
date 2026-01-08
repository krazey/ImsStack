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

#include "JniUceServiceThread.h"

#include "IUce.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.UCE");

PUBLIC
JniUceServiceThread::JniUceServiceThread() :
        BaseServiceThread()
{
    IMS_TRACE_D("UCE_M : JniUceServiceThread = %" PFLS_u, sizeof(JniUceServiceThread), 0, 0);
    IMS_TRACE_I("JniUceServiceThread : ", 0, 0, 0);
}

PUBLIC VIRTUAL JniUceServiceThread::~JniUceServiceThread()
{
    IMS_TRACE_D("UCE_F : JniUceServiceThread = %" PFLS_u, sizeof(JniUceServiceThread), 0, 0);
    IMS_TRACE_I("~JniUceServiceThread : ", 0, 0, 0);
}

PUBLIC IMS_BOOL JniUceServiceThread::NotifyImsDeregistered()
{
    IMS_TRACE_D("NotifyImsDeregistered", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_IMS_AGENT_DISCONNECTED_IND);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::NotifyImsRegistered(
        IN IMS_UINT32 registeredService, IN IMS_SINT32 registeredNetwork)
{
    IMS_TRACE_D("NotifyImsRegistered", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_IMS_AGENT_CONNECTED_IND);
    objParcel.writeInt64(registeredService);
    objParcel.writeInt32(registeredNetwork);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::PublishResponseInd(IMS_UINT32 key, IMS_UINT32 responseCode,
        IMS_UINT32 capability, AString reason, IMS_UINT32 reasonHeaderCause,
        AString reasonHeaderText, AString etag, IMS_UINT32 needToRetry)
{
    IMS_TRACE_D("PublishResponseInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_PUBLISH_RESPONSE_IND);
    objParcel.writeInt32(key);
    objParcel.writeInt64(capability);
    objParcel.writeInt32(responseCode);
    objParcel.writeString16(android::String16(reason.GetStr()));
    objParcel.writeInt32(reasonHeaderCause);
    objParcel.writeString16(android::String16(reasonHeaderText.GetStr()));
    objParcel.writeString16(android::String16(etag.GetStr()));
    objParcel.writeInt32(needToRetry);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::PublishUpdatedInd(IMS_UINT32 capability,
        IMS_SINT32 responseCode, AString reason, IMS_SINT32 reasonHeaderCause,
        AString reasonHeaderText, AString eTag, IMS_UINT32 needToRetry)
{
    IMS_TRACE_D("PublishUpdatedInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_PUBLISH_UPDATED_IND);
    objParcel.writeInt64(capability);
    objParcel.writeInt32(responseCode);
    objParcel.writeString16(android::String16(reason.GetStr()));
    objParcel.writeInt32(reasonHeaderCause);
    objParcel.writeString16(android::String16(reasonHeaderText.GetStr()));
    objParcel.writeString16(android::String16(eTag.GetStr()));
    objParcel.writeInt32(needToRetry);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::PublishErrorInd(IMS_UINT32 key, IMS_UINT32 commandError)
{
    IMS_TRACE_D("PublishErrorInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_PUBLISH_CMD_ERROR_IND);
    objParcel.writeInt32(key);
    objParcel.writeInt32(commandError);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::UnPublishedInd()
{
    IMS_TRACE_D("UnPublishedInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_UNPUBLISHED_IND);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::SubscribeResponseInd(IMS_UINT32 key, IMS_SINT32 responseCode,
        AString reason, IMS_SINT32 reasonHeaderCause, AString reasonHeaderText)
{
    IMS_TRACE_D("SubscribeResponseInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_SUBSCRIBE_RESPONSE_IND);
    objParcel.writeInt32(key);
    objParcel.writeInt32(responseCode);
    objParcel.writeString16(android::String16(reason.GetStr()));
    objParcel.writeInt32(reasonHeaderCause);
    objParcel.writeString16(android::String16(reasonHeaderText.GetStr()));
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::NotifyInd(
        IMS_UINT32 key, IMS_UINT32 count, ImsList<AString> pidfXmls)
{
    IMS_TRACE_D("NotifyInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_PRESENCE_NOTIFY_IND);
    objParcel.writeInt32(key);
    objParcel.writeInt32(count);
    for (IMS_UINT32 n = 0; n < count; n++)
    {
        AString pidfXml = pidfXmls.GetAt(n);
        objParcel.writeString16(android::String16(pidfXml.GetStr()));
    }
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::SubscribeErrorInd(IMS_UINT32 key, IMS_UINT32 commandError)
{
    IMS_TRACE_D("SubscribeErrorInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_SUBSCRIBE_CMD_ERROR_IND);
    objParcel.writeInt32(key);
    objParcel.writeInt32(commandError);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::SubscribeResourceTerminatedInd(
        IMS_UINT32 key, IMS_UINT32 count, ImsList<IUceTerminatedReason*> terminateContacts)
{
    IMS_TRACE_D("SubscribeResourceTerminatedInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND);
    objParcel.writeInt32(key);
    objParcel.writeInt32(count);
    for (IMS_UINT32 i = 0; i < count; i++)
    {
        IUceTerminatedReason* pTempContact = (terminateContacts).GetAt(i);
        if (pTempContact != null)
        {
            objParcel.writeString16(android::String16(pTempContact->m_strContact.GetStr()));
            objParcel.writeString16(android::String16(pTempContact->m_strReason.GetStr()));
        }
        else
        {
            AString temp = AString::ConstEmpty();
            objParcel.writeString16(android::String16(temp.GetStr()));
            objParcel.writeString16(android::String16(temp.GetStr()));
        }
    }
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::SubscribeTerminatedInd(
        IMS_UINT32 key, AString reason, IMS_UINT32 retryAfterMillsecond)
{
    IMS_TRACE_D("SubscribeTerminatedInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_SUBSCRIBE_TERMINATED_IND);
    objParcel.writeInt32(key);
    objParcel.writeString16(android::String16(reason.GetStr()));
    objParcel.writeInt32(retryAfterMillsecond);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::OptionsResponseInd(
        IMS_UINT32 key, IMS_UINT32 responseCode, AString reason, IMS_UINT32 theirCaps)
{
    IMS_TRACE_D("OptionsResponseInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_OPTIONS_RESPONSE_IND);
    objParcel.writeInt32(key);
    objParcel.writeInt32(responseCode);
    objParcel.writeString16(android::String16(reason.GetStr()));
    objParcel.writeInt64(theirCaps);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::OptionsErrorInd(IMS_UINT32 key, IMS_UINT32 commandError)
{
    IMS_TRACE_D("OptionsErrorInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_OPTIONS_CMD_ERROR_IND);
    objParcel.writeInt32(key);
    objParcel.writeInt32(commandError);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::OptionsReceivedInd(
        IMS_UINT32 key, AString remote, IMS_UINT32 remoteCaps)
{
    IMS_TRACE_D("OptionsReceivedInd", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_OPTIONS_RECEIVED_IND);
    objParcel.writeInt32(key);
    objParcel.writeString16(android::String16(remote.GetStr()));
    objParcel.writeInt64(remoteCaps);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::NotifyImsRegiRefreshed(IN IMS_SINT32 registeredNetwork)
{
    IMS_TRACE_D("NotifyImsRegiRefreshed", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_IMS_AGENT_REFRESHED_IND);
    objParcel.writeInt32(registeredNetwork);
    return SendData2Java(objParcel);
}

PUBLIC IMS_BOOL JniUceServiceThread::NotifyNetworkChanged(IN IMS_SINT32 changedNetwork)
{
    IMS_TRACE_D("NotifyNetworkChanged", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IUUceService::UCE_NETWORK_CHANGED);
    objParcel.writeInt32(changedNetwork);
    return SendData2Java(objParcel);
}