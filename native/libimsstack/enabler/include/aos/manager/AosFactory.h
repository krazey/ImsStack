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
#ifndef AOS_FACTORY_H_
#define AOS_FACTORY_H_

#include "IMSMap.h"
#include "AString.h"

class IMutex;
class ImsAosManager;

/**
 * @brief This is the factory class for AoS.
 */

class AosFactory
{
public:
    AosFactory();
    virtual ~AosFactory();

    static ImsAosManager* GetManager(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    static void Start(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    static void Stop(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

private:
    static AosFactory* m_gpAosFactory;
    static IMutex* m_gpiLock;

    /// <slot-id, ImsAosManager>
    static IMSMap<IMS_SINT32, ImsAosManager*> m_objManagers;
};

#endif  // AOS_FACTORY_H_
