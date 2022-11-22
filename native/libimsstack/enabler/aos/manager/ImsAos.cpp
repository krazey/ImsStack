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
#include "manager/AosFactory.h"
#include "manager/ImsAosManager.h"
#include "ImsAos.h"

PUBLIC GLOBAL IImsAos* ImsAos::GetImsAos(IN const AString& strAppId, IN const AString& strServiceId,
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    ImsAosManager* pManager = AosFactory::GetManager(nSlotId);

    return (pManager != IMS_NULL) ? pManager->GetImsAos(strAppId, strServiceId) : IMS_NULL;
}

PUBLIC GLOBAL IMSList<IImsAos*> ImsAos::GetImsAosList(IN const AString& strAppId,
        IN const AString& strServiceId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    ImsAosManager* pManager = AosFactory::GetManager(nSlotId);

    return (pManager != IMS_NULL) ? pManager->GetImsAosList(strAppId, strServiceId)
                                  : IMSList<IImsAos*>();
}

PUBLIC GLOBAL IMSList<IImsAos*> ImsAos::GetImsAosList(
        IN const AString& strAppId, IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    ImsAosManager* pManager = AosFactory::GetManager(nSlotId);

    return (pManager != IMS_NULL) ? pManager->GetImsAosList(strAppId) : IMSList<IImsAos*>();
}
