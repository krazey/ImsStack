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
#ifndef SERVICE_PROTOCOL_H_
#define SERVICE_PROTOCOL_H_

#include "Protocol.h"

class IService;

class ServiceProtocol : public Protocol
{
public:
    inline ServiceProtocol() :
            Protocol()
    {
    }
    ~ServiceProtocol() override = default;

    ServiceProtocol(IN const ServiceProtocol&) = delete;
    ServiceProtocol& operator=(IN const ServiceProtocol&) = delete;

public:
    IConnection* OpenPrim(IN const AString& strName) override;
    IConnection* OpenPrim(IN const AString& strScheme, IN const AString& strTarget,
            IN const AString& strParams) override;

protected:
    virtual IService* CreateService(IN const AString& strAppId, IN const AString& strServiceId,
            IN const AString& strUserId);
    virtual const IMS_CHAR* GetConnectionScheme() const;

public:
    // "userId"
    static const IMS_CHAR CONNECTION_PARAM_USER_ID[];
    // "serviceId"
    static const IMS_CHAR CONNECTION_PARAM_SERVICE_ID[];
};

#endif
