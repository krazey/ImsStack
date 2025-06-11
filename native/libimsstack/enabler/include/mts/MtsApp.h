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
#include "IMtsJni.h"
#include "ImsApp.h"
#include "MtsDef.h"
#include "MtsService.h"
#include "message/MtsMessageController.h"
#include "utility/MtsDynamicLoader.h"

class IJniMtsAppThread;
class IMtsDynamicLoader;
class IMtsMessageController;
class IMtsService;

class MtsApp final : public IMtsApp, public ImsApp, public IMtsContext, public IMtsJni
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
    const IMtsService& GetService(IN MtsServiceType eServiceType) const override;
    inline IMtsMessageController& GetMessageController() override
    {
        return m_objMtsMessageController;
    }
    inline const IMtsDynamicLoader& GetDynamicLoader() const override
    {
        return m_objMtsDynamicLoader;
    }
    IJniMtsAppThread* GetJniAppThread() const override;

    // IMtsJni
    inline void NotifyJniEnablerSet() override {}
    void SendMoSmsByServiceType(IN SmsFormatType eSmsFormat, IN ByteArray* pContent,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency,
            IN IMS_UINT32 nRetryCount) override;

private:
    void AttachJni();
    IMS_BOOL IsEmergencySmsOverImsSupported() const;

    IMS_SINT32 m_nSlotId;
    MtsService m_objNormalService;
    MtsService m_objEmergencyService;
    MtsMessageController m_objMtsMessageController;
    MtsDynamicLoader m_objMtsDynamicLoader;
};

#endif
