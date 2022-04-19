#define IMS_STL_USE
#include "AString.h"
#include "FailReason.h"
#include "IMSTypeDef.h"
#include "JniMtcUtils.h"
#include "MtcDef.h"
#include <binder/Parcel.h>

using namespace android;

PUBLIC GLOBAL
void JniMtcUtils::ConvertString(IN const String16& strSource, OUT AString& strDest)
{
    String8 str8(strSource);
    strDest = str8.string();
}

PUBLIC GLOBAL
CallType JniMtcUtils::ReadCallType(IN const android::Parcel& objParcel)
{
    return static_cast<CallType>(objParcel.readInt32());
}

PUBLIC GLOBAL
CallInfo JniMtcUtils::ReadCallInfo(IN const Parcel& objParcel)
{
    CallInfo objCallInfo;

    objCallInfo.eCallType = ReadCallType(objParcel);
    objCallInfo.bWifi = (objParcel.readInt32()) ? IMS_TRUE : IMS_FALSE;
    objCallInfo.bEmergency = (objParcel.readInt32()) ? IMS_TRUE : IMS_FALSE;
    objCallInfo.bOffline = (objParcel.readInt32()) ? IMS_TRUE : IMS_FALSE;
    objCallInfo.bUssi = (objParcel.readInt32()) ? IMS_TRUE : IMS_FALSE;

    return objCallInfo;
}

PUBLIC GLOBAL
MediaInfo* JniMtcUtils::ReadMediaInfo(IN const Parcel& objParcel)
{
    MediaInfo* pMediaInfo = new MediaInfo();

    pMediaInfo->eAQuality = objParcel.readInt32();
    pMediaInfo->eVQuality = objParcel.readInt32();
    pMediaInfo->eADir = objParcel.readInt32();
    pMediaInfo->eVDir = objParcel.readInt32();
    pMediaInfo->eTDir = objParcel.readInt32();
    pMediaInfo->eGTTMode = objParcel.readInt32();

    return pMediaInfo;
}

PUBLIC GLOBAL
IMSMap<IMS_UINT32, SuppService*> JniMtcUtils::ReadSupplementaryService(IN const Parcel& objParcel)
{
    IMSMap<IMS_UINT32, SuppService*> objSupp;

    IMS_UINT32 nSuppService = objParcel.readInt32();
    for(IMS_UINT32 index = 0; index < nSuppService; index++)
    {
        SuppService* pSuppService = new SuppService();

        pSuppService->nType = objParcel.readInt32();
        ConvertString(objParcel.readString16(), pSuppService->aStrValue);
        pSuppService->nValue= objParcel.readInt32();
        pSuppService->bValue= (objParcel.readInt32()) ? IMS_TRUE : IMS_FALSE;

        objSupp.Add(pSuppService->nType, pSuppService);
    }

    return objSupp;
}

PUBLIC GLOBAL
IMSList<ConfUser*> JniMtcUtils::ReadConferenceParticipants(IN const Parcel& objParcel)
{
    IMSList<ConfUser*> objUsers;
    IMS_UINT32 nUsersSize = objParcel.readInt32();
    for (IMS_UINT32 index = 0; index < nUsersSize; index++)
    {
        ConfUser* pUser = new ConfUser();

        pUser->nConnectionId = objParcel.readInt64(); // TODO: how to ... call key!!!!!???
        ConvertString(objParcel.readString16(), pUser->aStrTarget);
        ConvertString(objParcel.readString16(), pUser->aStrUserEntity);
        ConvertString(objParcel.readString16(), pUser->aStrEPEntity);
        ConvertString(objParcel.readString16(), pUser->aStrDisplayName);
        pUser->eStatus = objParcel.readInt32();
        pUser->eStatusCode = objParcel.readInt32();
        pUser->eCCType = objParcel.readInt32();
        pUser->bAnonymize = (objParcel.readInt32()) ? IMS_TRUE : IMS_FALSE;

        objUsers.Append(pUser);
    }

    return objUsers;
}

PUBLIC GLOBAL
void JniMtcUtils::WriteCallInfoToParcel(IN CallInfo* pCallInfo, IN_OUT Parcel& objParcel)
{
    objParcel.writeInt32(static_cast<int32_t>(pCallInfo->eServiceType));
    objParcel.writeInt32(static_cast<int32_t>(pCallInfo->eCallType));
    IMS_SINT32 bWifi = (pCallInfo->bWifi) ? 1 : 0;
    objParcel.writeInt32(bWifi);
    IMS_SINT32 bEmergency = (pCallInfo->bEmergency) ? 1 : 0;
    objParcel.writeInt32(bEmergency);
    IMS_SINT32 bOffline = (pCallInfo->bOffline) ? 1 : 0;
    objParcel.writeInt32(bOffline);
    IMS_SINT32 bUssi = (pCallInfo->bUssi) ? 1 : 0;
    objParcel.writeInt32(bUssi);
    IMS_SINT32 bConf = (pCallInfo->bConference) ? 1 : 0; // nConf? nIsConf?
    objParcel.writeInt32(bConf);
    IMS_SINT32 bEnabledConf = (pCallInfo->bConferenceEnabled) ? 1 : 0;
    objParcel.writeInt32(bEnabledConf);
    IMS_SINT32 bConfSub = (pCallInfo->bConferenceSubscriptionRequired) ? 1 : 0;
    objParcel.writeInt32(bConfSub);
    IMS_SINT32 bRttCapable = (pCallInfo->bRttCapable) ? 1 : 0;
    objParcel.writeInt32(bRttCapable);
    IMS_SINT32 bVideoCapable = (pCallInfo->bVideoCapable) ? 1 : 0;
    objParcel.writeInt32(bVideoCapable);
}

