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
#include "ImsApp.h"

class IMtsService;
class MtsDynamicLoader;
class MtsMessageController;

class MtsApp final : public IMtsApp, public ImsApp
{
public:
    explicit MtsApp(IN IMS_SINT32 nSlotId);
    ~MtsApp();

    // IMtsApp
    virtual void Start() override;
    virtual void Stop() override;

    inline IMtsService* GetMtsService() { return m_piMtsService; }
    inline MtsDynamicLoader* GetMtsDynamicLoader() { return m_pMtsDynamicLoader; }
    inline MtsMessageController* GetMtsMessageController() { return m_pMtsMessageController; }

private:
    void CreateMtsMessageController();
    void CreateMtsService();
    void CreateMtsUtils();

private:
    IMS_SINT32 m_nSlotId;
    IMtsService* m_piMtsService;
    MtsDynamicLoader* m_pMtsDynamicLoader;
    MtsMessageController* m_pMtsMessageController;
};

#endif
