#ifndef JNI_MTC_UTILS_H_
#define JNI_MTC_UTILS_H_

#include "AString.h"
#include "IMSTypeDef.h"
#include "CallInfo.h"
#include "MtcDef.h"
#include <binder/Parcel.h>
#include "IMSMap.h"

class JniMtcUtils final
{
public:
    static void ConvertString(IN const android::String16& strSource, OUT AString& strDest);
    static CallType ReadCallType(IN const android::Parcel& objParcel);
    static CallInfo ReadCallInfo(IN const android::Parcel& objParcel);
    static MediaInfo* ReadMediaInfo(IN const android::Parcel& objParcel);
    static IMSMap<SuppType, SuppService*> ReadSupplementaryService(
            IN const android::Parcel& objParcel);
    static IMSList<ConfUser*> ReadConferenceParticipants(IN const android::Parcel& objParcel);

    static void WriteCallInfoToParcel(IN CallInfo* pCallInfo, IN_OUT android::Parcel& objParcel);
    static void WriteMediaInfoToParcel(IN MediaInfo* pMediaInfo, IN_OUT android::Parcel& objParcel);
    static void WriteSuppServicesToParcel(IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN_OUT android::Parcel& objParcel);
    static void WriteConfUsersToParcel(
            IN const IMSList<ConfUser*>& objUsers, IN_OUT android::Parcel& objParcel);
    static void WriteDialogInfoToParcel(IN DialogInfo* pInfo, IN_OUT android::Parcel& objParcel);
    static void WriteFailReasonToParcel(
            IN const FailReason& objFailReason, IN_OUT android::Parcel& objParcel);
};

#endif
