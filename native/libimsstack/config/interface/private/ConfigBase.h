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
#ifndef CONFIG_BASE_H_
#define CONFIG_BASE_H_

#include "ImsList.h"
#include "ImsMap.h"
#include "ImsSlot.h"

#include "ICarrierConfig.h"
#include "IConfigurable.h"

class IImsPrivateProperty;

class IConfigBuffer;

class ConfigBase : public ImsSlot, public ICarrierConfigListener
{
public:
    explicit ConfigBase(IN IMS_SINT32 nSlotId);
    virtual ~ConfigBase() = 0;

    ConfigBase(IN const ConfigBase&) = delete;
    ConfigBase& operator=(IN const ConfigBase&) = delete;

public:
    inline virtual IMS_BOOL Init() { return Load(); }
    inline virtual void Refresh() {}

    IMS_BOOL Load(IN const AString& strConfName = AString::ConstNull());
    IMS_BOOL Store(IN const AString& strConfName = AString::ConstNull());

protected:
    inline void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 /*nSlotId*/) override {}

    virtual IMS_BOOL ReadFrom();
    virtual IMS_BOOL WriteTo();
    virtual IMS_BOOL ReadFrom(IN const AString& strConfName);
    virtual IMS_BOOL WriteTo(IN const AString& strConfName);
    virtual IMS_BOOL Update(IN IMS_SINT32 nCpi, IN const AString& strValue = AString::ConstNull());

    IMS_BOOL AddListener(IN IMS_SINT32 nCpi, IN IConfigUpdateListener* piListener);
    void RemoveListener(IN IMS_SINT32 nCpi, IN IConfigUpdateListener* piListener);
    IMS_BOOL NotifyUpdate(IN IMS_SINT32 nCpi, IN const AString& strConfName = AString::ConstNull(),
            IN const AString& strExtraParam = AString::ConstNull());

    IConfigBuffer* GetConfigBufferFromContent(IN const AString& strContent) const;

    ICarrierConfig* GetCarrierConfig();
    IImsPrivateProperty* GetPrivateProperty();

protected:
    class Configurable : public IConfigurable
    {
    public:
        inline Configurable(IN ConfigBase* pConfig) :
                m_pConfig(pConfig)
        {
        }
        inline virtual ~Configurable() {}

        Configurable(IN const Configurable&) = delete;
        Configurable& operator=(IN const Configurable&) = delete;

    public:
        // IConfigurable class
        inline virtual IMS_BOOL AddListener(
                IN IMS_SINT32 nCpi, IN IConfigUpdateListener* piListener)
        {
            return (m_pConfig != IMS_NULL) ? m_pConfig->AddListener(nCpi, piListener) : IMS_FALSE;
        }

        inline virtual void RemoveListener(IN IMS_SINT32 nCpi, IN IConfigUpdateListener* piListener)
        {
            if (m_pConfig != IMS_NULL)
            {
                m_pConfig->RemoveListener(nCpi, piListener);
            }
        }

        inline virtual IMS_BOOL Update(
                IN IMS_SINT32 nCpi, IN const AString& strValue = AString::ConstNull())
        {
            return (m_pConfig != IMS_NULL) ? m_pConfig->Update(nCpi, strValue) : IMS_FALSE;
        }

    private:
        ConfigBase* m_pConfig;
    };

public:
    static const IMS_CHAR SECTION_UNIQUENESS[];

    // Listener for configuration update notification
    IMSMap<IMS_SINT32, IMSList<IConfigUpdateListener*>> m_objConfigUpdateListeners;
};

#endif
