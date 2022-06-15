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
#ifndef INTERFACE_ON_DIRECT_CORE_SERVICE_LISTENER_H_
#define INTERFACE_ON_DIRECT_CORE_SERVICE_LISTENER_H_

#include "ImsTypeDef.h"

class CoreService;
class ISipConnectionFactory;

/**
 * @brief A listener type for receiving an incoming SIP request notifications.
 *        All the transactions are managed by the application.
 *
 * @see ICoreService
 */
class IOnDirectCoreServiceListener
{
public:
    /**
     * @brief Notifies the application when the SIP server transaction is created and received.
     *
     * @param piService The concerned IService object
     * @param piScf The ISipConnectionFactory object
     * @return The result of a direct SIP transaction.\n
     *         #CoreService#RESULT_DIRECT_TXN_HANDLED\n
     *         #CoreService#RESULT_DIRECT_TXN_NOT_HANDLED\n
     *         #CoreService#RESULT_DIRECT_TXN_BYPASS
     */
    virtual IMS_SINT32 OnDirectCoreService_TransactionReceived(
            IN CoreService* pService, IN ISipConnectionFactory* piScf) = 0;
};

#endif
