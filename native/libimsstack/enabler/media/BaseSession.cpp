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
        m_eEnforcedDirection(MEDIA_DIRECTION_INVALID)
{
}

PUBLIC VIRTUAL BaseSession::~BaseSession() {}

PUBLIC VIRTUAL void BaseSession::SetNegoId(IMS_UINTP nNegoId)
{
    m_listNegoId.Append(nNegoId);
}

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
    //    IMS_TRACE_D("BaseSession::SetDirection() - [%d]->[%d]", m_eEnforcedDirection, eDir, 0);
    // To Do, Media build
    m_eEnforcedDirection = eDir;
}

PUBLIC VIRTUAL IMS_BOOL BaseSession::IsSameNegoId(IMS_UINTP nNegoId)
{
    IMS_BOOL bRet = IMS_FALSE;

    // check nego id
    for (IMS_UINT32 i = 0; i < m_listNegoId.GetSize(); i++)
    {
        if (nNegoId == m_listNegoId.GetAt(i))
        {
            bRet = IMS_TRUE;
            break;
        }
    }
    return bRet;
}

// do it later : IsSameRemoteNetwork is currently not used
/*
PUBLIC VIRTUAL IMS_BOOL IsSameRemoteNetwork(IPAddress address, IMS_UINT32 port) {
    (void)address;
    (void)port;
    return IMS_TRUE;
}
*/
