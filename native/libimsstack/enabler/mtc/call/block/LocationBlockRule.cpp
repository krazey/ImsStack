/*
 * Copyright (C) 2025 The Android Open Source Project
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
#include "call/IMtcCallContext.h"
#include "call/block/LocationBlockRule.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcLocationRefresher.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
LocationBlockRule::LocationBlockRule(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_objLocationRefresher(objContext.GetLocationRefresher()),
        m_pListener(IMS_NULL)
{
}

PUBLIC VIRTUAL LocationBlockRule::~LocationBlockRule()
{
    m_objLocationRefresher.RemoveListener(*this);
}

PUBLIC VIRTUAL LocationBlockRule::Result LocationBlockRule::Check(
        IN IMtcBlockRuleCheckListener& objListener)
{
    if (!IsLocationUpdateRequired())
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (IsLocationUpdateCompleted())
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Still refreshing the location", 0, 0, 0);
    m_objLocationRefresher.AddListener(*this);
    m_pListener = &objListener;
    return Result(Result::Status::PENDING);
}

PUBLIC VIRTUAL void LocationBlockRule::LocationUpdate_OnCompleted()
{
    if (m_pListener != IMS_NULL)
    {
        m_pListener->OnBlockRuleChecked(Result(Result::Status::UNBLOCKED));
    }
}

PRIVATE
IMS_BOOL LocationBlockRule::IsLocationUpdateRequired() const
{
    return m_objContext.GetCallInfo().eEmergencyType != EmergencyType::NONE &&
            m_objContext.GetConfigurationProxy().GetInt(
                    ConfigEmergency::KEY_REFRESH_GEOLOCATION_TIMEOUT_MILLIS_INT) > 0;
}

PRIVATE
IMS_BOOL LocationBlockRule::IsLocationUpdateCompleted() const
{
    // It assumes that a update request have been made by EmergencyServiceManager if it's required.
    return m_objLocationRefresher.GetState() == MtcLocationRefresher::LocationRefreshState::IDLE;
}
