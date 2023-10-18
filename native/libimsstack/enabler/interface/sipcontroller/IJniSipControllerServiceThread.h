/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef INTERFACE_JNI_SIPCONTROLLER_THREAD_H_
#define INTERFACE_JNI_SIPCONTROLLER_THREAD_H_

#include "BaseServiceThread.h"
#include "IJniEnablerThread.h"

class IJniSipControllerServiceThread : public IJniEnablerThread
{
public:
    virtual ~IJniSipControllerServiceThread() {}

    /**
     * @brief Sends a new incoming SIP message to the message application
     *
     */
    virtual void OnMessageReceived();

    /**
     * @brief Notifies the remote application that a previous request to send a SIP message using
     * { @link SipDelegate#sendMessage } has succeeded.
     */
    virtual void OnMessageSent();

    /**
     * @brief Notifies the remote application that a previous request to send a SIP message using
     * { @link SipDelegate#sendMessage } has failed.
     */
    virtual void OnMessageSendFailure();

    /**
     * @brief Updates the ims registration with new values
     * @param nParam Feature tags
     */
    virtual void OnRegistrationUpdated(IN IMS_UINTP nParam);

    /**
     * @brief Updates configuration values
     */
    virtual void OnConfigurationUpdated();
};

#endif  // INTERFACE_JNI_SIPCONTROLLER_THREAD_H_
