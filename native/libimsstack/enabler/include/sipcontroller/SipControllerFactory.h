/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef SIPCONTROLLER_FACTORY_H_
#define SIPCONTROLLER_FACTORY_H_

#include "ImsMap.h"
#include "AString.h"

class SipControllerManager;

/**
 * @brief This is the factory class for SipController.
 */
class SipControllerFactory
{
public:
    SipControllerFactory();
    virtual ~SipControllerFactory();

    static void Start(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    static void Stop(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    static SipControllerManager* GetSipControllerManager(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

protected:
    static SipControllerFactory* m_gpFactory;
    // <slotId, SipControllerManager>
    static IMSMap<IMS_SINT32, SipControllerManager*> m_objManagers;
};
#endif  // SIPCONTROLLER_FACTORY_H_
