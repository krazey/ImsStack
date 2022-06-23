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

#ifndef SRVCC_EVENT_HANDLER_H_
#define SRVCC_EVENT_HANDLER_H_

#include "IMSTypeDef.h"
#include "ImsList.h"
#include "helper/ISrvccStateListener.h"

class IMtcContext;

class SrvccEventHandler final
{
public:
    SrvccEventHandler(IN IMtcContext& objContext);
    ~SrvccEventHandler();
    SrvccEventHandler(IN const SrvccEventHandler&) = delete;
    SrvccEventHandler& operator=(IN const SrvccEventHandler&) = delete;

    void AddListener(IN ISrvccStateListener* piListener);
    void RemoveListener(IN ISrvccStateListener* piListener);

    void UpdateSrvccState(IN SrvccState eState);

private:
    void NotifyListeners();
    void HandleCalls();

private:
    IMtcContext& m_objContext;
    SrvccState m_eState;
    IMSList<ISrvccStateListener*> m_objListeners;
};

#endif
