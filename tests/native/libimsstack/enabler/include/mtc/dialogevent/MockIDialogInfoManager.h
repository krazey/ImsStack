/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef MOCK_I_DIALOG_INFO_MANAGER_H_
#define MOCK_I_DIALOG_INFO_MANAGER_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "dialogevent/DialogInfo.h"
#include "dialogevent/IDialogInfoManager.h"
#include <gmock/gmock.h>

class AString;

class MockIDialogInfoManager : public IDialogInfoManager
{
public:
    virtual ~MockIDialogInfoManager() {}

    MOCK_METHOD(IMS_RESULT, Update, (IN const AString&), (override));
    MOCK_METHOD(const ImsList<Dialog*>&, GetDialogs, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetState, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetVersion, (), (const, override));
    MOCK_METHOD(const AString&, GetEntity, (), (const, override));
};

class MockState : public Dialog::State
{
public:
    explicit MockState(IN IMS_UINT32 nEvent, IN IMS_UINT32 nCode, IN IMS_UINT32 nState) :
            Dialog::State()
    {
        m_nEvent = nEvent;
        m_nCode = nCode;
        m_nState = nState;
    }
    ~MockState() {}
};

class MockNameAddr : public Dialog::NameAddr
{
public:
    explicit MockNameAddr(IN const AString& strDisplay, IN const AString& strUri) :
            Dialog::NameAddr()
    {
        m_strDisplay = strDisplay;
        m_strUri = strUri;
    }
    ~MockNameAddr() {}
};

class MockTarget : public Dialog::Target
{
public:
    explicit MockTarget(IN const ImsMap<AString, AString>& objParamMap, IN const AString& strUri) :
            Dialog::Target()
    {
        m_objParamMap = objParamMap;
        m_strUri = strUri;
    }
    ~MockTarget() {}
};

class MockParticipant : public Dialog::Participant
{
public:
    explicit MockParticipant(
            IN const Dialog::NameAddr& objIdentity, IN const Dialog::Target& objTarget) :
            Dialog::Participant()
    {
        m_objIdentity = objIdentity;
        m_objTarget = objTarget;
    }
    ~MockParticipant() {}
};

class MockExtraInfo : public Dialog::ExtraInfo
{
public:
    explicit MockExtraInfo(IN const AString& strExclusive, IN const MediaInfo& objMediaInfo) :
            Dialog::ExtraInfo()
    {
        m_strExclusive = strExclusive;
        m_objMediaInfo = objMediaInfo;
    }
    ~MockExtraInfo() {}
};

class MockDialog : public Dialog
{
public:
    MockDialog() :
            Dialog()
    {
    }

    ~MockDialog() {}
    inline void SetRemoteUri(IN const AString& strUri)
    {
        m_objRemote = MockParticipant(
                MockNameAddr("", strUri), MockTarget(ImsMap<AString, AString>(), ""));
    }
    inline void SetLocalUri(IN const AString& strUri)
    {
        m_objLocal = MockParticipant(
                MockNameAddr("", strUri), MockTarget(ImsMap<AString, AString>(), ""));
    }
    inline void SetLocalUri(IN const AString& strUri, IN const Target& objTarget)
    {
        m_objLocal = MockParticipant(MockNameAddr("", strUri), objTarget);
    }
    inline void SetDialogInfo(IN const AString& strId, IN const AString& strCallId)
    {
        m_strId = strId;
        m_strCallId = strCallId;
    }
    inline void SetExtraInfo(IN const ExtraInfo& objExtraInfo) { m_objExtraInfo = objExtraInfo; }
    inline void SetState(IN const State& objState) { m_objState = objState; }
};

#endif
