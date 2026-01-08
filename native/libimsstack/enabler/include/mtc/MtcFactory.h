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

#ifndef MTC_FACTORY_H_
#define MTC_FACTORY_H_

#include "ImsMap.h"
#include "ImsTypeDef.h"

class IMtcApp;
class IMutex;

class MtcFactory
{
public:
    MtcFactory();
    virtual ~MtcFactory();

    static MtcFactory* GetInstance();

    void Start(IN IMS_SINT32 nSlotId);
    void Stop(IN IMS_SINT32 nSlotId);

protected:
    // Visible for test.
    ImsMap<IMS_UINT32, IMtcApp*> m_objMtcApps;

private:
    IMutex* m_piLock;
};

#endif
