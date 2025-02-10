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
#include "AString.h"
#include "CallReasonInfo.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "JniCallInfo.h"
#include "JniMtcUtils.h"
#include "MtcDef.h"
#include "conferencecall/ConferenceDef.h"
#include <binder/Parcel.h>

using namespace android;

PUBLIC GLOBAL void JniMtcUtils::ConvertString(IN const String16& strSource, OUT AString& strDest)
{
    strDest = String8(strSource).c_str();
}

PUBLIC GLOBAL CallType JniMtcUtils::ReadCallType(IN const android::Parcel& objParcel)
{
    return static_cast<CallType>(objParcel.readInt32());
}

PUBLIC GLOBAL ServiceType JniMtcUtils::ReadServiceType(IN const android::Parcel& objParcel)
{
    return static_cast<ServiceType>(objParcel.readInt32());
}

PUBLIC GLOBAL EmergencyType JniMtcUtils::ReadEmergencyType(IN const android::Parcel& objParcel)
{
    return static_cast<EmergencyType>(objParcel.readInt32());
}

PUBLIC GLOBAL JniCallInfo JniMtcUtils::ReadCallInfo(IN const Parcel& objParcel)
{
    JniCallInfo objCallInfo;

    objCallInfo.eCallType = ReadCallType(objParcel);
    objCallInfo.eEmergencyType = ReadEmergencyType(objParcel);
    objCallInfo.bOffline = (objParcel.readInt32()) ? IMS_TRUE : IMS_FALSE;
    objCallInfo.bUssi = (objParcel.readInt32()) ? IMS_TRUE : IMS_FALSE;

    return objCallInfo;
}

PUBLIC GLOBAL MediaInfo& JniMtcUtils::ReadMediaInfo(
        IN const Parcel& objParcel, IN_OUT MediaInfo& objMediaInfo)
{
    objMediaInfo.eAudioQuality = objParcel.readInt32();
    objMediaInfo.eVideoQuality = objParcel.readInt32();
    objMediaInfo.eAudioDirection = objParcel.readInt32();
    objMediaInfo.eVideoDirection = objParcel.readInt32();
    objMediaInfo.eTextDirection = objParcel.readInt32();
    objMediaInfo.eGttMode = objParcel.readInt32();

    return objMediaInfo;
}

PUBLIC GLOBAL ImsMap<SuppType, SuppService*> JniMtcUtils::ReadSupplementaryService(
        IN const Parcel& objParcel)
{
    ImsMap<SuppType, SuppService*> objSupp;

    IMS_UINT32 nSuppService = objParcel.readInt32();
    for (IMS_UINT32 index = 0; index < nSuppService; index++)
    {
        SuppService* pSuppService = new SuppService();

        objSupp.Add(static_cast<SuppType>(objParcel.readInt32()), pSuppService);
        ConvertString(objParcel.readString16(), pSuppService->strValue);
        pSuppService->nValue = objParcel.readInt32();
        pSuppService->bValue = (objParcel.readInt32()) ? IMS_TRUE : IMS_FALSE;
    }

    return objSupp;
}

PUBLIC GLOBAL ImsList<ConfUser*> JniMtcUtils::ReadConferenceParticipants(IN const Parcel& objParcel)
{
    ImsList<ConfUser*> objUsers;
    IMS_UINT32 nUsersSize = objParcel.readInt32();
    for (IMS_UINT32 index = 0; index < nUsersSize; index++)
    {
        ConfUser* pUser = new ConfUser();

        pUser->nConnectionId = objParcel.readInt64();  // TODO: how to ... call key!!!!!???
        ConvertString(objParcel.readString16(), pUser->strTarget);
        ConvertString(objParcel.readString16(), pUser->strUserEntity);
        ConvertString(objParcel.readString16(), pUser->strEpEntity);
        ConvertString(objParcel.readString16(), pUser->strDisplayName);
        pUser->eStatus = objParcel.readInt32();
        pUser->eStatusCode = objParcel.readInt32();
        pUser->eCcType = objParcel.readInt32();
        pUser->bAnonymize = (objParcel.readInt32()) ? IMS_TRUE : IMS_FALSE;

        objUsers.Append(pUser);
    }

    return objUsers;
}

