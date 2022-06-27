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
#ifndef INTERFACE_REGISTRATION_LISTENER_H_
#define INTERFACE_REGISTRATION_LISTENER_H_

#include "ByteArray.h"

class IRegistration;

/**
 * @brief This class provides a listener interface for IMS registration.
 */
class IRegistrationListener
{
public:
    /**
     * @brief Notifies the application when the registration is challenged by S-CSCF.
     *
     * When the method is invoked, the application can determine whether it sends
     * the second SIP request or not.\n
     * If bResponseToChallenge is IMS_FALSE, the engine do not any action and maintain
     * all the information of this registration, but the state of this registration is
     * in the IRegistration#STATE_TERMINATED.
     *
     * @param nAlgorithm Type of algorithm; refer to #Credential in Credential.h
     * @param bResponseToChallenge Flag to determine whether the second SIP request
     *                             (w/ Authorization header) should be sent or not;
     *                             The default value is IMS_TRUE
     */
    virtual void Registration_AuthenticationChallenged(
            IN IMS_SINT32 nAlgorithm, OUT IMS_BOOL& bResponseToChallenge) = 0;

    /**
     * @brief Notifies the application when the AKA response is received from ISIM application.
     *
     * This method is invoked only and only if the algorithm is AKAv1-MD5.
     *
     * @param nResult Result of AKA authentication; refer to ImsAkaParam class in Credential.h
     * @param objIk Integrity key value; it is valid if nResult is only RESULT_OK
     * @param objCk Ciphering key value; it is valid if nResult is only RESULT_OK
     * @param bResultOfSa Result of security association
     */
    virtual void Registration_NotifyAkaResponse(IN IMS_SINT32 nResult, IN const ByteArray& objIk,
            IN const ByteArray& objCk, OUT IMS_BOOL& bResultOfSa) = 0;

    /**
     * @brief Notifies the application when the registration's refresh timer is expired.
     *
     * The refresh of registration will be handled by the application.
     *
     * @param bDoImplicitRefresh flag to indicate that refresh request is sent from engine or not
     */
    virtual void Registration_RefreshTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) = 0;

    /**
     * @brief Notifies the application when the initial registration is done successfully.
     */
    virtual void Registration_Started() = 0;

    /**
     * @brief Notifies the application when the initial registration is failed.
     *
     * @param nReason Reason code (#IRegistration)
     */
    virtual void Registration_StartFailed(IN IMS_SINT32 nReason) = 0;

    /**
     * @brief Notifies the application when the re-registration(modify) is done successfully.
     */
    virtual void Registration_Updated() = 0;

    /**
     * @brief Notifies the application when the re-registration is failed.
     *
     * @param nReason Reason code (#IRegistration)
     */
    virtual void Registration_UpdateFailed(IN IMS_SINT32 nReason) = 0;

    /**
     * @brief Notifies the application that the registration is successfully removed.
     */
    virtual void Registration_Removed() = 0;

    /**
     * @brief Notifies the application when the de-registration is completed.
     *
     * @param nReason Reason code (#IRegistration)
     */
    virtual void Registration_Terminated(IN IMS_SINT32 nReason) = 0;
};

#endif
