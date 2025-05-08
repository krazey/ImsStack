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
#include "call/block/LocationBlockRule.h"
#include "helper/MtcLocationRefresher.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
LocationBlockRule::LocationBlockRule(IN MtcLocationRefresher& objLocationRefresher) :
        m_objLocationRefresher(objLocationRefresher),
        m_pListener(IMS_NULL)
{
}

PUBLIC VIRTUAL LocationBlockRule::~LocationBlockRule()
{
    m_objLocationRefresher.ResetListener();
}

PUBLIC VIRTUAL LocationBlockRule::Result LocationBlockRule::Check(
        IN IMtcBlockRuleCheckListener& objListener)
{
    // It assumes that a update request have been made by the LocationBlockRule's owner if it's
    // required.
    if (m_objLocationRefresher.GetState() != MtcLocationRefresher::LocationRefreshState::REFRESHING)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Still refreshing the location", 0, 0, 0);
    m_objLocationRefresher.SetListener(*this);
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
