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

#include "IMutex.h"
#include "ServiceMutex.h"
#include "call/IMtcCallContext.h"
#include "call/state/MtcCallStateMachine.h"

PUBLIC
MtcCallStateMachine::MtcCallStateMachine(IN IMtcCallContext& objContext,
        IN IMtcCall::State eInitialState, IN std::unique_ptr<IMtcCallStateFactory> pStateFactory,
        IN IMtcCallStateWatcher* pTransitionWatcher) :
        m_objContext(objContext),
        m_pStateFactory(std::move(pStateFactory)),
        m_pTransitionWatcher(pTransitionWatcher),
        m_pCurrentState(nullptr),
        m_pStateTransitionLock(MutexService::GetMutexService()->CreateMutex())
{
    IMS_ASSERT(m_pStateFactory != nullptr);

    TransitToState(eInitialState);
}

PUBLIC
MtcCallStateMachine::~MtcCallStateMachine()
{
    MutexService::GetMutexService()->DestroyMutex(m_pStateTransitionLock);
}

PUBLIC
void MtcCallStateMachine::RunStateOperation(
        IN std::function<IMtcCall::State(IMtcCallState*)> objOperation)
{
    IMtcCall::State eNextState = objOperation(m_pCurrentState.get());
    TransitToState(eNextState);
}

PUBLIC
IMtcCall::State MtcCallStateMachine::GetState() const
{
    return m_pCurrentState->GetStateName();
}

PRIVATE
void MtcCallStateMachine::TransitToState(IN IMtcCall::State eState)
{
    LockGuard objLock(m_pStateTransitionLock);

    if (m_pCurrentState && m_pCurrentState->GetStateName() == eState)
    {
        return;
    }

    std::unique_ptr<IMtcCallState> pNewState(m_pStateFactory->CreateState(eState, m_objContext));
    IMS_ASSERT(pNewState != nullptr);

    if (m_pCurrentState)
    {
        m_pCurrentState->OnExit();
    }
    m_pCurrentState = std::move(pNewState);
    m_pCurrentState->OnEnter();

    if (m_pTransitionWatcher)
    {
        m_pTransitionWatcher->OnStateTransition(eState);
    }
}
