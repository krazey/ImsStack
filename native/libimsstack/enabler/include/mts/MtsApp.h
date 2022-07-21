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

#ifndef MTS_APP_H_
#define MTS_APP_H_

#include "IMtsApp.h"
#include "IMtsCallTrackerListener.h"
#include "IMtsMessageControllerListener.h"
#include "MtsServiceState.h"
#include "ImsApp.h"
#include "ImsService.h"
#include "utility/MtsDynamicLoader.h"

class IMtsCallTracker;
class MtsCallTracker;
class MtsDynamicLoader;
class MtsMessageController;
class MtsService;
class MtsServiceState;

class MtsApp :
        public ImsApp,
        public IMtsApp,
        public IMtsCallTrackerListener,
        public IMtsMessageControllerListener
{
public:
    MtsApp(IN IMS_SINT32 nSlotId);
    virtual ~MtsApp();

    // IMtsApp
    virtual void Start() override;
    virtual void Stop() override;

    // IMtsMessageControllerListener
    virtual void MtsMessageController_NoTransaction();

    // IMtsCallTrackerListener
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

    inline MtsService* GetMtsService() { return m_pMtsService; }
    inline MtsMessageController* GetMtsMessageController() { return m_pMtsMessageController; }
    inline MtsDynamicLoader* GetMtsDynamicLoader() { return m_pMtsDynamicLoader; }
    inline MtsServiceState* GetMtsServiceState() { return m_pMtsServiceState; }
    inline MtsCallTracker* GetMtsCallTracker() { return m_pCallTracker; }

private:
    void CreateMtsService();
    void CreateMtsMessageController();
    void CreateMtsUtils();
    void DestroyMtsUtils();
    void GetSmOverIpConfigInfo();

private:
    IMS_SINT32 m_nSlotId;
    MtsService* m_pMtsService;
    MtsMessageController* m_pMtsMessageController;
    MtsDynamicLoader* m_pMtsDynamicLoader;
    MtsServiceState* m_pMtsServiceState;
    MtsCallTracker* m_pCallTracker;
};

#endif
