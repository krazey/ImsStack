/**
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
#include "BaseSession.h"

PUBLIC
BaseSession::BaseSession(IN IMS_SINT32 nSlodId) :
        m_nSlodId(nSlodId),
        m_piMediaSessionListener(IMS_NULL),
        m_pEnvironment(IMS_NULL),
        m_eEnforcedDirection(MEDIA_DIRECTION_INVALID),
        m_nState(0)
{
}

PUBLIC VIRTUAL BaseSession::~BaseSession() {}

PUBLIC VIRTUAL void BaseSession::SetServiceType(MEDIA_SERVICE_TYPE eServiceType)
{
    m_pEnvironment->eServiceType = eServiceType;
}

PUBLIC VIRTUAL void BaseSession::SetMediaSessionListener(IN IMediaSessionListener* pListener)
{
    m_piMediaSessionListener = pListener;
}

PUBLIC VIRTUAL void BaseSession::SetMediaEnvironment(MediaEnvironment* pEnvironment)
{
    m_pEnvironment = pEnvironment;
}

PUBLIC VIRTUAL void BaseSession::SetDirection(MEDIA_DIRECTION eDir)
{
    m_eEnforcedDirection = eDir;
}

PUBLIC VIRTUAL IMS_SINT32 BaseSession::GetState()
{
    return m_nState;
}

PUBLIC VIRTUAL void BaseSession::SetState(IMS_SINT32 state)
{
    m_nState = state;
}
