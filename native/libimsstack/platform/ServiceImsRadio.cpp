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
#include "ImsMap.h"
#include "ImsMessageDef.h"
#include "ImsRadio.h"
#include "PlatformContext.h"
#include "ServiceImsRadio.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"

__IMS_TRACE_TAG_BASE__;

class ImsRadioServicePrivate
{
public:
    ImsRadioServicePrivate();
    ~ImsRadioServicePrivate();

    ImsRadioServicePrivate(IN const ImsRadioServicePrivate&) = delete;
    ImsRadioServicePrivate& operator=(IN const ImsRadioServicePrivate&) = delete;

public:
    IImsTraffic* GetImsTraffic();
    ImsRadio* GetImsRadio(IN IMS_SINT32 nSlotId);

public:
    ImsMap<IMS_SINT32, ImsRadio*> m_objImsRadios;

private:
    IImsTraffic* m_piImsTraffic;
};

PUBLIC
ImsRadioServicePrivate::ImsRadioServicePrivate() :
        m_piImsTraffic(IMS_NULL)
{
    for (IMS_UINT32 i = 0; i < SystemConfig::GetSupportedSimCount(); ++i)
    {
        m_objImsRadios.Add(i, IMS_NULL);
    }
}

PUBLIC
ImsRadioServicePrivate::~ImsRadioServicePrivate()
{
    IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();

    piOsFactory->DestroyImsTraffic(m_piImsTraffic);

    for (IMS_UINT32 i = 0; i < m_objImsRadios.GetSize(); ++i)
    {
        ImsRadio* pImsRadio = m_objImsRadios.GetValueAt(i);

        if (pImsRadio != IMS_NULL)
        {
            delete pImsRadio;
        }
    }

    m_objImsRadios.Clear();
}

PUBLIC
IImsTraffic* ImsRadioServicePrivate::GetImsTraffic()
{
    if (m_piImsTraffic == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        m_piImsTraffic = piOsFactory->CreateImsTraffic();
    }

    return m_piImsTraffic;
}

PUBLIC
ImsRadio* ImsRadioServicePrivate::GetImsRadio(IN IMS_SINT32 nSlotId)
{
    if (nSlotId < 0 || nSlotId >= static_cast<IMS_SINT32>(m_objImsRadios.GetSize()))
    {
        return IMS_NULL;
    }

    ImsRadio* pImsRadio = m_objImsRadios.GetValue(nSlotId);

    if (pImsRadio == IMS_NULL)
    {
        IOsFactory* piOsFactory = PlatformContext::GetInstance()->GetOsFactory();
        pImsRadio = piOsFactory->CreateImsRadio(nSlotId);

        m_objImsRadios.SetValue(nSlotId, pImsRadio);
    }

    return pImsRadio;
}

PRIVATE
ImsRadioService::ImsRadioService() :
        m_pPrivate(new ImsRadioServicePrivate())
{
}

PRIVATE
ImsRadioService::~ImsRadioService()
{
    delete m_pPrivate;
}

PUBLIC
IImsRadio* ImsRadioService::GetImsRadio(IN IMS_SINT32 nSlotId)
{
    return m_pPrivate->GetImsRadio(nSlotId);
}

PUBLIC
IImsTraffic* ImsRadioService::GetImsTraffic()
{
    return m_pPrivate->GetImsTraffic();
}

PUBLIC
void ImsRadioService::DispatchServiceMessage(IN ImsMessage& objMsg)
{
    IMS_TRACE_D("ImsRadioService: DispatchServiceMessage - msg=%d, wp=%" PFLS_u ", lp=%" PFLS_u,
            objMsg.GetName(), objMsg.nWparam, objMsg.nLparam);

    switch (objMsg.GetName())
    {
        case IMS_MSG_RADIO:
        {
            IMS_SINT32 nSlotId = LONG_TO_INT(objMsg.nWparam);
            ImsRadio* piRadio = m_pPrivate->GetImsRadio(nSlotId);

            if (piRadio != IMS_NULL)
            {
                piRadio->DispatchServiceMessage(objMsg.nWparam, objMsg.nLparam);
            }
        }
        break;

        case IMS_MSG_TRAFFIC:
        {
            IImsTraffic* piTraffic = m_pPrivate->GetImsTraffic();

            if (piTraffic != IMS_NULL)
            {
                piTraffic->DispatchServiceMessage(objMsg.nWparam, objMsg.nLparam);
            }
        }
        break;
        default:
            // no-op
            break;
    }
}

PUBLIC GLOBAL ImsRadioService* ImsRadioService::GetImsRadioService()
{
    return DYNAMIC_CAST(ImsRadioService*,
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_RADIO));
}
