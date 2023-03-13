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
#ifndef INTERFACE_ISIM_LISTENER_H_
#define INTERFACE_ISIM_LISTENER_H_

#include "ByteArray.h"
#include "ImsList.h"

class IIsimListener
{
protected:
    virtual ~IIsimListener() = default;

public:
    /**
     * @brief Notifies the application that the specified field value is retrieved.
     *
     * @param nField The field type to be requested
     * @param objValues The field values
     */
    virtual void Isim_OnField(IN IMS_SINT32 nField, IN const ImsList<ByteArray>& objValues) = 0;

    /**
     * @brief Notifies the application that the home domain name is retrieved.
     *
     * @param objDomainName The home domain name
     */
    virtual void Isim_OnHomeDomainName(IN const ByteArray& objDomainName) = 0;

    /**
     * @brief Notifies the application that the private user identity is retrieved.
     *
     * @param objImpi The private user identity
     */
    virtual void Isim_OnImpi(IN const ByteArray& objImpi) = 0;

    /**
     * @brief Notifies the application that the public user identities are retrieved.
     *
     * @param objImpus The list of public user identity
     */
    virtual void Isim_OnImpu(IN const ImsList<ByteArray>& objImpus) = 0;

    /**
     * @brief Notifies the application that the error occurrs in the ISIM module.
     *
     * @param nErrorCode The error code
     */
    virtual void Isim_OnError(IN IMS_SINT32 nErrorCode) = 0;

    /**
     * @brief Notifies the application that the ISIM state is changed.
     *
     * @param nState The ISIM state
     */
    virtual void Isim_OnStateChanged(IN IMS_SINT32 nState) = 0;
};

#endif