PUBLIC GLOBAL
void JniMtcUtils::WriteMediaInfoToParcel(IN MediaInfo* pMediaInfo,
        IN_OUT Parcel& objParcel)
{
    objParcel.writeInt32(pMediaInfo->eAQuality);
    objParcel.writeInt32(pMediaInfo->eVQuality);
    objParcel.writeInt32(pMediaInfo->eADir);
    objParcel.writeInt32(pMediaInfo->eVDir);
    objParcel.writeInt32(pMediaInfo->eTDir);
    objParcel.writeInt32(pMediaInfo->eGTTMode);
}

PUBLIC GLOBAL
void JniMtcUtils::WriteSuppServicesToParcel(
        IN const IMSMap<IMS_UINT32, SuppService*>& objSuppServices, IN_OUT Parcel& objParcel)
{
    IMS_UINT32 nSuppService = objSuppServices.GetSize();

    objParcel.writeInt32(nSuppService);
    for( IMS_UINT32 index = 0; index < nSuppService; index++)
    {
        SuppService* pService = objSuppServices.GetValueAt(index);

        objParcel.writeInt32(pService->nType);
        objParcel.writeString16(String16(pService->aStrValue.GetStr()));
        objParcel.writeInt32(pService->nValue);
        objParcel.writeInt32(pService->bValue);
    }
}

PUBLIC GLOBAL
void JniMtcUtils::WriteConfUsersToParcel(IN const IMSList<ConfUser*>& objUsers,
        IN_OUT android::Parcel& objParcel)
{
    IMS_UINT32 nUsersSize = objUsers.GetSize();

    objParcel.writeInt32(nUsersSize);
    for(IMS_UINT32 i = 0; i < nUsersSize; i++)
    {
        ConfUser* pUser = objUsers.GetAt(i);

        objParcel.writeInt64(static_cast<IMS_SINTP>(pUser->nConnectionId)); // TODO: Int32.
        objParcel.writeString16(android::String16(pUser->aStrTarget.GetStr()));
        objParcel.writeString16(android::String16(pUser->aStrUserEntity.GetStr()));
        objParcel.writeString16(android::String16(pUser->aStrEPEntity.GetStr()));
        objParcel.writeString16(android::String16(pUser->aStrDisplayName.GetStr()));
        objParcel.writeInt32(pUser->eStatus);
        objParcel.writeInt32(pUser->eStatusCode);
        objParcel.writeInt32(pUser->eCCType);
        objParcel.writeInt32(pUser->bAnonymize);
    }
}

PUBLIC GLOBAL
void JniMtcUtils::WriteDialogInfoToParcel(IN DialogInfo* pInfo, IN_OUT Parcel& objParcel)
{
    objParcel.writeString16(String16(pInfo->aStrID.GetStr()));

    objParcel.writeInt32(pInfo->eState);
    objParcel.writeInt32(pInfo->eReason);
    objParcel.writeInt32(pInfo->eCode);

    objParcel.writeString16(String16(pInfo->aStrLocalName.GetStr()));
    objParcel.writeString16(String16(pInfo->aStrLocalNumber.GetStr()));

    objParcel.writeString16(String16(pInfo->aStrRemoteName.GetStr()));
    objParcel.writeString16(String16(pInfo->aStrRemoteNumber.GetStr()));

    IMS_SINT32 bInitiator = (pInfo->bInitiator) ? 1 : 0;
    objParcel.writeInt32(bInitiator);
    IMS_SINT32 bIsConf = (pInfo->bConference) ? 1 : 0;
    objParcel.writeInt32(bIsConf);
    IMS_SINT32 bEnablePull = (pInfo->bEnablePull) ? 1 : 0;
    objParcel.writeInt32(bEnablePull);
}

PUBLIC GLOBAL
void JniMtcUtils::WriteFailReasonToParcel(IN const FailReason& objFailReason,
        IN_OUT Parcel& objParcel)
{
    objParcel.writeInt32(objFailReason.nReason);
    objParcel.writeInt32(objFailReason.nExtra);
    objParcel.writeString16(android::String16(objFailReason.strExtra.GetStr()));
}
