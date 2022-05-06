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
#ifndef IMS_CARRIER_CONFIG_H_
#define IMS_CARRIER_CONFIG_H_

#include "ICarrierConfig.h"
#include "ImsSlot.h"

class ImsCarrierConfig : public ImsSlot, public ICarrierConfig
{
public:
    inline explicit ImsCarrierConfig(IN IMS_SINT32 nSlotId) :
            ImsSlot(nSlotId)
    {
    }
    inline virtual ~ImsCarrierConfig() {}

    ImsCarrierConfig(IN const ImsCarrierConfig&) = delete;
    ImsCarrierConfig& operator=(IN const ImsCarrierConfig&) = delete;

public:
    virtual void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) = 0;
    virtual void LoadConfig() = 0;

    inline IMS_BOOL GetBoolean(
            IN const IMS_CHAR* /*pszKey*/, IN IMS_BOOL bDefaultValue = IMS_FALSE) const override
    {
        return bDefaultValue;
    }
    inline IMS_SINT32 GetInt(
            IN const IMS_CHAR* /*pszKey*/, IN IMS_SINT32 nDefaultValue = -1) const override
    {
        return nDefaultValue;
    }
    inline IMS_SLONG GetLong(
            IN const IMS_CHAR* /*pszKey*/, IN IMS_SLONG nDefaultValue = -1L) const override
    {
        return nDefaultValue;
    }
    inline AString GetString(IN const IMS_CHAR* /*pszKey*/,
            IN const AString& strDefaultValue = AString::ConstNull()) const override
    {
        return strDefaultValue;
    }
    inline IMSVector<IMS_BOOL> GetBooleanArray(IN const IMS_CHAR* /*pszKey*/) const override
    {
        return IMSVector<IMS_BOOL>();
    }
    inline IMSVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* /*pszKey*/) const override
    {
        return IMSVector<IMS_SINT32>();
    }
    inline IMSVector<IMS_SLONG> GetLongArray(IN const IMS_CHAR* /*pszKey*/) const override
    {
        return IMSVector<IMS_SLONG>();
    }
    inline IMSVector<AString> GetStringArray(IN const IMS_CHAR* /*pszKey*/) const override
    {
        return IMSVector<AString>();
    }
    inline ICarrierConfig* GetBundle(IN const IMS_CHAR* /*pszKey*/) const override
    {
        return IMS_NULL;
    }
    inline void ReleaseBundle() override {}
    inline void AddListener(IN ICarrierConfigListener* /*piListener*/) override {}
    inline void RemoveListener(IN ICarrierConfigListener* /*piListener*/) override {}
};

#endif
