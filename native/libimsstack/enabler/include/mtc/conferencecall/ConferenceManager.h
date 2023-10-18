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

#ifndef CONFERENCE_MANAGER_H_
#define CONFERENCE_MANAGER_H_

#include "ImsMap.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCallManager.h"
#include "conferencecall/ConferenceController.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceFactory.h"
#include "conferencecall/IConferenceControllerListener.h"
#include "conferencecall/IConferenceManager.h"
#include "helper/ObjectAsyncDestroyer.h"

class IMtcContext;
enum class ConferenceType;

class ConferenceManager final : public IConferenceControllerListener, public IConferenceManager
{
public:
    explicit ConferenceManager(IN IMtcContext& objContext);
    virtual ~ConferenceManager();
    ConferenceManager(IN const ConferenceManager&) = delete;
    ConferenceManager& operator=(IN const ConferenceManager&) = delete;

    // IConferenceControllerListener interface implementation
    void OnClosed(IN ConferenceController* pController) override;

    IConferenceController& CreateController(IN CallKey nCallKey, IN ConferenceType eType) override;
    IConferenceController* GetController(IN IMS_UINTP nCallKey) const override;

private:
    void ReleaseController(IN ConferenceController* pController);

private:
    IMtcContext& m_objContext;
    ConferenceFactory m_objConferenceFactory;
    ImsMap<CallKey, ConferenceController*> m_objControllers;
    ObjectAsyncDestroyer<ConferenceController> m_objDestroyer;
    CallConnectionIdManager& m_objCallConnectionIdManager;
};

#endif
