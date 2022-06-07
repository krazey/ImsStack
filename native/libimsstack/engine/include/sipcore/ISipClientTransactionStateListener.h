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
#ifndef INTERFACE_SIP_CLIENT_TRANSACTION_STATE_LISTENER_H_
#define INTERFACE_SIP_CLIENT_TRANSACTION_STATE_LISTENER_H_

#include "msg/SipMessage.h"

class SipClientTransactionState;

/**
 * @brief The class defines a listener interface for an incoming SIP response.
 */
class ISipClientTransactionStateListener
{
public:
    virtual void ClientTransactionState_ForkedResponseReceived(
            IN SipClientTransactionState* pCtState) = 0;

    virtual void ClientTransactionState_ResponseReceived(IN ::SipMessage* pSipMsg) = 0;
};

#endif
