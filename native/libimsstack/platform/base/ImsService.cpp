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
#include "ImsService.h"
#include "ServiceMemory.h"

PUBLIC
ImsService::ImsService(IN const AString& strName) :
        ImsActivity(strName),
        m_nState(IMS_INVALID_STATE),
        m_nOldState(IMS_INVALID_STATE)
{
}

EMPTY_STATE_MAP(ImsService)

PROTECTED
IMS_BOOL ImsService::SetState(IN IMS_UINT32 nState)
{
    m_nOldState = m_nState;
    m_nState = nState;
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL ImsService::DispatchMessage(IN ImsMessage& objMsg)
{
    IMS_BOOL bResult = IMS_TRUE;

    (void)OnPreprocess(objMsg);

    if (!OnStateMsgProcess(objMsg))
    {
        bResult = OnMessage(objMsg);
    }

    (void)OnPostprocess(objMsg);

    return bResult;
}

PRIVATE
IMS_BOOL ImsService::OnStateMsgProcess(IN ImsMessage& objMsg)
{
    const StateMap* pStateMap = GetStateMap();
    IMS_UINT32 nStateIndex = 0;
    IMS_BOOL bStateFound = IMS_FALSE;

    while (pStateMap[nStateIndex].nState != IMS_INVALID_STATE)
    {
        if (pStateMap[nStateIndex].nState == m_nState)
        {
            bStateFound = IMS_TRUE;
            break;
        }
        nStateIndex++;
    }

    if (bStateFound == IMS_TRUE)
    {
        const StateMsgMap* pStateMsgMap = (pStateMap[nStateIndex].pfnGetStateMsgMap)();

        if (pStateMsgMap != IMS_NULL)
        {
            IMS_UINT32 nMsgIndex = 0;

            while (pStateMsgMap[nMsgIndex].nMsg != IMS_INVALID_MSG)
            {
                if (pStateMsgMap[nMsgIndex].nMsg == static_cast<IMS_UINT32>(objMsg.GetName()))
                {
                    if (pStateMsgMap[nMsgIndex].pfnStateMsgHandler != IMS_NULL)
                    {
                        return (this->*(pStateMsgMap[nMsgIndex].pfnStateMsgHandler))(objMsg);
                    }
                }
                nMsgIndex++;
            }
        }
    }

    return IMS_FALSE;
}
