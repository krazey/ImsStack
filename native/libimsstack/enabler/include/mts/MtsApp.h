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
#include "IMtsContext.h"
#include "ImsApp.h"
#include "MtsService.h"
#include "message/MtsMessageController.h"
#include "utility/MtsDynamicLoader.h"

class IMtsDynamicLoader;
class IMtsMessageController;
class IMtsService;

class MtsApp final : public IMtsApp, public ImsApp, public IMtsContext
{
public:
    explicit MtsApp(IN IMS_SINT32 nSlotId);
    ~MtsApp();
    MtsApp(IN const MtsApp&) = delete;
    MtsApp& operator=(IN const MtsApp&) = delete;

    // IMtsApp
    virtual void Start() override;
    virtual void Stop() override;

    // IMtsContext
    inline IMS_SINT32 GetSlotId() const override { return m_nSlotId; }
    inline IMtsService& GetService() override { return m_objMtsService; }
    inline IMtsMessageController& GetMessageController() override
    {
        return m_objMtsMessageController;
    }
    inline const IMtsDynamicLoader& GetDynamicLoader() const override
    {
        return m_objMtsDynamicLoader;
    }

private:
    IMS_SINT32 m_nSlotId;
    MtsService m_objMtsService;
    MtsMessageController m_objMtsMessageController;
    MtsDynamicLoader m_objMtsDynamicLoader;
};

#endif
