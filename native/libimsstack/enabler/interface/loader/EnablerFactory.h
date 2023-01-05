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
#ifndef ENABLER_FACTORY_H_
#define ENABLER_FACTORY_H_

#include "ImsList.h"
#include "ImsMap.h"

#include "IEnabler.h"

class IMutex;

class EnablerFactory
{
public:
    EnablerFactory();
    ~EnablerFactory();

    EnablerFactory(IN const EnablerFactory&) = delete;
    EnablerFactory& operator=(IN const EnablerFactory&) = delete;

public:
    void CreateEnablers(IN IMS_SINT32 nSlotId);
    void DestroyEnablers(IN IMS_SINT32 nSlotId);
    const IMSList<IEnabler*>* GetEnablers(IN IMS_SINT32 nSlotId) const;

private:
    void CreateEnablers(IN IMS_SINT32 nSlotId, OUT IMSList<IEnabler*>*& pEnablers);

private:
    IMutex* m_piLock;
    // <slotId, enablers>
    IMSMap<IMS_SINT32, IMSList<IEnabler*>*> m_objImsEnablers;
};

#endif
