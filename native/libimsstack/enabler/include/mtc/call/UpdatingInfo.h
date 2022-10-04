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

#ifndef UPDATING_INFO_H_
#define UPDATING_INFO_H_

#include "IMtcCall.h"
#include "MtcDef.h"
#include "call/IMtcCallContext.h"

class UpdatingInfo final
{
public:
    UpdatingInfo(IN IMtcCallContext& objContext);
    virtual ~UpdatingInfo();
    UpdatingInfo(IN const UpdatingInfo&) = delete;
    UpdatingInfo& operator=(IN const UpdatingInfo&) = delete;

public:
    inline CallType GetTargetCallType() const { return m_eTargetCallType; }
    inline MediaInfo& GetNegotiatedInfo() { return m_objNegotiatedInfo; }
    inline MediaInfo& GetModifyingInfo() { return m_objModifyingInfo; }
    inline MediaInfo& GetAlertingInfo() { return m_objAlertingInfo; }
    inline MediaInfo& GetModifiedInfo() { return m_objModifiedInfo; }
    inline IMS_BOOL IsModifier() { return m_bModifier; }
    inline IMS_BOOL IsAlerted() { return m_bAlerted; }
    inline void SetTargetCallType(IN CallType eCallType) { m_eTargetCallType = eCallType; }
    inline void SetModifier() { m_bModifier = IMS_TRUE; }
    inline void SetAlerted() { m_bAlerted = IMS_TRUE; }
    inline void SetPendingUpdate(IN IMS_BOOL bHasPendingUpdate)
    {
        // TODO: check if setting to false is required.
        m_bHasPendingUpdate = bHasPendingUpdate;
    }

    IMS_BOOL IsHeld();
    IMS_BOOL IsHeldBy();
    IMS_BOOL IsResumed();
    IMS_BOOL IsResumedBy();
    IMS_BOOL IsNeedToAlert();
    IMS_BOOL IsRequestedHoldResume();
    IMS_BOOL IsRequestedModifying();
    IMS_BOOL IsModified();
    inline IMS_BOOL HasPendingUpdate() { return m_bHasPendingUpdate; }

private:
    CallType GetCurrentCallType() const;

private:
    IMtcCallContext& m_objContext;
    CallType m_eTargetCallType;
    MediaInfo m_objNegotiatedInfo;
    MediaInfo m_objModifyingInfo;
    MediaInfo m_objAlertingInfo;
    MediaInfo m_objModifiedInfo;
    IMS_BOOL m_bModifier;
    IMS_BOOL m_bAlerted;
    IMS_BOOL m_bHasPendingUpdate;
};

#endif
