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
#ifndef INTERFACE_SIP_CONNECTION_NOTIFIER_ERROR_LISTENER_H_
#define INTERFACE_SIP_CONNECTION_NOTIFIER_ERROR_LISTENER_H_

#include "AString.h"

class ISipConnectionNotifier;

/**
 * @brief This class provides a listener interface for notifications about failure
 *        of the transport layer's operations.
 *
 * @see ISipConnectionNotifier
 */
class ISipConnectionNotifierErrorListener
{
protected:
    virtual ~ISipConnectionNotifierErrorListener() = default;

public:
    /**
     * @brief Called when any error occurs in the SipConnectionNotifier.
     *
     * @param piScn Pointer to ISipConnectionNotifier object which error occurs
     * @param nCode Reason code of error
     * @param strMessage Reason phrase of error\n
     *                   Implementation dependent non-localized information about the error
     */
    virtual void ConnectionNotifierError_NotifyError(IN ISipConnectionNotifier* piScn,
            IN IMS_SINT32 nCode, IN const AString& strMessage) = 0;
};

#endif
