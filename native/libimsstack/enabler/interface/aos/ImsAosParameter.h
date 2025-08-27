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
 * Warning: The order of these enum values must not be changed.
 *          Some modules rely on the hardcoded integer values of these enums.
 *
 * The API using this class is below.
 *     - IImsAos#Control(IN IMS_UINT32 nType)
 */
class ImsAosControl
{
public:
    enum
    {
        /// An initial registration is attempted in an offline state.
        REGISTER_START = 0,
        /// An initial registration with WLAN is attempted in an offline state.
        REGISTER_START_WITH_WLAN,
        /// A re-registration is attempted in the registered state.
        REGISTER_REFRESH,
        /// A de-registration is attempted.
        REGISTER_STOP,
        /// A de-registration is attempted, and registration is blocked for a specific period.
        REGISTER_STOP_BY_ROAMING,
        /**
         * An initial registration is attempted after the previous registration is destroyed.
         *
         * [NOTE] If there are any IMS calls, the registration will be performed after they are
         *        terminated.
         */
        REGISTER_REINITIATE,
        /**
         * The registration is destroyed, and an initial registration is attempted based on the
         * CS call state and RAT.
         *
         * [NOTE] If there are any IMS calls, the registration will be performed after they are
         *        terminated.
         */
        REGISTER_REINITIATE_BY_CSFB,
        /// The emergency registration is skipped, and the next PCSCF is used.
        E_REGISTER_FAKE_WITH_NEXT_PCSCF,
        /**
         * An initial registration is attempted with the next PCSCF.
         *
         * [NOTE] If there are any IMS calls, the registration will be performed after they are
         *        terminated.
         */
        PCSCF_NEXT,
        /// The current PCSCF is not used because it is unavailable. An initial registration is
        /// attempted with the next PCSCF. PCSCF discovery is tried if no PCSCF is available.
        PCSCF_NEXT_WITH_DISCOVERY,
        /// An initial registration is attempted without IPSEC.
        IPSEC_DISABLED,
        /**
         * If the retry count is increased and reaches the maximum count, an initial registration
         * is performed with an available P-CSCF if one exists. If no available P-CSCF is found,
         * a RAT block operation is performed.
         *
         * [NOTE] If there are any IMS calls, the registration will be performed after they are
         *        terminated.
         */
        RETRY_COUNT_INCREASE,
        /**
         * When the retry count increases and reaches the max count, it behaves like
         * RETRY_COUNT_INCREASE. If it does not reach the max count, it performs an initial
         * registration.
         *
         * [NOTE] If there are any IMS calls, the registration will be performed after they are
         *        terminated.
         */
        RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION,
        /// An update of registration is attempted for SipDelegate, including the initial
        /// registration. This replaces ImsRegistrationImplBase.updateSipDelegateRegistration().
        UPDATE_SIP_DELEGATE_REGISTRATION,
        /// A de-registration is attempted for SipDelegate.
        /// This replaces ImsRegistrationImplBase.triggerSipDelegateDeregistration().
        TRIGGER_SIP_DELEGATE_DEREGISTRATION,
        /// The full IMS registration is torn down, and a re-registration is performed for
        /// SipDelegate. This replaces ImsRegistrationImplBase.triggerFullNetworkRegistration().
        TRIGGER_FULL_NETWORK_REGISTRATION,
        /// The current PLMN is blocked with a timeout.
        PLMN_BLOCK_WITH_TIMEOUT,
        /// The emergency registration is skipped, and the current PCSCF is used.
        E_REGISTER_FAKE_WITH_SAME_PCSCF
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
        MMTEL = (0x00000001),  // "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\""
        VIDEO = (0x00000002),  // "video"
        TEXT = (0x00000004),   // "text"
        NW_INIT_USSI = (0x00000008),  // "+g.3gpp.nw-init-ussi"
        VERSTAT = (0x00000010),       // "+g.3gpp.verstat"

        /// MTS
        SMSIP = (0x00000020),  // "+g.3gpp.smsip"

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
        PRESENCE = (0x00800000),

        /// ALL
        ALL = (0xFFFFFFFF)
    };
};

/**
 * @brief This class provides the feature-tag data for IImsAos#UpdateFeature()
 *
 * The API using this class is below. \n
 *     - IImsAos#UpdateFeature(IN ImsList<ImsAosFeatureTag*>& objFeatureTag)
 */
class ImsAosFeatureTag
{
public:
    ImsAosFeatureTag(IN const AString& strName, IN const AString& strValue) :
            m_strName(strName),
            m_strValue(strValue)
    {
    }
    virtual ~ImsAosFeatureTag() {}

    inline AString& GetName() { return m_strName; }
    inline AString& GetValue() { return m_strValue; }

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

#endif  // IMS_AOS_PARAMETER_H_
