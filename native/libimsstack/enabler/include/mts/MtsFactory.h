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

#ifndef MTS_FACTORY_H_
#define MTS_FACTORY_H_

#include "ImsMap.h"

class IMtsApp;
class MtsApp;

class MtsFactory final
{
public:
    MtsFactory();
    ~MtsFactory();

    static MtsFactory* GetInstance();

    void Destroy(IN IMS_SINT32 nSlotId);
    void StartMts(IN IMS_SINT32 nSlotId);
    void StopMts(IN IMS_SINT32 nSlotId);

    IMS_BOOL DestroyMtsApp(IN IMS_SINT32 nSlotId);
    MtsApp* GetMtsApp(IN IMS_SINT32 nSlotId);
    IMS_UINT32 GetMtsAppListSize();

private:
    IMtsApp* CreateMtsApp(IN IMS_SINT32 nSlotId);

private:
    ImsMap<IMS_SINT32, IMtsApp*> m_objMtsApp;
};

#endif
