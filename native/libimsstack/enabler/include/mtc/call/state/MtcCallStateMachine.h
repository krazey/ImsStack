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

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/state/IMtcCallState.h"
#include <functional>
#include <memory>

class IMutex;
class IMtcCallContext;
class IMtcCallStateFactory;
class IMtcCallStateWatcher;

class MtcCallStateMachine final
{
public:
    explicit MtcCallStateMachine(IN IMtcCallContext& objContext, IN IMtcCall::State eInitialState,
            IN std::unique_ptr<IMtcCallStateFactory> pStateFactory,
            IN IMtcCallStateWatcher* pTransitionWatcher = IMS_NULL);
    ~MtcCallStateMachine();

    /**
     * Runs given function with the current state instance.
     * And it transfers state to the returned state, initiating `OnExit()` and `OnEnter()` of the
     * states. Transition won't be happened if the states are same as before.
     *
     * @param objOperation Function to run. The current state instance is passed to the parameter.
     *                     It should returns the next state name.
     */
    void RunStateOperation(IN const std::function<IMtcCall::State(IMtcCallState*)>& objOperation);

    /**
     * Returns the current state name.
     *
     * @return Name of the current state.
     */
    IMtcCall::State GetState() const;

private:
    IMtcCallContext& m_objContext;

    std::unique_ptr<IMtcCallStateFactory> m_pStateFactory;
    IMtcCallStateWatcher* m_pTransitionWatcher;

    std::unique_ptr<IMtcCallState> m_pCurrentState;
    IMutex* m_pStateTransitionLock;

    void TransitToState(IN IMtcCall::State eState);
};

class IMtcCallStateFactory
{
public:
    virtual ~IMtcCallStateFactory(){};

    /**
     * Creates new state instance corresponding to given `eState`.
     *
     * @param eState State name to create.
     * @param objContext Call context for new state instance.
     * @return New state instance. It mustn't be null.
     */
    virtual IMtcCallState* CreateState(
            IN IMtcCall::State eState, IN IMtcCallContext& objContext) = 0;
};

class IMtcCallStateWatcher
{
public:
    virtual ~IMtcCallStateWatcher(){};

    /**
     * Notifies when transition to another state happens.
     *
     * @param eState Transited state name.
     */
    virtual void OnStateTransition(IN IMtcCall::State eState) = 0;
};

#endif
