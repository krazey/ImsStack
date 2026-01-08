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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "AsyncConfigHelper.h"
#include "IAsyncConfig.h"

__IMS_TRACE_TAG_CONF__;

class AsyncAction
{
public:
    inline AsyncAction(IN IAsyncConfig* piConfig, IN IMS_SINT32 nMsg, IN IMS_SINTP nParam1,
            IN IMS_SINTP nParam2) :
            m_piConfig(piConfig),
            m_nMsg(nMsg),
            m_nParam1(nParam1),
            m_nParam2(nParam2)
    {
    }

    inline ~AsyncAction() {}

    AsyncAction(IN const AsyncAction&) = delete;
    AsyncAction& operator=(IN const AsyncAction&) = delete;

public:
    IAsyncConfig* m_piConfig;
    IMS_SINT32 m_nMsg;
    IMS_SINTP m_nParam1;
    IMS_SINTP m_nParam2;
};

PUBLIC
AsyncConfigHelper::AsyncConfigHelper() :
        ImsActivityEx(),
        m_objAsyncConfigs(ImsList<IAsyncConfig*>())
{
}

PUBLIC VIRTUAL AsyncConfigHelper::~AsyncConfigHelper()
{
    m_objAsyncConfigs.Clear();
}

PUBLIC
void AsyncConfigHelper::Register(IN IAsyncConfig* piConfig)
{
    if (IsRegisteredConfig(piConfig))
    {
        return;
    }

    m_objAsyncConfigs.Append(piConfig);
}

PUBLIC
IMS_BOOL AsyncConfigHelper::SendTo(
        IN IAsyncConfig* piConfig, IN IMS_SINT32 nMsg, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2)
{
    if (!IsRegisteredConfig(piConfig))
    {
        IMS_TRACE_D("AsyncConfig is not registered ...", 0, 0, 0);
        return IMS_FALSE;
    }

    AsyncAction* pAction = new AsyncAction(piConfig, nMsg, nParam1, nParam2);

    if (pAction == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!PostMessage(AMSG_SEND_TO, 0, reinterpret_cast<IMS_UINTP>(pAction)))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void AsyncConfigHelper::Unregister(IN const IAsyncConfig* piConfig)
{
    for (IMS_UINT32 i = 0; i < m_objAsyncConfigs.GetSize(); ++i)
    {
        const IAsyncConfig* piTmpConfig = m_objAsyncConfigs.GetAt(i);

        if (piTmpConfig == piConfig)
        {
            m_objAsyncConfigs.RemoveAt(i);
            break;
        }
    }
}

PROTECTED VIRTUAL IMS_BOOL AsyncConfigHelper::OnMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_SEND_TO:
        {
            AsyncAction* pAction = reinterpret_cast<AsyncAction*>(objMsg.nLparam);

            if (pAction == IMS_NULL)
            {
                IMS_TRACE_D("No action in the message", 0, 0, 0);
                break;
            }

            if (!IsRegisteredConfig(pAction->m_piConfig))
            {
                IMS_TRACE_D("AsyncConfig is not registered, so message(%d) is dropped",
                        pAction->m_nMsg, 0, 0);

                delete pAction;
                break;
            }

            pAction->m_piConfig->HandleMessage(
                    pAction->m_nMsg, pAction->m_nParam1, pAction->m_nParam2);

            delete pAction;
            break;
        }
        default:
            // no-op
            break;
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AsyncConfigHelper::IsRegisteredConfig(IN const IAsyncConfig* piConfig)
{
    for (IMS_UINT32 i = 0; i < m_objAsyncConfigs.GetSize(); ++i)
    {
        const IAsyncConfig* piTmpConfig = m_objAsyncConfigs.GetAt(i);

        if (piTmpConfig == piConfig)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
