/**
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

#ifndef _IMS_MEDIA_ENVIRONMENT_H_
#define _IMS_MEDIA_ENVIRONMENT_H_

#include "IService.h"
#include "IMSTypeDef.h"
#include "MediaDef.h"

class MediaEnvironment
{
public:
    MEDIA_NETWORK_TYPE eNetworkType;
    MEDIA_SERVICE_TYPE eServiceType;
    IService* pIService;

public:
    MediaEnvironment() :
            eNetworkType(MEDIA_NETWORK_NONE),
            eServiceType(MEDIA_SERVICE_DEFAULT),
            pIService(IMS_NULL){};

    MediaEnvironment(MEDIA_NETWORK_TYPE eNetwork, MEDIA_SERVICE_TYPE eCapa, IService* service) :
            eNetworkType(eNetwork),
            eServiceType(eCapa),
            pIService(service){};
    void Copy(IN MediaEnvironment* pEnvironment)
    {
        if (pEnvironment != IMS_NULL)
        {
            eNetworkType = pEnvironment->eNetworkType;
            eServiceType = pEnvironment->eServiceType;
            pIService = pEnvironment->pIService;
        }
    };
};
#endif /* _IMS_MEDIA_ENVIRONMENT_H_ */
