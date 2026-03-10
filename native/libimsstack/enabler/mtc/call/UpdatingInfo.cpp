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

#include "MtcDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/MtcCallStringUtils.h"
#include "call/UpdatingInfo.h"
#include "media/MtcMediaManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UpdatingInfo::UpdatingInfo(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_eOriginalCallType(GetCurrentCallType()),
        m_eTargetCallType(CallType::UNKNOWN),
        m_eRequestingType(UpdateType::NORMAL),
        m_objOriginalInfo(MediaInfo()),
        m_objModifyingInfo(MediaInfo()),
        m_objAlertingInfo(MediaInfo()),
        m_objModifiedInfo(MediaInfo()),
        m_bModifier(IMS_FALSE),
        m_bAlerted(IMS_FALSE),
        m_bHasPendingUpdate(IMS_FALSE)
{
    IMS_TRACE_D("+UpdatingInfo CallType[%s]",
            MtcCallStringUtils::ConvertCallType(m_eOriginalCallType), 0, 0);
}

PUBLIC VIRTUAL UpdatingInfo::~UpdatingInfo()
{
    IMS_TRACE_D("~UpdatingInfo", 0, 0, 0);
}

PUBLIC void UpdatingInfo::UpdateRequestingTypeForIncomingUpdate()
{
    if (IsNeedToAlert())
    {
        m_eRequestingType = UpdateType::SESSION;
    }
    else if (IsHeldBy())
    {
        m_eRequestingType = UpdateType::HOLD;
    }
    else if (IsResumedBy())
    {
        m_eRequestingType = UpdateType::RESUME;
    }
    else
    {
        m_eRequestingType = UpdateType::NORMAL;
    }
    IMS_TRACE_I(
            "Requesting type [%s]", MtcCallStringUtils::ConvertUpdateType(m_eRequestingType), 0, 0);
}

PUBLIC void UpdatingInfo::UpdateRequestingTypeForOfferlessReInvite()
{
    if (IsRequestedModifying())
    {
        m_eRequestingType = UpdateType::SESSION;
    }
    else if (IsHeld())
    {
        m_eRequestingType = UpdateType::HOLD;
    }
    else if (IsResumed())
    {
        m_eRequestingType = UpdateType::RESUME;
    }
    else
    {
        m_eRequestingType = UpdateType::NORMAL;
    }
    IMS_TRACE_I(
            "Requesting type [%s]", MtcCallStringUtils::ConvertUpdateType(m_eRequestingType), 0, 0);
}

