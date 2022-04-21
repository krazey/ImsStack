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
#include "IMSMsgDef.h"
#include "SystemConfig.h"
#include "PlatformFactory.h"
#include "ImsVoNr.h"
#include "ServiceVoNR.h"


class VoNRHolder
{
public:
    inline VoNRHolder()
        : piVoNR(IMS_NULL)
    {}
    inline ~VoNRHolder()
    {
        PlatformFactory::DestroyVoNr(piVoNR);
    }

private:
    VoNRHolder(IN CONST VoNRHolder &objRHS);
    VoNRHolder& operator=(IN CONST VoNRHolder &objRHS);

public:
    inline IVoNR* GetVoNR(IN IMS_SINT32 nSlotId)
    {
        if (piVoNR == IMS_NULL)
        {
            piVoNR = PlatformFactory::CreateVoNr(nSlotId);
        }

        return piVoNR;
    }

private:
    IVoNR *piVoNR;
};

class VoNRServicePrivate
{
public:
    VoNRServicePrivate();
    ~VoNRServicePrivate();

private:
    VoNRServicePrivate(IN CONST VoNRServicePrivate &objRHS);
    VoNRServicePrivate& operator=(IN CONST VoNRServicePrivate &objRHS);

public:
    VoNRHolder* GetHolder(IN IMS_SINT32 nSlotId);

private:
    VoNRHolder **ppHolder;
};

PUBLIC
VoNRServicePrivate::VoNRServicePrivate()
    : ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    ppHolder = new VoNRHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        ppHolder[i] = new VoNRHolder();
    }
}

PUBLIC
VoNRServicePrivate::~VoNRServicePrivate()
{
    if (ppHolder != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (ppHolder[i] != IMS_NULL)
            {
                delete ppHolder[i];
            }
        }

        delete[] ppHolder;
    }
}

PUBLIC
VoNRHolder* VoNRServicePrivate::GetHolder(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        nSlotId = IMS_SLOT_0;
    }

    return ppHolder[nSlotId];
}

PRIVATE
VoNRService::VoNRService()
    : pPrivate(new VoNRServicePrivate())
{
}

PRIVATE
VoNRService::~VoNRService()
{
    if (pPrivate != IMS_NULL)
    {
        delete pPrivate;
    }
}

PUBLIC
IVoNR* VoNRService::GetVoNR(IN IMS_SINT32 nSlotId)
{
    VoNRHolder *pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetVoNR(nSlotId);
}

PUBLIC
void VoNRService::DispatchServiceMessage(IN IMSMSG &objMSG)
{
    switch (objMSG.GetName())
    {
        case IMS_MSG_VONR:
        {
            IMS_SINT32 nSlotId = LONG_TO_SINT(objMSG.nWparam);
            ImsVoNr* pVoNR = DYNAMIC_CAST(ImsVoNr*, GetVoNR(nSlotId));

            if (pVoNR != IMS_NULL)
            {
                pVoNR->DispatchServiceMessage(objMSG.nWparam, objMSG.nLparam);
            }
            break;
        }

        default:
            // no-op
            break;
    }
}

PUBLIC GLOBAL
VoNRService* VoNRService::GetVoNRService()
{
    static VoNRService *pVoNRService = IMS_NULL;

    if (pVoNRService == IMS_NULL)
    {
        pVoNRService = new VoNRService();
    }

    return pVoNRService;
}
