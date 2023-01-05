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

#ifndef IMS_IDENTITY_PROXY_H_
#define IMS_IDENTITY_PROXY_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "common/ImsIdentity.h"

class AccessNetworkInfo;

class ImsIdentityProxy
{
public:
    inline ImsIdentityProxy() {}
    inline virtual ~ImsIdentityProxy() {}
    ImsIdentityProxy(IN const ImsIdentityProxy&) = delete;
    ImsIdentityProxy& operator=(IN const ImsIdentityProxy&) = delete;

    inline virtual AString CreateSipUserId(IN const AString& strDialString, IN IMS_SINT32 nSlotId,
            IN IMS_BOOL bUserPhoneParam = IMS_FALSE,
            IN const AString& strPhoneContext = AString::ConstNull()) const
    {
        return ImsIdentity::CreateSipUserId(
                strDialString, nSlotId, bUserPhoneParam, strPhoneContext);
    }

    inline virtual AString CreateSipUserIdWithDialString(IN const AString& strDialString,
            IN IMS_SINT32 nSlotId, IN const AString& strPhoneContext = AString::ConstNull()) const
    {
        return ImsIdentity::CreateSipUserIdWithDialString(strDialString, nSlotId, strPhoneContext);
    }

    inline virtual AString CreateSipUserIdWithPhone(IN const AString& strDialString,
            IN IMS_SINT32 nSlotId, IN const AString& strPhoneContext = AString::ConstNull()) const
    {
        return ImsIdentity::CreateSipUserIdWithPhone(strDialString, nSlotId, strPhoneContext);
    }

    inline virtual const AString GetPhoneContext(IN IMS_SINT32 nDialingPolicy,
            IN IMS_SINT32 nSlotId, IN AccessNetworkInfo* pAni = IMS_NULL,
            IN const AString& strSubscriberId = AString::ConstNull()) const
    {
        return ImsIdentity::GetPhoneContext(nDialingPolicy, nSlotId, pAni, strSubscriberId);
    }

    inline virtual const AString GetHomeDomainName(IN IMS_SINT32 nSlotId) const
    {
        return ImsIdentity::GetHomeDomainName(nSlotId);
    }
};

#endif
