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

#include "ServiceTrace.h"
#include "call/IMtcSession.h"
#include "call/UpdatingInfo.h"
#include "media/MtcMediaManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UpdatingInfo::UpdatingInfo(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_eTargetCallType(CallType::UNKNOWN),
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
IMS_BOOL UpdatingInfo::IsHeld()
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
IMS_BOOL UpdatingInfo::IsHeldBy()
{
    IMS_SINT32 eNewAudioDirection = m_objModifiedInfo.eAudioDirection;
    if (eNewAudioDirection == DIRECTION_INVALID)
    {
        eNewAudioDirection = m_objAlertingInfo.eAudioDirection;
    }

    if (m_objNegotiatedInfo.eAudioDirection == DIRECTION_SEND_RECEIVE)
    {
        if (eNewAudioDirection == DIRECTION_RECEIVE)
        {
            return IMS_TRUE;
        }
        else if (eNewAudioDirection == DIRECTION_INACTIVE &&
                m_objModifyingInfo.eAudioDirection != DIRECTION_INACTIVE)
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
IMS_BOOL UpdatingInfo::IsResumed()
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
IMS_BOOL UpdatingInfo::IsResumedBy()
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
        if (eNewAudioDirection == DIRECTION_SEND)
        {
            return IMS_TRUE;
        }
        else if (eNewAudioDirection == DIRECTION_SEND_RECEIVE &&
                m_objModifyingInfo.eAudioDirection != DIRECTION_SEND_RECEIVE)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsNeedToAlert()
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
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL UpdatingInfo::IsRequestedHoldResume()
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
IMS_BOOL UpdatingInfo::IsRequestedModifying()
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
IMS_BOOL UpdatingInfo::IsModified()
{
    ISession& objSession = m_objContext.GetSession()->GetISession();
    if (GetCurrentCallType() != m_objContext.GetMediaManager().GetNegotiatedCallType(&objSession))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
void UpdatingInfo::AdjustDirectionIfNeededForHoldOrResume(IN MediaInfo& objMediaInfo)
{
    if (objMediaInfo.eAudioDirection != DIRECTION_INVALID)
    {
        objMediaInfo.eAudioDirection = m_objAlertingInfo.eAudioDirection;
    }

    if (objMediaInfo.eVideoDirection != DIRECTION_INVALID)
    {
        objMediaInfo.eVideoDirection = m_objAlertingInfo.eVideoDirection;
    }

    if (objMediaInfo.eTextDirection != DIRECTION_INVALID)
    {
        objMediaInfo.eTextDirection = m_objAlertingInfo.eTextDirection;
    }
}

PRIVATE
CallType UpdatingInfo::GetCurrentCallType() const
{
    IMtcSession* pSession = m_objContext.GetSession();

    if (!pSession)
    {
        return CallType::UNKNOWN;
    }

    return pSession->GetCallType();
}
