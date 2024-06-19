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
#define IMS_STL_USE

#include "ServiceTrace.h"
#include "BaseServiceThread.h"
#include "IIAosService.h"
#include "JniAosServiceThread.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.AOS");

PUBLIC
JniAosServiceThread::JniAosServiceThread() :
        BaseServiceThread()
{
    IMS_TRACE_D("+JniAosServiceThread", 0, 0, 0);
}

PUBLIC VIRTUAL JniAosServiceThread::~JniAosServiceThread()
{
    IMS_TRACE_D("~JniAosServiceThread", 0, 0, 0);
}

PUBLIC
IMS_BOOL JniAosServiceThread::NotifyRegistered(IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType,
        IN IMS_UINT32 nFeatureTagBits, IN const ImsList<AString>& objFeatureTags)
{
    IMS_TRACE_D("NotifyRegistered", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IIAosService::N2J_NOTIFY_REGISTERED);
    objParcel.writeInt32(nRegType);
    objParcel.writeInt32(nNetworkType);
    objParcel.writeInt32(nFeatureTagBits);

    IMS_UINT32 nCount = objFeatureTags.GetSize();
    objParcel.writeInt32(nCount);

    for (IMS_UINT32 i = 0; i < nCount; ++i)
    {
        objParcel.writeString16(String16(objFeatureTags.GetAt(i).GetStr()));
    }

    return SendData2Java(objParcel);
}

PUBLIC
IMS_BOOL JniAosServiceThread::NotifyRegistering(IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType,
        IN IMS_UINT32 nFeatureTagBits, IN const ImsList<AString>& objFeatureTags)
{
    IMS_TRACE_D("NotifyRegistering", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IIAosService::N2J_NOTIFY_REGISTERING);
    objParcel.writeInt32(nRegType);
    objParcel.writeInt32(nNetworkType);
    objParcel.writeInt32(nFeatureTagBits);

    IMS_UINT32 nCount = objFeatureTags.GetSize();
    objParcel.writeInt32(nCount);

    for (IMS_UINT32 i = 0; i < nCount; ++i)
    {
        objParcel.writeString16(String16(objFeatureTags.GetAt(i).GetStr()));
    }

    return SendData2Java(objParcel);
}

PUBLIC
IMS_BOOL JniAosServiceThread::NotifyDeregistered(IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nReason)
{
    IMS_TRACE_D("NotifyDeregistered", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IIAosService::N2J_NOTIFY_DEREGISTERED);
    objParcel.writeInt32(nNetworkType);
    objParcel.writeInt32(nReason);

    return SendData2Java(objParcel);
}

PUBLIC
IMS_BOOL JniAosServiceThread::NotifyTechnologyChangeFailed(
        IN IMS_SINT32 nRegType, IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nCauseCode)
{
    IMS_TRACE_D("NotifyTechnologyChangeFailed", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IIAosService::N2J_NOTIFY_TECHNOLOGY_CHANGE_FAILED);
    objParcel.writeInt32(nRegType);
    objParcel.writeInt32(nNetworkType);
    objParcel.writeInt32(nCauseCode);

    return SendData2Java(objParcel);
}

PUBLIC
IMS_BOOL JniAosServiceThread::NotifyAssociatedUriChanged(IN const ImsList<AString>& objUris)
{
    IMS_TRACE_D("NotifyAssociatedUriChanged", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IIAosService::N2J_NOTIFY_ASSOCIATED_URI_CHANGED);

    IMS_UINT32 nCount = objUris.GetSize();
    objParcel.writeInt32(nCount);

    for (IMS_UINT32 i = 0; i < nCount; ++i)
    {
        objParcel.writeString16(String16(objUris.GetAt(i).GetStr()));
    }

    return SendData2Java(objParcel);
}

PUBLIC
IMS_BOOL JniAosServiceThread::NotifyCapabilitiesUpdateFailed(
        IN IMS_UINT32 nCapabilities, IN IMS_SINT32 nNetworkType, IN IMS_SINT32 nReason)
{
    IMS_TRACE_D("NotifyCapabilitiesUpdateFailed", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IIAosService::N2J_NOTIFY_CAPABILITIES_UPDATE_FAILED);
    objParcel.writeInt32(nCapabilities);
    objParcel.writeInt32(nNetworkType);
    objParcel.writeInt32(nReason);

    return SendData2Java(objParcel);
}

PUBLIC
IMS_BOOL JniAosServiceThread::NotifyRegEventState(
        IN IMS_UINT32 nStatusCode, IN const ImsList<AString>& objImpus)
{
    IMS_TRACE_D("NotifyRegEventState", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IIAosService::N2J_NOTIFY_REG_EVENT_STATE);
    objParcel.writeInt32(nStatusCode);

    IMS_UINT32 nCount = objImpus.GetSize();
    objParcel.writeInt32(nCount);

    for (IMS_UINT32 i = 0; i < nCount; ++i)
    {
        objParcel.writeString16(String16(objImpus.GetAt(i).GetStr()));
    }

    return SendData2Java(objParcel);
}

PUBLIC
IMS_BOOL JniAosServiceThread::NotifyAosIsimState(IN IMS_UINT32 nState)
{
    IMS_TRACE_D("NotifyAosIsimState", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IIAosService::N2J_NOTIFY_AOS_ISIM_STATE);
    objParcel.writeInt32(nState);

    return SendData2Java(objParcel);
}

PUBLIC
IMS_BOOL JniAosServiceThread::RequestPhoneNumberRetry(IN IMS_UINT32 nState)
{
    IMS_TRACE_D("RequestPhoneNumberRetry", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IIAosService::N2J_REQUEST_PHONE_NUMBER_RETRY);
    objParcel.writeInt32(nState);

    return SendData2Java(objParcel);
}

PUBLIC
IMS_BOOL JniAosServiceThread::RequestWifiService(IN IMS_BOOL bIsOn)
{
    IMS_TRACE_D("RequestWifiService", 0, 0, 0);

    Parcel objParcel;
    objParcel.writeInt32(IIAosService::N2J_REQUEST_WIFI_SERVICE);
    objParcel.writeInt32(bIsOn ? 1 : 0);

    return SendData2Java(objParcel);
}
