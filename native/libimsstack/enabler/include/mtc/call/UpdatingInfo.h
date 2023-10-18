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

#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"

class UpdatingInfo final
{
public:
    explicit UpdatingInfo(IN IMtcCallContext& objContext);
    virtual ~UpdatingInfo();
    UpdatingInfo(IN const UpdatingInfo&) = delete;
    UpdatingInfo& operator=(IN const UpdatingInfo&) = delete;

public:
    inline CallType GetTargetCallType() const { return m_eTargetCallType; }
    inline UpdateType GetRequestingType() const { return m_eRequestingType; }
    inline MediaInfo& GetNegotiatedInfo() { return m_objNegotiatedInfo; }
    inline MediaInfo& GetModifyingInfo() { return m_objModifyingInfo; }
    inline MediaInfo& GetAlertingInfo() { return m_objAlertingInfo; }
    inline MediaInfo& GetModifiedInfo() { return m_objModifiedInfo; }
    inline IMS_BOOL IsModifier() { return m_bModifier; }
    inline IMS_BOOL IsAlerted() { return m_bAlerted; }
    inline void SetTargetCallType(IN CallType eCallType) { m_eTargetCallType = eCallType; }
    inline void SetRequestingType(IN UpdateType eType) { m_eRequestingType = eType; }
    inline void SetModifier() { m_bModifier = IMS_TRUE; }
    inline void SetAlerted() { m_bAlerted = IMS_TRUE; }
    inline void SetPendingUpdate(IN IMS_BOOL bHasPendingUpdate)
    {
        // TODO: check if setting to false is required.
        m_bHasPendingUpdate = bHasPendingUpdate;
    }

    IMS_BOOL IsHeld() const;
    IMS_BOOL IsHeldBy() const;
    IMS_BOOL IsResumed() const;
    IMS_BOOL IsResumedBy() const;
    IMS_BOOL IsNeedToAlert() const;
    IMS_BOOL IsRequestedHoldResume() const;
    IMS_BOOL IsRequestedModifying() const;

    /**
     * @brief Checks whether the received Request is call modification or not.
     *        It compares the CallType before the Request and the CallType of this Request.
     *
     * @return IMS_TRUE if the CallTypes are different. IMS_FALSE otherwise.
     */
    IMS_BOOL IsModified() const;
    IMS_BOOL IsDowngraded() const;
    inline IMS_BOOL HasPendingUpdate() const { return m_bHasPendingUpdate; }
    void AdjustDirectionIfNeededForHoldOrResume(IN MediaInfo& objMediaInfo) const;

private:
    // This returns the original CallType before this update is successfully completed.
    CallType GetCurrentCallType() const;

private:
    IMtcCallContext& m_objContext;
    CallType m_eTargetCallType;
    UpdateType m_eRequestingType;
    MediaInfo m_objNegotiatedInfo;  // Info before starting update.
    MediaInfo m_objModifyingInfo;   // Info after sending update.
    MediaInfo m_objAlertingInfo;    // Info after receiving update.
    MediaInfo m_objModifiedInfo;    // Info after update completed.
    IMS_BOOL m_bModifier;
    IMS_BOOL m_bAlerted;
    IMS_BOOL m_bHasPendingUpdate;
};

#endif
