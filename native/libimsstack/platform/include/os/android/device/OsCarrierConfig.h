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
#ifndef OS_CARRIER_CONFIG_H_
#define OS_CARRIER_CONFIG_H_

#include <binder/PersistableBundle.h>

#include "ImsCarrierConfig.h"
#include "system-intf/ISystemListener.h"

class IThread;

class OsCarrierConfig : public ImsCarrierConfig, public ISystemListener
{
public:
    explicit OsCarrierConfig(IN IMS_SINT32 nSlotId);
    virtual ~OsCarrierConfig();

    OsCarrierConfig(IN const OsCarrierConfig&) = delete;
    OsCarrierConfig& operator=(IN const OsCarrierConfig&) = delete;

private:
    explicit OsCarrierConfig(IN IMS_SINT32 nSlotId, IN IMS_BOOL bIsBundle,
            IN const android::os::PersistableBundle& objConfig);

public:
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) final;
    void LoadConfig() final;

    IMS_BOOL GetBoolean(
            IN const IMS_CHAR* pszKey, IN IMS_BOOL bDefaultValue = IMS_FALSE) const final;
    IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nDefaultValue = -1) const final;
    IMS_SLONG GetLong(IN const IMS_CHAR* pszKey, IN IMS_SLONG nDefaultValue = -1L) const final;
    AString GetString(IN const IMS_CHAR* pszKey,
            IN const AString& strDefaultValue = AString::ConstNull()) const final;
    IMSVector<IMS_BOOL> GetBooleanArray(IN const IMS_CHAR* pszKey) const final;
    IMSVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* pszKey) const final;
    IMSVector<IMS_SLONG> GetLongArray(IN const IMS_CHAR* pszKey) const final;
    IMSVector<AString> GetStringArray(IN const IMS_CHAR* pszKey) const final;
    ICarrierConfig* GetBundle(IN const IMS_CHAR* pszKey) const final;
    void ReleaseBundle() final;
    void AddListener(IN ICarrierConfigListener* piListener) final;
    void RemoveListener(IN ICarrierConfigListener* piListener) final;

private:
    void NotifyConfigChanged();

#ifdef __IMS_DEBUG__
    void DisplayCarrierConfig();
    void DisplaySpecificConfigs();
#endif

private:
    IThread* m_piOwnerThread;
    IMS_BOOL m_bIsBundle;
    IMSList<ICarrierConfigListener*> m_objListeners;
    android::os::PersistableBundle m_objConfig;
};

#endif
