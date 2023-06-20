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
#ifndef ENABLER_LOADER_H_
#define ENABLER_LOADER_H_

#include "ImsMap.h"

#include "IEnablerLoader.h"

class ImsAppThread;
class EnablerFactory;
class EnablerThread;

class EnablerLoader : public IEnablerLoader
{
private:
    EnablerLoader();
    virtual ~EnablerLoader();

public:
    EnablerLoader(IN const EnablerLoader&) = delete;
    EnablerLoader& operator=(IN const EnablerLoader&) = delete;

public:
    void Init();
    void StartEnabler(IN IMS_SINT32 nSlotId) override;
    void StopEnabler(IN IMS_SINT32 nSlotId) override;

    static void CreateInstance();
    static void DestroyInstance();
    static EnablerLoader* GetInstance();

private:
    void CreateAndAddThread(IN IMS_SINT32 nSlotId);
    EnablerThread* GetEnablerThread(IN IMS_SINT32 nSlotId) const;

    static ImsAppThread* CreateThread(IN void* pvParam);

private:
    class EnablerThreadParam
    {
    public:
        inline EnablerThreadParam(IN EnablerFactory* pEnablerFactory, IN IMS_SINT32 nSlotId) :
                m_pEnablerFactory(pEnablerFactory),
                m_nSlotId(nSlotId)
        {
        }
        inline ~EnablerThreadParam() {}

    public:
        EnablerFactory* m_pEnablerFactory;
        IMS_SINT32 m_nSlotId;
    };

    static EnablerLoader* s_pEnablerLoader;

    EnablerFactory* m_pEnablerFactory;

    // <slot-id, enabler-thread>
    ImsMap<IMS_SINT32, EnablerThread*> m_objEnablerThreads;
};

#endif
