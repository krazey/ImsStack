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
#ifndef INTERFACE_DIRECT_CORE_SERVICE_LISTENER_H_
#define INTERFACE_DIRECT_CORE_SERVICE_LISTENER_H_

#include "ImsTypeDef.h"

class ICoreService;
class ISipConnectionFactory;

/**
 * @brief This class provides a listener interface to receive an incoming SIP request
 *        notifications.
 *
 * All the transactions are managed and handled by the application.
 *
 * @see ICoreService
 */
class IDirectCoreServiceListener
{
public:
    /**
     * @brief Notifies the application when the SIP server transaction is created and received.
     *
     * @param piService Pointer to the concerned IService
     * @param piScf Pointer to the ISipConnectionFactory
     * @return The result of direct transaction handling.\n
     *         #ICoreService#RESULT_DIRECT_TXN_HANDLED\n
     *         #ICoreService#RESULT_DIRECT_TXN_NOT_HANDLED\n
     *         #ICoreService#RESULT_DIRECT_TXN_BYPASS
     */
    virtual IMS_SINT32 DirectCoreService_TransactionReceived(
            IN ICoreService* piService, IN ISipConnectionFactory* piScf) = 0;
};

#endif
