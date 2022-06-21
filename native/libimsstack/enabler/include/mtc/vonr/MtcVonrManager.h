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

#ifndef MTC_VONR_MANAGER_H_
#define MTC_VONR_MANAGER_H_

#include "call/IMtcCall.h"
#include "vonr/IMtcVonrListener.h"
#include "vonr/IMtcVonrManager.h"
#include "vonr/MtcVonr.h"

class MtcVonrManager : public IMtcVonrListener, public IMtcVonrManager

{
public:
    explicit MtcVonrManager();
    virtual ~MtcVonrManager();
    MtcVonrManager(IN const MtcVonrManager&) = delete;
    MtcVonrManager& operator=(IN const MtcVonrManager&) = delete;

    // to check FEATURE_VONR
    static IMS_BOOL IsSupported();

    // to check call type / VoNR mode by runtime.
    IMS_BOOL IsUacRequired(IN IMS_BOOL bWifi) override;
    void CheckBarring(
            IN IMtcCall* piMtcCall, IN CallType eCallType, IN IMS_BOOL bEmergency) override;

    // IMtcVonrListener implementation
    void OnTerminated(IN MtcVonr* pMtcVonr) override;
    IMS_BOOL IsOtherSessionAlive(IN MtcVonr* pMtcVonr, IN IMS_UINT32 nUacType) override;
    void SetInitiateType(IN MtcVonr::VonrInitType eType) override;
    MtcVonr::VonrInitType GetTotalInitiateType() override;

private:
    void Initialize();
    MtcVonr* CreateMtcVonr();
    void DestroyMtcVonr(IN MtcVonr* pMtcVonr);

private:
    IMSList<MtcVonr*> m_lstMtcVonrs;
    MtcVonr::VonrInitType m_eTotalInitiateType;
    IMS_BOOL m_bMtk;
};

#endif