PUBLIC
IMS_BOOL UpdatingInfo::IsHeld() const
{
    if (m_objOriginalInfo.eAudioDirection == DIRECTION_SEND_RECEIVE)
    {
        if (m_objModifyingInfo.eAudioDirection == DIRECTION_SEND ||
                m_objModifyingInfo.eAudioDirection == DIRECTION_INACTIVE)
        {
            return IMS_TRUE;
        }
    }
    else if (m_objOriginalInfo.eAudioDirection == DIRECTION_RECEIVE &&
            m_objModifyingInfo.eAudioDirection == DIRECTION_INACTIVE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsHeldBy() const
{
    IMS_SINT32 eNewAudioDirection = m_objModifiedInfo.eAudioDirection;
    if (eNewAudioDirection == DIRECTION_INVALID)
    {
        eNewAudioDirection = m_objAlertingInfo.eAudioDirection;
    }

    if (m_objOriginalInfo.eAudioDirection == DIRECTION_SEND_RECEIVE)
    {
        if (eNewAudioDirection == DIRECTION_RECEIVE ||
                (eNewAudioDirection == DIRECTION_INACTIVE &&
                        m_objModifyingInfo.eAudioDirection != DIRECTION_INACTIVE))
        {
            return IMS_TRUE;
        }
    }
    else if (m_objOriginalInfo.eAudioDirection == DIRECTION_SEND &&
            eNewAudioDirection == DIRECTION_INACTIVE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsResumed() const
{
    if (m_objOriginalInfo.eAudioDirection == DIRECTION_SEND &&
            m_objModifyingInfo.eAudioDirection == DIRECTION_SEND_RECEIVE)
    {
        return IMS_TRUE;
    }
    else if (m_objOriginalInfo.eAudioDirection == DIRECTION_INACTIVE)
    {
        if (m_objModifyingInfo.eAudioDirection == DIRECTION_RECEIVE ||
                m_objModifyingInfo.eAudioDirection == DIRECTION_SEND_RECEIVE)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsResumedBy() const
{
    IMS_SINT32 eNewAudioDirection = m_objModifiedInfo.eAudioDirection;
    if (eNewAudioDirection == DIRECTION_INVALID)
    {
        eNewAudioDirection = m_objAlertingInfo.eAudioDirection;
    }

    if (m_objOriginalInfo.eAudioDirection == DIRECTION_RECEIVE &&
            eNewAudioDirection == DIRECTION_SEND_RECEIVE)
    {
        return IMS_TRUE;
    }
    else if (m_objOriginalInfo.eAudioDirection == DIRECTION_INACTIVE)
    {
        if (eNewAudioDirection == DIRECTION_SEND ||
                (eNewAudioDirection == DIRECTION_SEND_RECEIVE &&
                        m_objModifyingInfo.eAudioDirection != DIRECTION_SEND_RECEIVE))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsNeedToAlert() const
{
    if (IsModified())
    {
        return IMS_TRUE;
    }

    if (IsHeldBy() || IsResumedBy())
    {
        return IMS_FALSE;
    }

    if (m_objOriginalInfo.eVideoDirection != m_objAlertingInfo.eVideoDirection)
    {
        if (m_eOriginalCallType == CallType::VT || m_eOriginalCallType == CallType::VIDEO_RTT)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsRequestedHoldResume() const
{
    if (m_objModifyingInfo.eAudioDirection == DIRECTION_INVALID)
    {
        return IMS_FALSE;
    }

    if (m_objOriginalInfo.eAudioDirection == m_objModifyingInfo.eAudioDirection)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsRequestedModifying() const
{
    if (m_eOriginalCallType != GetTargetCallType())
    {
        return IMS_TRUE;
    }

    if (!IsRequestedHoldResume() &&
            m_objOriginalInfo.eVideoDirection != m_objModifyingInfo.eVideoDirection)
    {
        return IMS_TRUE;  // 1-way <-> 2-way video
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsModified() const
{
    ISession& objSession = m_objContext.GetSession()->GetISession();
    return m_eOriginalCallType != m_objContext.GetMediaManager().GetNegotiatedCallType(&objSession);
}

PUBLIC
IMS_BOOL UpdatingInfo::IsDowngraded() const
{
    CallType eModifyingCallType = m_objContext.GetMediaManager().GetNegotiatedCallType(
            &m_objContext.GetSession()->GetISession());
    if (m_eOriginalCallType == eModifyingCallType)
    {
        return IMS_FALSE;
    }

    if (eModifyingCallType == CallType::VOIP)
    {
        return IMS_TRUE;
    }

    return m_eOriginalCallType == CallType::VIDEO_RTT;
}

PUBLIC
MediaInfo UpdatingInfo::GetModifiedMediaInfoWithOriginalAudioDir() const
{
    MediaInfo objInfoForUpdatedNotifying(m_objModifiedInfo);
    // Don't need to care about video and text direction because the ISIL relies on
    // upper layer(or directly sets in the specific cases) for video, text direction.
    objInfoForUpdatedNotifying.eAudioDirection = m_objOriginalInfo.eAudioDirection;

    return objInfoForUpdatedNotifying;
}

PUBLIC GLOBAL IMS_BOOL UpdatingInfo::IsValidHoldDirection(
        IN IMS_SINT32 eCurrentAudioDir, IN IMS_SINT32 eTargetAudioDir)
{
    if ((eTargetAudioDir == DIRECTION_SEND && eCurrentAudioDir == DIRECTION_RECEIVE) ||
            (eTargetAudioDir == DIRECTION_INACTIVE && eCurrentAudioDir == DIRECTION_SEND_RECEIVE))
    {
        IMS_TRACE_E(0, "hold race condition happened", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL UpdatingInfo::IsValidResumeDirection(
        IN IMS_SINT32 eCurrentAudioDir, IN IMS_SINT32 eTargetAudioDir)
{
    // If the AP IMS has a conventional unhold feature(b/364186357) in the future,
    // A first condition below has to be modified.
    if ((eTargetAudioDir == DIRECTION_SEND_RECEIVE && eCurrentAudioDir == DIRECTION_INACTIVE) ||
            (eTargetAudioDir == DIRECTION_RECEIVE && eCurrentAudioDir == DIRECTION_SEND))
    {
        IMS_TRACE_E(0, "resume race condition happened", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
CallType UpdatingInfo::GetCurrentCallType() const
{
    const IMtcSession* pSession = m_objContext.GetSession();
    if (!pSession)
    {
        return CallType::UNKNOWN;
    }

    return pSession->GetCallType();
}
