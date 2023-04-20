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
#ifndef INTERFACE_AOS_REGISTRATION_H_
#define INTERFACE_AOS_REGISTRATION_H_

#include "ImsTypeDef.h"

class AString;
class IAosRegistrationListener;

/**
 * @brief This class provides the interface for AosRegistration.
 */

class IAosRegistration
{
public:
    virtual ~IAosRegistration(){};

    /**
     * @brief Start the initial registration.
     */
    virtual void Start() = 0;

    /**
     * @brief Try the de-registration.
     */
    virtual void Stop() = 0;

    /**
     * @brief Update the registration.
     *
     * @param bIgnoreRetryTimer Indicate the existing timer is killed \n
     *                          and the re-registration is tryied.
     * @param bExplicitUpdate If the registration is trying, it is performed after completing it.
     */
    virtual void Update(
            IN IMS_BOOL bIgnoreRetryTimer = IMS_FALSE, IN IMS_BOOL bExplicitUpdate = IMS_TRUE) = 0;

    /**
     * @brief Update the registration with the changed feature-tag.
     */
    virtual void Reconfig() = 0;

    /**
     * @brief Destroy the registration.
     */
    virtual void Destroy() = 0;

    /**
     * @brief Set the listener for monitoring the registration.
     *
     * @param piRegListener Indicate the registration listener.
     */
    virtual void SetListener(IN IAosRegistrationListener* piRegListener) = 0;

