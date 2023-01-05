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
#include "ImsCarrierConfig.h"
#include "ImsMap.h"
#include "ImsMessageDef.h"
#include "PlatformContext.h"
#include "ServiceConfig.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"

__IMS_TRACE_TAG_BASE__;

class ConfigServicePrivate
{
public:
    ConfigServicePrivate();
    ~ConfigServicePrivate();

    ConfigServicePrivate(IN const ConfigServicePrivate&) = delete;
    ConfigServicePrivate& operator=(IN const ConfigServicePrivate&) = delete;

public:
    ImsCarrierConfig* GetCarrierConfig(IN IMS_SINT32 nSlotId);

public:
    IMSMap<IMS_SINT32, ImsCarrierConfig*> m_objCarrierConfigs;
};

PUBLIC
ConfigServicePrivate::ConfigServicePrivate()
{
    for (IMS_UINT32 i = 0; i < SystemConfig::GetSupportedSimCount(); ++i)
    {
        m_objCarrierConfigs.Add(i, IMS_NULL);
    }
}

PUBLIC
ConfigServicePrivate::~ConfigServicePrivate()
{
    for (IMS_UINT32 i = 0; i < m_objCarrierConfigs.GetSize(); ++i)
    {
        ImsCarrierConfig* pCc = m_objCarrierConfigs.GetValueAt(i);

        if (pCc != IMS_NULL)
        {
            pCc->Destroy();
        }
    }

    m_objCarrierConfigs.Clear();
}

PUBLIC
ImsCarrierConfig* ConfigServicePrivate::GetCarrierConfig(IN IMS_SINT32 nSlotId)
{
    if (nSlotId < 0 || nSlotId >= static_cast<IMS_SINT32>(m_objCarrierConfigs.GetSize()))
    {
        return IMS_NULL;
    }

    ImsCarrierConfig* pCc = m_objCarrierConfigs.GetValue(nSlotId);

    if (pCc == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        pCc = piOsFactory->CreateCarrierConfig(nSlotId);

        m_objCarrierConfigs.SetValue(nSlotId, pCc);
    }

    return pCc;
}

PRIVATE
ConfigService::ConfigService() :
        m_pPrivate(new ConfigServicePrivate())
{
}

PRIVATE
ConfigService::~ConfigService()
{
    delete m_pPrivate;
}

PUBLIC
ICarrierConfig* ConfigService::GetCarrierConfig(IN IMS_SINT32 nSlotId)
{
    return m_pPrivate->GetCarrierConfig(nSlotId);
}

PUBLIC
void ConfigService::LoadCarrierConfig(IN IMS_SINT32 nSlotId)
{
    ImsCarrierConfig* pCc = m_pPrivate->GetCarrierConfig(nSlotId);

    if (pCc != IMS_NULL)
    {
        pCc->LoadConfig();
    }
}

PUBLIC
void ConfigService::DispatchServiceMessage(IN ImsMessage& objMsg)
{
    IMS_TRACE_D("ConfigService: DispatchServiceMessage - msg=%d, wp=%" PFLS_u ", lp=%" PFLS_u,
            objMsg.GetName(), objMsg.nWparam, objMsg.nLparam);

    switch (objMsg.GetName())
    {
        case IMS_MSG_CONFIGURATION:
        {
            IMS_SINT32 nSlotId = LONG_TO_INT(objMsg.nWparam);
            ImsCarrierConfig* pCc = m_pPrivate->GetCarrierConfig(nSlotId);

            if (pCc != IMS_NULL)
            {
                pCc->DispatchServiceMessage(objMsg.nWparam, objMsg.nLparam);
            }
        }
        break;
        default:
            // no-op
            break;
    }
}

PUBLIC GLOBAL ConfigService* ConfigService::GetConfigService()
{
    return DYNAMIC_CAST(ConfigService*,
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_CONFIG));
}
