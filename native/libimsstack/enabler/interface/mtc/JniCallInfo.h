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
#include "INetworkWatcher.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

/**
 * @brief Structure containing call information to be passed to the JNI layer.
 *
 * This structure aggregates various properties of a call, such as service type,
 * call type, capabilities, and network status, to notify the upper layers (Java)
 * about the call's state and attributes.
 */
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
            bVideoCapable(IMS_FALSE),
            bCrossSim(IMS_FALSE),
            eRatType(INetworkWatcher::RADIOTECH_TYPE_INVALID)
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
            bVideoCapable(objRhs.bVideoCapable),
            bCrossSim(objRhs.bCrossSim),
            eRatType(objRhs.eRatType)
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
            bCrossSim = objRhs.bCrossSim;
            eRatType = objRhs.eRatType;
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
                bRttCapable == objRhs.bRttCapable && bVideoCapable == objRhs.bVideoCapable &&
                bCrossSim == objRhs.bCrossSim && eRatType == objRhs.eRatType;
    }

public:
    /// The type of service (e.g., NORMAL, EMERGENCY).
    ServiceType eServiceType;
    /// The type of the call (e.g., VOIP, VT, RTT).
    CallType eCallType;
    /// The type of emergency call (e.g., NONE, EMERGENCY_ROUTING).
    EmergencyType eEmergencyType;

    /// Indicates if the call is an offline call.
    IMS_BOOL bOffline;
    /// Indicates if the session is for USSI (Unstructured Supplementary Service Data over IMS).
    IMS_BOOL bUssi;
    /// Indicates if the call is a conference call.
    IMS_BOOL bConference;
    /// Indicates if conference features are enabled.
    IMS_BOOL bConferenceEnabled;
    /// Indicates if subscription to the conference event package is required.
    IMS_BOOL bConferenceSubscriptionRequired;
    /// Indicates if the session is capable of RTT (Real-time Text).
    IMS_BOOL bRttCapable;
    /// Indicates if the session is capable of Video.
    IMS_BOOL bVideoCapable;
    /// Indicates if the call is connected via Cross-SIM.
    IMS_BOOL bCrossSim;
    /// The Radio Access Technology type (e.g., LTE, NR, IWLAN).
    IMS_SINT32 eRatType;
};

#endif