    /**
     * @brief Request the operation based on command type.
     *
     * @param nCmdType Indicate the command type (@see CMD_XXX enum)
     * @param nReason Indicate the detail reason. (@see REASON_XXX enum)
     */
    virtual void RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason = 0) = 0;

    /**
     * @brief Get the registration mode. (@see MODE_XXX enum)
     */
    virtual IMS_UINT32 GetMode() = 0;

    /**
     * @brief Get the registration information.
     *
     * @param nType Indicate the type or registration information. (@see PROPERTY_XXX enum)
     * @param nValue Indicate the output integer value.
     * @param strValue Indicate the output string value.
     */
    virtual IMS_UINT32 GetProperty(
            IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue) = 0;

    /**
     * @brief Get the registration state. (@see STATE_XXX enum)
     */
    virtual IMS_UINT32 GetState() = 0;

    /**
     * @brief Get the registration type. (@see AosRegistrationType)
     */
    virtual AosRegistrationType GetRegType() = 0;

    /**
     * @brief Check whether IMS is registered or not.
     *
     * @return IMS_BOOL Return whether IMS is registered or not.
     */
    virtual IMS_BOOL IsRegistered() = 0;

    /**
     * @brief Check whether the registration is refreshing or not.
     *
     * @return IMS_BOOL Return whether the registration is refreshing or not.
     */
    virtual IMS_BOOL IsRefreshing() = 0;

    /**
     * @brief Check whether the retry timer is on or not.
     *
     * @return IMS_BOOL Return whether the retry timer is on or not.
     */
    virtual IMS_BOOL IsRetryTimer() = 0;

    /**
     * @brief Check whether the retry is held or not.
     *
     * @return IMS_BOOL Return whether the retry is held or not.
     */
    virtual IMS_BOOL IsRetryHeld() = 0;

    /**
     * @brief Check whether the terminated operation is pending or not.
     *
     * @return IMS_BOOL Return whether the terminated operation is pending or not.
     */
    virtual IMS_BOOL IsTerminated() = 0;

    /**
     * @brief Set the application state whether it is ready or not.
     *
     * @param bReady Indicate whether application state is ready or not.
     */
    virtual void SetAppReady(IN IMS_BOOL bReady) = 0;

    /**
     * @brief Indicate the type and the detailed reason for requesting the operation.
     *
     * @see RequestCmd()
     */
    enum
    {
        /// nCmdType
        CMD_INIT_PCSCF = 0,
        CMD_INIT_AWT,
        CMD_SET_IPSEC,
        CMD_SET_MODE,
        CMD_REFRESH_REGINFO,
        CMD_UPDATE_REG_BINDING,
        CMD_IPCAN_CHANGED,
        CMD_UPDATE_IPCAN,
        CMD_UPDATE_VOPS_STATE,
        CMD_FAKE_MODE,
        CMD_SET_VIDEO_CAPABILITY,
        CMD_CLEAR_RETRY_COUNT,
        CMD_SCSCF_RESTORATION,
        CMD_CLEAR_SERVER_SOCKET_ERROR_COUNT,
        CMD_CLEAR_TRIED_PCSCFS,
        CMD_SCBM_STARTED,
        CMD_SCBM_TERMINATED,
        CMD_SCBM_TERMINATED_ECALL,
        CMD_SCBM_TERMINATED_ESMS,
        CMD_ECALL_INIT,
        CMD_ECALL_DONE,
        CMD_ESMS_INIT,
        CMD_ESMS_DONE,
        CMD_DELAY_CLEAR_SOCKET,
        CMD_UNAVAILABLE_FEATURE_TAG,
        CMD_INCREASE_FAILURE_COUNT_FOR_PDN_REACTIVATED,
        CMD_SET_EPS_5GS_ONLY,

        /// nReason
        REASON_INIT_PCSCF_CLEAR = 30,
        REASON_INIT_PCSCF_UPDATE,
        REASON_INIT_PCSCF_NEXT,
        REASON_INIT_PCSCF_NEXT_AFTER_DEREG,

        REASON_SET_IPSEC_INIT,
        REASON_SET_IPSEC_ENABLE,
        REASON_SET_IPSEC_DISABLE,

        REASON_FAKE_MODE_SAME_PCSCF,
        REASON_FAKE_MODE_NEXT_PCSCF,

        REASON_SET_ENABLE,
        REASON_SET_DISABLE
    };

    /**
     * @brief Indicate the registration mode.
     *
     * @see GetMode()
     */
    enum
    {
        MODE_NORMAL = 0,
        MODE_LIMITED,
        MODE_FAKE
    };

    /**
     * @brief Indicate the registration state.
     *
     * @see GetState()
     */
    enum
    {
        STATE_OFFLINE = 0,
        STATE_REGISTERING,
        STATE_REGSTOP,
        STATE_REGISTERED,
        STATE_REFRESHING,
        STATE_REFRESHSTOP,
        STATE_DEREGISTERING
    };

    /**
     * @brief Indicate the type for getting the registration information.
     *
     * @see GetProperty()
     */
    enum
    {
        PROPERTY_LOCAL_PORT = 0,
        PROPERTY_LOCAL_ADDRESS,
        PROPERTY_PCSCF_PORT,
        PROPERTY_PCSCF_ADDRESS,
        PROPERTY_ASSOCIATED_URI,
        PROPERTY_PATH,
        PROPERTY_SERVICE_ROUTE,
        PROPERTY_LAST_PATH,
        PROPERTY_SUPPORTED,
        PROPERTY_PROTECTED,
        PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION,
        PROPERTY_NETWORK_BINDING_FEATURES,
        PROPERTY_PDN_REACIVATE_WAIT_TIME,
        PROPERTY_REG_FAILURE_COUNT
    };

    /**
     * @brief Indicate the registration result.
     *
     * @see IAosRegistrationListener#Registration_StateChanged
     */
    enum
    {
        RESULT_NONE = 0,

        RESULT_SUCCESS,
        RESULT_TRYING,
        RESULT_FAILURE
    };

    /**
     * @brief Indicate the reason of the registration result.
     *
     * @see IAosRegistrationListener#Registration_StateChanged
     */
    enum
    {
        REASON_NONE = 0,

        /// RESULT_SUCCESS

        /// RESULT_TRYING
        REASON_TRYING_START,
        REASON_TRYING_UPDATE,
        REASON_TRYING_STOP,

        /// RESULT_FAILURE
        REASON_FAILURE_GENERAL,
        REASON_FAILURE_SPECIAL,

        REASON_FAILURE_FORBIDDEN,
        REASON_FAILURE_AUTHENTICATION,
        REASON_FAILURE_TERMINATED,
        REASON_FAILURE_INTERNAL,
        REASON_FAILURE_BANNDED,
        REASON_FAILURE_INVALID_REGINFO,
        REASON_FAILURE_PDN_RECONNECT,
        REASON_FAILURE_PDN_RECONNECT_WITH_AWT,
        REASON_FAILURE_NEXT_PCSCF_REQUIRED,
        REASON_FAILURE_REG_TERMINATING,
        REASON_FAILURE_PCO_LIMITED_SERVICE,

        REASON_WFC_E911ADDRESS_MISSING,
        REASON_WFC_E911ADDRESS_DONE,

        /// Specific Define
        REASON_OTHERS = 20,
        REASON_FAILURE_OUTAGE,
        REASON_FAILURE_FORBIDDEN_NOWARNING
    };

protected:
    friend class AosBuildDirector;
    friend class AosAppContext;

    /**
     * @brief Initialize the registration class
     */
    virtual void Init() = 0;

    /**
     * @brief Clean up the registration class
     */
    virtual void CleanUp() = 0;
};
#endif  // INTERFACE_AOS_REGISTRATION_H_
