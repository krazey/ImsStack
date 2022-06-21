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

#ifndef MTC_CALL_STATE_MACHINE_H_
#define MTC_CALL_STATE_MACHINE_H_

#include "IMSTypeDef.h"
#include "IMutex.h"
#include "ServiceMutex.h"
#include <functional>

class IMutex;
template <typename State, typename StateName>
class IMtcCallStateFactory;
template <typename StateName>
class IMtcCallStateWatcher;

template <typename State, typename StateName>
class MtcCallStateMachine
{
public:
    explicit MtcCallStateMachine(IN StateName eInitialState,
            IN IMtcCallStateFactory<State, StateName>& objStateFactory,
            IN IMtcCallStateWatcher<StateName>* pTransitionWatcher = IMS_NULL);
    ~MtcCallStateMachine();

    /**
     * Runs given function with the current state instance.
     * And it transfers state to the returned state, initiating `OnExit()` and `OnEnter()` of the
     * states. Transition won't be happened if the states are same as before.
     *
     * @param objOperation Function to run. The current state instance is passed to the parameter.
     *                     It should returns the next state name.
     */
    void RunStateOperation(IN std::function<StateName(State*)> objOperation);

    /**
     * Returns the current state name.
     *
     * @return Name of the current state.
     */
    StateName GetState() const;

private:
    IMtcCallStateFactory<State, StateName>& m_objStateFactory;
    IMtcCallStateWatcher<StateName>* m_pTransitionWatcher;

    std::unique_ptr<State> m_pCurrentState;
    IMutex* m_pStateTransitionLock;

    void TransitToState(IN StateName eState);
};

template <typename State, typename StateName>
class IMtcCallStateFactory
{
public:
    ~IMtcCallStateFactory(){};

    /**
     * Creates new state instance corresponding to given `eState`.
     *
     * @param eState State name to create.
     * @return New state instance. It mustn't be null.
     */
    virtual State* CreateState(IN StateName eState) = 0;
};

template <typename StateName>
class IMtcCallStateWatcher
{
public:
    ~IMtcCallStateWatcher(){};

    /**
     * Notifies when transition to another state happens.
     *
     * @param eState Transited state name.
     */
    virtual void OnStateTransition(IN StateName eState) = 0;
};

PUBLIC
template <typename State, typename StateName>
MtcCallStateMachine<State, StateName>::MtcCallStateMachine(IN StateName eInitialState,
        IN IMtcCallStateFactory<State, StateName>& objStateFactory,
        IN IMtcCallStateWatcher<StateName>* pTransitionWatcher) :
        m_objStateFactory(objStateFactory),
        m_pTransitionWatcher(pTransitionWatcher),
        m_pCurrentState(nullptr),
        m_pStateTransitionLock(MutexService::GetMutexService()->CreateMutex())
{
    TransitToState(eInitialState);
}

PUBLIC
template <typename State, typename StateName>
MtcCallStateMachine<State, StateName>::~MtcCallStateMachine()
{
    MutexService::GetMutexService()->DestroyMutex(m_pStateTransitionLock);
}

PUBLIC
template <typename State, typename StateName>
void MtcCallStateMachine<State, StateName>::RunStateOperation(
        IN std::function<StateName(State*)> objOperation)
{
    StateName eNextState = objOperation(m_pCurrentState.get());
    TransitToState(eNextState);
}

PUBLIC
template <typename State, typename StateName>
StateName MtcCallStateMachine<State, StateName>::GetState() const
{
    return m_pCurrentState->GetStateName();
}

PRIVATE
template <typename State, typename StateName>
void MtcCallStateMachine<State, StateName>::TransitToState(IN StateName eState)
{
    LockGuard objLock(m_pStateTransitionLock);

    if (m_pCurrentState && m_pCurrentState->GetStateName() == eState)
    {
        return;
    }

    State* pNewState = m_objStateFactory.CreateState(eState);
    IMS_ASSERT(pNewState != IMS_NULL);

    if (m_pCurrentState)
    {
        m_pCurrentState->OnExit();
    }
    m_pCurrentState = std::unique_ptr<State>(pNewState);
    m_pCurrentState->OnEnter();

    if (m_pTransitionWatcher)
    {
        m_pTransitionWatcher->OnStateTransition(eState);
    }
}

#endif
