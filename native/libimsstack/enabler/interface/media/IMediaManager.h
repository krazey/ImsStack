/**
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

#ifndef INTERFACE_MEDIA_MANAGER_H_
#define INTERFACE_MEDIA_MANAGER_H_

#include "IService.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"

class IMediaSession;

class IMediaManager
{
public:
    /**
     * @brief Destructor of IMediaManager
     */
    virtual ~IMediaManager(){};

    static IMediaManager* GetInstance(IN IMS_SINT32 nSlotId = 0);

    /**
     * @brief Creates a MediaSession instance with the service type and assign the jni thread
     * instance to communicate with jni thread to send and receive a message from java layer
     *
     * @param eNetwork The network type
     * @param eServiceType The service type, normal or emergency
     * @param pService The instance of the IMS service
     * @param nCallKey The key to identify the call session, each MediaSession has a unique key to
     * match with the call session
     * @return IMediaSession* created IMediaSession instance
     */
    virtual IMediaSession* CreateSession(IN MEDIA_NETWORK_TYPE eNetwork,
            IN MEDIA_SERVICE_TYPE eServiceType, IN IService* pService, IN IMS_SINTP nCallKey);

    /**
     * @brief Destroys the MediaSession instance
     *
     * @param piSession The instance to destroy
     */
    virtual void DestroySession(IN const IMediaSession* piSession);
};

#endif
