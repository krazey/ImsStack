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
#ifndef REG_BINDING_PROXY_H_
#define REG_BINDING_PROXY_H_

#include "AString.h"

class CallerCapability;
class IRegContact;
class IRegistrationEx;

class RegBindingProxy
{
public:
    RegBindingProxy() = delete;

public:
    // Create / Destroy
    static IMS_BOOL CreateBinding(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId, IN IRegistrationEx* piRegEx);
    static void DestroyBinding(
            IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId);
    static void DestroyBinding(IN IMS_SINT32 nSlotId, IN IRegistrationEx* piRegEx);

    // Contact binding
    static IMS_BOOL BindContact(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId, IN IRegContact* piContact);
    static void UnbindContact(
            IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId);
    static void UnbindContact(IN IMS_SINT32 nSlotId, IN IRegContact* piContact);

    // Additional informations
    static void QueryCapability(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId, OUT CallerCapability*& pCapability);
    static void QueryRegistrationHeaders(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId, OUT AStringArray& objHeaders);
};

#endif
