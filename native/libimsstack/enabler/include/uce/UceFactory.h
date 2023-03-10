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
#ifndef UCE_FACTORY_H_
#define UCE_FACTORY_H_

#include "ImsMap.h"
#include "AString.h"

class UceApp;
/**
 * @brief This is the factory class for Uce.
 */

class UceFactory
{
public:
    UceFactory();
    virtual ~UceFactory();

    static void Start(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    static void Stop(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    static UceApp* GetUceApp(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

protected:
    static UceFactory* m_gpUceFactory;

    /// <slot-id, ImsAosManager>
    static ImsMap<IMS_SINT32, UceApp*> m_objUceManagers;
};

#endif  // UCE_FACTORY_H_
