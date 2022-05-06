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
#ifndef IMS_PRIVATE_PROPERTY_H_
#define IMS_PRIVATE_PROPERTY_H_

#include "IImsPrivateProperty.h"

class ImsPrivateProperty : public IImsPrivateProperty
{
private:
    ImsPrivateProperty();
    ~ImsPrivateProperty();

public:
    static ImsPrivateProperty* GetInstance();

public:
    // Ephemeral properties : cleared whenever IMS process starts
    virtual AString Get(IN const AString& strKey, IN IMS_SINT32 nSlotId);
    virtual IMS_BOOL GetBoolean(IN const AString& strKey, IN IMS_SINT32 nSlotId);
    virtual IMS_SINT32 GetInt(IN const AString& strKey, IN IMS_SINT32 nSlotId);
    virtual void Set(IN const AString& strKey, IN const AString& strValue, IN IMS_SINT32 nSlotId);
    virtual void SetBoolean(IN const AString& strKey, IN IMS_BOOL bValue, IN IMS_SINT32 nSlotId);
    virtual void SetInt(IN const AString& strKey, IN IMS_SINT32 nValue, IN IMS_SINT32 nSlotId);

    // Persistent properties : preserved even though device or IMS re-starts
    virtual AString GetPersistent(IN const AString& strKey, IN IMS_SINT32 nSlotId);
    virtual IMS_BOOL GetPersistentBoolean(IN const AString& strKey, IN IMS_SINT32 nSlotId);
    virtual IMS_SINT32 GetPersistentInt(IN const AString& strKey, IN IMS_SINT32 nSlotId);
    virtual void SetPersistent(
            IN const AString& strKey, IN const AString& strValue, IN IMS_SINT32 nSlotId);
    virtual void SetPersistentBoolean(
            IN const AString& strKey, IN IMS_BOOL bValue, IN IMS_SINT32 nSlotId);
    virtual void SetPersistentInt(
            IN const AString& strKey, IN IMS_SINT32 nValue, IN IMS_SINT32 nSlotId);
};

#endif