PUBLIC GLOBAL void JniMtcUtils::WriteCallInfoToParcel(
        IN const JniCallInfo& objCallInfo, IN_OUT Parcel& objParcel)
{
    objParcel.writeInt32(static_cast<IMS_SINT32>(objCallInfo.eServiceType));
    objParcel.writeInt32(static_cast<IMS_SINT32>(objCallInfo.eCallType));
    objParcel.writeInt32(static_cast<IMS_SINT32>(objCallInfo.eEmergencyType));
    IMS_SINT32 bOffline = (objCallInfo.bOffline) ? 1 : 0;
    objParcel.writeInt32(bOffline);
    IMS_SINT32 bUssi = (objCallInfo.bUssi) ? 1 : 0;
    objParcel.writeInt32(bUssi);
    IMS_SINT32 bConf = (objCallInfo.bConference) ? 1 : 0;  // nConf? nIsConf?
    objParcel.writeInt32(bConf);
    IMS_SINT32 bEnabledConf = (objCallInfo.bConferenceEnabled) ? 1 : 0;
    objParcel.writeInt32(bEnabledConf);
    IMS_SINT32 bConfSub = (objCallInfo.bConferenceSubscriptionRequired) ? 1 : 0;
    objParcel.writeInt32(bConfSub);
    IMS_SINT32 bRttCapable = (objCallInfo.bRttCapable) ? 1 : 0;
    objParcel.writeInt32(bRttCapable);
    IMS_SINT32 bVideoCapable = (objCallInfo.bVideoCapable) ? 1 : 0;
    objParcel.writeInt32(bVideoCapable);
}

PUBLIC GLOBAL void JniMtcUtils::WriteMediaInfoToParcel(
        IN const MediaInfo& objMediaInfo, IN_OUT Parcel& objParcel)
{
    objParcel.writeInt32(objMediaInfo.eAudioQuality);
    objParcel.writeInt32(objMediaInfo.eVideoQuality);
    objParcel.writeInt32(objMediaInfo.eAudioDirection);
    objParcel.writeInt32(objMediaInfo.eVideoDirection);
    objParcel.writeInt32(objMediaInfo.eTextDirection);
    objParcel.writeInt32(objMediaInfo.eGttMode);
}

PUBLIC GLOBAL void JniMtcUtils::WriteSuppServicesToParcel(
        IN const ImsMap<SuppType, SuppService*>& objSuppServices, IN_OUT Parcel& objParcel)
{
    IMS_UINT32 nSuppService = objSuppServices.GetSize();

    objParcel.writeInt32(nSuppService);
    for (IMS_UINT32 index = 0; index < nSuppService; index++)
    {
        SuppService* pService = objSuppServices.GetValueAt(index);

        objParcel.writeInt32(static_cast<IMS_SINT32>(objSuppServices.GetKeyAt(index)));
        objParcel.writeString16(String16(pService->strValue.GetStr()));
        objParcel.writeInt32(pService->nValue);
        objParcel.writeInt32(pService->bValue);
    }
}

PUBLIC GLOBAL void JniMtcUtils::WriteConfUsersToParcel(
        IN const ImsList<ConfUser*>& objUsers, IN_OUT android::Parcel& objParcel)
{
    IMS_UINT32 nUsersSize = objUsers.GetSize();

    objParcel.writeInt32(nUsersSize);
    for (IMS_UINT32 i = 0; i < nUsersSize; i++)
    {
        ConfUser* pUser = objUsers.GetAt(i);

        objParcel.writeInt64(static_cast<IMS_SINTP>(pUser->nConnectionId));  // TODO: Int32.
        objParcel.writeString16(android::String16(pUser->strTarget.GetStr()));
        objParcel.writeString16(android::String16(pUser->strUserEntity.GetStr()));
        objParcel.writeString16(android::String16(pUser->strEpEntity.GetStr()));
        objParcel.writeString16(android::String16(pUser->strDisplayName.GetStr()));
        objParcel.writeInt32(pUser->eStatus);
        objParcel.writeInt32(pUser->eStatusCode);
        objParcel.writeInt32(pUser->eCcType);
        objParcel.writeInt32(pUser->bAnonymize);
    }
}

PUBLIC GLOBAL void JniMtcUtils::WriteExternalCallsToParcel(
        IN ImsList<const JniExternalCall*>& objJniExternalCalls, IN_OUT Parcel& objParcel)
{
    IMS_UINT32 nCallsSize = objJniExternalCalls.GetSize();
    objParcel.writeInt32(nCallsSize);

    for (IMS_UINT32 i = 0; i < nCallsSize; i++)
    {
        const JniExternalCall* pJniExternalCall = objJniExternalCalls.GetAt(i);

        objParcel.writeString16(android::String16(pJniExternalCall->m_strCallId.GetStr()));
        objParcel.writeString16(android::String16(pJniExternalCall->m_strAddress.GetStr()));
        objParcel.writeString16(android::String16(pJniExternalCall->m_strLocalAddress.GetStr()));
        objParcel.writeInt32((pJniExternalCall->m_bIsPullable) ? 1 : 0);
        objParcel.writeInt32(pJniExternalCall->m_nCallState);
        objParcel.writeInt32(pJniExternalCall->m_nCallType);
        objParcel.writeInt32((pJniExternalCall->m_bIsHeld) ? 1 : 0);
    }
}

PUBLIC GLOBAL void JniMtcUtils::WriteCallReasonInfoToParcel(
        IN const CallReasonInfo& objReason, IN_OUT Parcel& objParcel)
{
    objParcel.writeInt32(objReason.nCode);
    objParcel.writeInt32(objReason.nExtraCode);
    objParcel.writeString16(android::String16(objReason.strExtraMessage.GetStr()));
}
