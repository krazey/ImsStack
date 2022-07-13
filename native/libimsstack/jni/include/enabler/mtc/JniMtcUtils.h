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

#ifndef JNI_MTC_UTILS_H_
#define JNI_MTC_UTILS_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "JniCallInfo.h"
#include "MtcDef.h"
#include <binder/Parcel.h>
#include "ImsMap.h"

class JniMtcUtils final
{
public:
    static void ConvertString(IN const android::String16& strSource, OUT AString& strDest);
    static CallType ReadCallType(IN const android::Parcel& objParcel);
    static ServiceType ReadServiceType(IN const android::Parcel& objParcel);
    static JniCallInfo ReadCallInfo(IN const android::Parcel& objParcel);
    static MediaInfo* ReadMediaInfo(IN const android::Parcel& objParcel);
    static IMSMap<SuppType, SuppService*> ReadSupplementaryService(
            IN const android::Parcel& objParcel);
    static IMSList<ConfUser*> ReadConferenceParticipants(IN const android::Parcel& objParcel);

    static void WriteCallInfoToParcel(
            IN const JniCallInfo& objCallInfo, IN_OUT android::Parcel& objParcel);
    static void WriteMediaInfoToParcel(IN MediaInfo* pMediaInfo, IN_OUT android::Parcel& objParcel);
    static void WriteSuppServicesToParcel(IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN_OUT android::Parcel& objParcel);
    static void WriteConfUsersToParcel(
            IN const IMSList<ConfUser*>& objUsers, IN_OUT android::Parcel& objParcel);
    // static void WriteDialogInfoToParcel(IN DialogInfo* pInfo, IN_OUT android::Parcel& objParcel);
    static void WriteCallReasonInfoToParcel(
            IN const CallReasonInfo& objReason, IN_OUT android::Parcel& objParcel);
};

#endif
