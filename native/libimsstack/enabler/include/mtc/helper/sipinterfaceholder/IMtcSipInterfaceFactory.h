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

#ifndef INTERFACE_MTC_SIP_INTERFACE_FACTORY_H_
#define INTERFACE_MTC_SIP_INTERFACE_FACTORY_H_

class SessionInterfaceHolder;
class ReferenceInterfaceHolder;
class SubscriptionInterfaceHolder;

/**
 * @brief An interface for a factory that provides holders for various SIP interfaces.
 */
class IMtcSipInterfaceFactory
{
public:
    virtual ~IMtcSipInterfaceFactory() {}

    /**
     * @brief Gets the session interface holder.
     *
     * @return A reference to the SessionInterfaceHolder.
     */
    virtual SessionInterfaceHolder& GetISessionHolder() = 0;

    /**
     * @brief Gets the reference interface holder.
     *
     * @return A pointer to the ReferenceInterfaceHolder.
     */
    virtual ReferenceInterfaceHolder* GetIReferenceHolder() = 0;

    /**
     * @brief Gets the subscription interface holder.
     *
     * @return A pointer to the SubscriptionInterfaceHolder.
     */
    virtual SubscriptionInterfaceHolder* GetISubscriptionHolder() = 0;
};

#endif
