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
#include "ServiceMemory.h"

#include "ImsCoreProtocol.h"
#include "ProtocolPermission.h"
#include "SipProtocol.h"

struct ProtocolBinder
{
    const IMS_CHAR* pszName;
    Protocol* pProtocol;
};

static const ProtocolBinder PROTOCOL_BINDER[] = {
        {ImsCore::CONNECTION_SCHEME, ImsCoreProtocol::GetInstance()},
        {Sip::CONNECTION_SCHEME_SIP, SipProtocol::GetInstance()    }
  //    { Sip::CONNECTION_SCHEME_SIPS, SipProtocol::GetInstance() }
};

PUBLIC
Protocol* ProtocolPermission::Lookup(IN const AString& strName)
{
    IMS_SINT32 nNumOfBinder = sizeof(PROTOCOL_BINDER) / sizeof(PROTOCOL_BINDER[0]);

    for (IMS_SINT32 i = 0; i < nNumOfBinder; ++i)
    {
        const ProtocolBinder* pBinder = &(PROTOCOL_BINDER[i]);

        if (strName.EqualsIgnoreCase(pBinder->pszName))
        {
            return pBinder->pProtocol;
        }
    }

    return IMS_NULL;
}
