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
#ifndef INTERFACE_REG_BINDING_LISTENER_H_
#define INTERFACE_REG_BINDING_LISTENER_H_

#include "AStringArray.h"

class CallerCapability;
class SipAddress;

class IRegBindingListener
{
public:
    /**
     * @brief Notifies the application that the registration binding (AOR - Contact) is
     *        in the ACTIVE state.
     */
    virtual void RegBinding_OnActive() = 0;

    /**
     * @brief Notifies the application that the registration binding (AOR - Contact) is
     *        just removed (by de-REGISTER).
     */
    virtual void RegBinding_OnDestroy() = 0;

    /**
     * @brief Notifies the application that the registration binding (AOR - Contact) is
     *        in the INIT state.
     *
     * @param pAor The address of record that this RegBinding is in INIT state
     */
    virtual void RegBinding_OnInit(IN const SipAddress* pAor) = 0;

    /**
     * @brief Queries the service capability to the application. It is for the caller capabilities.
     *
     * @param pCapability The caller capabilities for this RegBinding.
     */
    virtual void RegBinding_OnQueryCapability(OUT CallerCapability*& pCapability) = 0;

    /**
     * @brief Queries the registration headers to the application.
     *
     * @param objHeaders The SIP headers to be set in REGISTER request
     */
    virtual void RegBinding_OnQueryRegistrationHeaders(OUT AStringArray& objHeaders) = 0;

    /**
     * @brief Notifies the application that the registration binding (AOR - Contact) is
     *        in the TERMINATED state.
     */
    virtual void RegBinding_OnTerminated() = 0;
};

#endif
