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
#ifndef INTERFACE_ON_SIP_CONNECTION_NOTIFIER_ERROR_LISTENER_H_
#define INTERFACE_ON_SIP_CONNECTION_NOTIFIER_ERROR_LISTENER_H_

#include "AString.h"

class SipConnectionNotifier;

/**
 * @brief This class defines a listener interface for notifications about failure of
 *        the transport layer operrations.
 *
 * @see SipConnectionNotifier
 */
class IOnSipConnectionNotifierErrorListener
{
public:
    /**
     * Called when any error occurrs in the SipConnectionNotifier.
     *
     * @param pScn The SipConnectionNotifier object which error occurrs
     * @param nCode The reason code of error
     * @param strMessage The reason phrase of error
     */
    virtual void OnConnectionNotifierError_NotifyError(
            IN SipConnectionNotifier* pScn, IN IMS_SINT32 nCode, IN const AString& strMessage) = 0;
};

#endif
