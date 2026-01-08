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

#ifndef INTERFACE_INTERFACE_HOLDER_LISTENER_H_
#define INTERFACE_INTERFACE_HOLDER_LISTENER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

/**
 * @brief A listener interface for receiving notifications about state changes in SIP interface
 * holders.
 *
 * This class defines callback methods for major state changes in the Session, Reference, and
 * Subscription interface holders.
 */
class IInterfaceHolderListener
{
public:
    inline virtual ~IInterfaceHolderListener() {};

    /**
     * @brief Notifies that the last ISession interface associated with the given call key has been
     *        released.
     *
     * @param nKey The call key for which the sessions were released.
     * @param objSession The last session interface that was released for this call key.
     */
    inline virtual void OnSessionInterfaceReleased(
            IN [[maybe_unused]] CallKey nKey, IN [[maybe_unused]] ISession& objSession) {};

    /**
     * @brief Notifies that all IReference interfaces have been cleared from the
     *        ReferenceInterfaceHolder.
     */
    inline virtual void OnReferenceInterfaceCleared() {};

    /**
     * @brief Notifies that all ISubscription interfaces have been cleared from the
     *        SubscriptionInterfaceHolder.
     */
    inline virtual void OnSubscriptionInterfaceCleared() {};
};

#endif
