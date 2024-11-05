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

#ifndef JNI_CALL_INFO_H_
#define JNI_CALL_INFO_H_

#include "IMtcService.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

struct JniCallInfo
{
public:
    JniCallInfo() :
            eServiceType(ServiceType::NORMAL),
            eCallType(CallType::VOIP),
            eEmergencyType(EmergencyType::NONE),
            bOffline(IMS_FALSE),
            bUssi(IMS_FALSE),
            bConference(IMS_FALSE),
            bConferenceEnabled(IMS_FALSE),
            bConferenceSubscriptionRequired(IMS_FALSE),
            bRttCapable(IMS_FALSE),
            bVideoCapable(IMS_FALSE)
    {
    }

    JniCallInfo(IN const JniCallInfo& objRhs) :
            eServiceType(objRhs.eServiceType),
            eCallType(objRhs.eCallType),
            eEmergencyType(objRhs.eEmergencyType),
            bOffline(objRhs.bOffline),
            bUssi(objRhs.bUssi),
            bConference(objRhs.bConference),
            bConferenceEnabled(objRhs.bConferenceEnabled),
            bConferenceSubscriptionRequired(objRhs.bConferenceSubscriptionRequired),
            bRttCapable(objRhs.bRttCapable),
            bVideoCapable(objRhs.bVideoCapable)
    {
    }

    JniCallInfo& operator=(IN const JniCallInfo& objRhs)
    {
        if (this != &objRhs)
        {
            eServiceType = objRhs.eServiceType;
            eCallType = objRhs.eCallType;
            eEmergencyType = objRhs.eEmergencyType;
            bOffline = objRhs.bOffline;
            bUssi = objRhs.bUssi;
            bConference = objRhs.bConference;
            bConferenceEnabled = objRhs.bConferenceEnabled;
            bConferenceSubscriptionRequired = objRhs.bConferenceSubscriptionRequired;
            bRttCapable = objRhs.bRttCapable;
            bVideoCapable = objRhs.bVideoCapable;
        }

        return *this;
    }

    IMS_BOOL operator==(const JniCallInfo& objRhs) const
    {
        if (this == &objRhs)
        {
            return IMS_TRUE;
        }

        return eServiceType == objRhs.eServiceType && eCallType == objRhs.eCallType &&
                eEmergencyType == objRhs.eEmergencyType && bOffline == objRhs.bOffline &&
                bUssi == objRhs.bUssi && bConference == objRhs.bConference &&
                bConferenceEnabled == objRhs.bConferenceEnabled &&
                bConferenceSubscriptionRequired == objRhs.bConferenceSubscriptionRequired &&
                bRttCapable == objRhs.bRttCapable && bVideoCapable == objRhs.bVideoCapable;
    }

public:
    ServiceType eServiceType;
    CallType eCallType;
    EmergencyType eEmergencyType;

    IMS_BOOL bOffline;
    IMS_BOOL bUssi;
    IMS_BOOL bConference;
    IMS_BOOL bConferenceEnabled;
    IMS_BOOL bConferenceSubscriptionRequired;
    IMS_BOOL bRttCapable;
    IMS_BOOL bVideoCapable;
};

#endif
