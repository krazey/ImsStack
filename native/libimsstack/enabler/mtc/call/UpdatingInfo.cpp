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
#include "call/IMtcSession.h"
#include "call/UpdatingInfo.h"
#include "media/MtcMediaManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UpdatingInfo::UpdatingInfo(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_eTargetCallType(CallType::UNKNOWN),
        m_eRequestingType(UpdateType::NORMAL),
        m_objNegotiatedInfo(MediaInfo()),
        m_objModifyingInfo(MediaInfo()),
        m_objAlertingInfo(MediaInfo()),
        m_objModifiedInfo(MediaInfo()),
        m_bModifier(IMS_FALSE),
        m_bAlerted(IMS_FALSE),
        m_bHasPendingUpdate(IMS_FALSE)
{
    IMS_TRACE_D("+UpdatingInfo", 0, 0, 0);
}

PUBLIC VIRTUAL UpdatingInfo::~UpdatingInfo()
{
    IMS_TRACE_D("~UpdatingInfo", 0, 0, 0);
}

PUBLIC
IMS_BOOL UpdatingInfo::IsHeld() const
{
    if (m_objNegotiatedInfo.eAudioDirection == DIRECTION_SEND_RECEIVE)
    {
        if (m_objModifyingInfo.eAudioDirection == DIRECTION_SEND ||
                m_objModifyingInfo.eAudioDirection == DIRECTION_INACTIVE)
        {
            return IMS_TRUE;
        }
    }
    else if (m_objNegotiatedInfo.eAudioDirection == DIRECTION_RECEIVE &&
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

    if (m_objNegotiatedInfo.eAudioDirection == DIRECTION_SEND_RECEIVE)
    {
        if (eNewAudioDirection == DIRECTION_RECEIVE ||
                (eNewAudioDirection == DIRECTION_INACTIVE &&
                        m_objModifyingInfo.eAudioDirection != DIRECTION_INACTIVE))
        {
            return IMS_TRUE;
        }
    }
    else if (m_objNegotiatedInfo.eAudioDirection == DIRECTION_SEND &&
            eNewAudioDirection == DIRECTION_INACTIVE)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsResumed() const
{
    if (m_objNegotiatedInfo.eAudioDirection == DIRECTION_SEND &&
            m_objModifyingInfo.eAudioDirection == DIRECTION_SEND_RECEIVE)
    {
        return IMS_TRUE;
    }
    else if (m_objNegotiatedInfo.eAudioDirection == DIRECTION_INACTIVE)
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

    if (m_objNegotiatedInfo.eAudioDirection == DIRECTION_RECEIVE &&
            eNewAudioDirection == DIRECTION_SEND_RECEIVE)
    {
        return IMS_TRUE;
    }
    else if (m_objNegotiatedInfo.eAudioDirection == DIRECTION_INACTIVE)
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

    if (m_objNegotiatedInfo.eVideoDirection != m_objAlertingInfo.eVideoDirection)
    {
        CallType eCurrentType = GetCurrentCallType();
        if (eCurrentType == CallType::VT || eCurrentType == CallType::VIDEO_RTT)
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

    if (m_objNegotiatedInfo.eAudioDirection == m_objModifyingInfo.eAudioDirection)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsRequestedModifying() const
{
    if (IsRequestedHoldResume())
    {
        return IMS_FALSE;
    }

    if (GetCurrentCallType() != GetTargetCallType())
    {
        return IMS_TRUE;
    }

    if (m_objNegotiatedInfo.eVideoDirection != m_objModifyingInfo.eVideoDirection)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsModified() const
{
    ISession& objSession = m_objContext.GetSession()->GetISession();
    return GetCurrentCallType() !=
            m_objContext.GetMediaManager().GetNegotiatedCallType(&objSession);
}

PUBLIC
IMS_BOOL UpdatingInfo::IsDowngraded() const
{
    CallType eOriginalCallType = GetCurrentCallType();
    CallType eModifyingCallType = m_objContext.GetMediaManager().GetNegotiatedCallType(
            &m_objContext.GetSession()->GetISession());
    if (eOriginalCallType == eModifyingCallType)
    {
        return IMS_FALSE;
    }

    if (eModifyingCallType == CallType::VOIP)
    {
        return IMS_TRUE;
    }

    return eOriginalCallType == CallType::VIDEO_RTT;
}

PUBLIC
MediaInfo UpdatingInfo::GetModifiedMediaInfoWithOriginalAudioDir() const
{
    MediaInfo objInfoForUpdatedNotiying(m_objModifiedInfo);
    // Don't need to care about video and text direction because the ISIL relies on
    // upper layer(or directly sets in the specific cases) for video, text direction.
    objInfoForUpdatedNotiying.eAudioDirection = m_objNegotiatedInfo.eAudioDirection;

    return objInfoForUpdatedNotiying;
}

PRIVATE
CallType UpdatingInfo::GetCurrentCallType() const
{
    IMtcSession* pSession = m_objContext.GetSession();
    if (!pSession)
    {
        return CallType::UNKNOWN;
    }

    return pSession->GetPreviousCallType();
}
