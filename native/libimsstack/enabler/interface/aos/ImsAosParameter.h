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
#ifndef IMS_AOS_PARAMETER_H_
#define IMS_AOS_PARAMETER_H_

#include "AString.h"

/**
 * @brief This class provides the type for IImsAos#Control()
 *
 * The API using this class is below. \n
 *     - IImsAos#Control(IN IMS_UINT32 nType)
 */
class ImsAosControl
{
public:
    enum
    {
        /// The initial registration is attempted in an offline state.
        REGISTER_START = 0,
        /// The initial registration with WLAN is attempted in an offline state.
        REGISTER_START_WITH_WLAN,
        /// The re-registration is attempted in the registered state.
        REGISTER_REFRESH,
        /// The de-registration is attempted.
        REGISTER_STOP,
        /// The de-registration is attempted and registration is blocked with some period.
        REGISTER_STOP_BY_ROAMING,
        /// The initial registration is attempted after the registration is destroyed.
        REGISTER_REINITIATE,
        /// The registration is destroyed and the initial registration is tried
        /// based on CS call state and RAT.
        REGISTER_REINITIATE_BY_CSFB,

        /// The emergency registration is skipped and next PCSCF is used.
        E_REGISTER_FAKE_WITH_NEXT_PCSCF,

        /// The initial registration is attempted with next PCSCF.
        PCSCF_NEXT,
        /// The current PCSCF is not used as unavailable. The initial registration is attempted
        /// with the next PCSCF. PCSCF discovery is tried if PCSCF is unavailable.
        PCSCF_NEXT_WITH_DISCOVERY,

        /// The initial registration is attempted without IPSEC.
        IPSEC_DISABLED,

        /// MTS service is temporarily blocked by server error with outage reason
        MTS_BLOCKED_BY_SERVER_OUTAGE,

        /// The number of retry failures increases.
        RETRY_COUNT_INCREASE,

        /// Update of registration is attempted for SipDelegate include initial registration.
        /// This is the replace of ImsRegistrationImplBase.updateSipDelegateRegistration().
        UPDATE_SIP_DELEGATE_REGISTRATION,

        /// De-registration is attempted for SipDelegate.
        /// This is the replace of ImsRegistrationImplBase.triggerSipDelegateDeregistration().
        TRIGGER_SIP_DELEGATE_DEREGISTRATION,

        /// Tear down the full IMS registration and re-register again for SipDelegate.
        /// This is the replace of ImsRegistrationImplBase.triggerFullNetworkRegistration().
        TRIGGER_FULL_NETWORK_REGISTRATION
    };
};

/**
 * @brief This class provides the feature type for IImsAos APIs
 *
 * The API using this class is below. \n
 *      - IImsAos#IsFeatureConnected() \n
 *      - IImsAos#GetFeatures() \n
 *      - IImsAos#UpdateFeature() \n
 *      - IImsAosInfo#GetImsFeatures()
 */
class ImsAosFeature
{
public:
    enum
    {
        NONE = 0,

        /// MTC
        MMTEL = (0x00000001), // "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\""
        VIDEO = (0x00000002), // "video"
        TEXT = (0x00000004), // "text"
        USSI = (0x00000008), // "+g.3gpp.nw-init-ussi"
        VERSTAT = (0x00000010), // "+g.3gpp.verstat"

        /// MTS
        SMSIP = (0x00000020), // "+g.3gpp.smsip"

        /// SIP Controller
        STANDALONE_MSG = (0x00000040),
        CHAT_IM = (0x00000080),
        CHAT_SESSION = (0x00000100),
        FILE_TRANSFER = (0x00000200),
        FILE_TRANSFER_VIA_SMS = (0x00000400),
        CALL_COMPOSER_ENRICHED_CALLING = (0x00000800),
        CALL_COMPOSER_VIA_TELEPHONY = (0x00001000),
        POST_CALL = (0x00002000),
        SHARED_MAP = (0x00004000),
        SHARED_SKETCH = (0x00008000),
        GEO_PUSH = (0x00010000),
        GEO_PUSH_VIA_SMS = (0x00020000),
        CHATBOT_COMMUNICATION_USING_SESSION = (0x00040000),
        CHATBOT_COMMUNICATION_USING_STANDALONE_MSG = (0x00080000),
        CHATBOT_VERSION_SUPPORTED = (0x00100000),
        CHATBOT_VERSION_V2_SUPPORTED = (0x00200000),
        CHATBOT_ROLE = (0x00400000),
        PRESENCE = (0x00800000)
    };
};


/**
 * @brief This class provides the feature-tag data for IImsAos#UpdateFeature()
 *
 * The API using this class is below. \n
 *     - IImsAos#UpdateFeature(IN IMSList<ImsAosFeatureTag*>& objFeatureTag)
 */
class ImsAosFeatureTag
{
public:
    ImsAosFeatureTag(IN AString strName, IN AString strValue)
        : m_strName(strName)
        , m_strValue(strValue) {}
    virtual ~ImsAosFeatureTag() {}

    inline AString& GetName() {return m_strName;}
    inline AString& GetValue() {return m_strValue;}

private:
    AString m_strName;
    AString m_strValue;
};

/**
 * @brief This class provides the service type related to service ID for IMS registration
 */
class ImsAosService
{
public:
    enum
    {
        NONE = 0,
        MTC = (0x0001),
        MTS = (0x0002),
        SIP_CONTROLLER = (0x0004),
        UCE = (0x0008),
        EMERGENCY_MTC = (0x0010),
        EMERGENCY_MTS = (0x0020)
    };
};

#endif // IMS_AOS_PARAMETER_H_
