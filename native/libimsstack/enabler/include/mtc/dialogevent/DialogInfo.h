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

#ifndef DIALOG_INFO_H_
#define DIALOG_INFO_H_

#include "AString.h"
#include "ImsMap.h"
#include "MtcDef.h"

class Dialog;
class IElement;
struct JniExternalCall;

class DialogInfo
{
public:
    explicit DialogInfo();
    ~DialogInfo();
    DialogInfo(IN const DialogInfo&) = delete;
    DialogInfo& operator=(IN const DialogInfo&) = delete;

    IMS_RESULT Update(IN IElement* piElementDialogInfo);
    ImsList<JniExternalCall*> GetJniExternalCalls() const;

    inline IMS_UINT32 GetState() const { return m_nState; }
    inline IMS_UINT32 GetVersion() const { return m_nVersion; }
    inline AString& GetEntity() { return m_strEntity; }

    static IElement* GetSubElement(IN const IElement* piElement, IN const IMS_CHAR* pszElement);
    static AString& GetSubElementValue(IN const IElement* piElement, IN const IMS_CHAR* pszElement,
            OUT AString& strElementValue);
    static AString& GetElementValue(IN const IElement* piElement, OUT AString& strElementValue);
    static IMS_BOOL IsElementExist(IN const IElement* piElement, IN const IMS_CHAR* pszElement);
    static IMS_BOOL IsAttrExist(IN const IElement* piElement, IN const IMS_CHAR* pszAttrName);

private:
    IMS_UINT32 ConvertState(IN const AString& strState);
    void Clear();
    IMS_SLONG GetIndexOfKeyHasSameId(IN const AString& strDialogId);
    AString GetDialogId(Dialog* pDialog) const;
    AString GetDialogRemoteAddress(Dialog* pDialog) const;
    AString GetDialogLocalAddress(Dialog* pDialog) const;
    IMS_BOOL IsPullableDialog(Dialog* pDialog) const;
    IMS_UINT32 GetDialogCallState(Dialog* pDialog) const;
    IMS_UINT32 GetDialogCallType(Dialog* pDialog) const;
    IMS_BOOL IsHeldDialog(Dialog* pDialog) const;

public:
    enum
    {
        STATE_FULL = 1,
        STATE_PARTIAL = 2,
    };

private:
    ImsList<Dialog*> m_objDialogs;

    IMS_UINT32 m_nVersion;
    IMS_UINT32 m_nState;
    AString m_strEntity;
};

class Dialog final
{
public:
    class State final
    {
    public:
        explicit State() :
                m_nEvent(EVENT_IDLE),
                m_nCode(0),
                m_nState(STATE_IDLE)
        {
        }
        ~State(){};
        State(IN const State&) = delete;
        State& operator=(IN const State&) = delete;

        void Update(IN const IElement* piElementState);

    private:
        IMS_UINT32 ConvertDialogState(IN const AString& strState);
        IMS_UINT32 ConvertDialogStateEvent(IN const AString& strStateEvent);

    public:
        enum
        {
            STATE_IDLE = 0,
            STATE_TRYING = 1,
            STATE_PROCEEDING = 2,
            STATE_EARLY = 3,
            STATE_CONFIRMED = 4,
            STATE_TERMINATED = 5,
        };
        enum
        {
            EVENT_IDLE = 0,
            EVENT_CANCELLED = 1,
            EVENT_REJECTED = 2,
            EVENT_REPLACED = 3,
            EVENT_LOCAL_BYE = 4,
            EVENT_REMOTE_BYE = 5,
            EVENT_ERROR = 5,
            EVENT_TIMEOUT = 5,
        };

    private:
        friend class DialogInfo;

        IMS_UINT32 m_nEvent;
        IMS_UINT32 m_nCode;

        IMS_UINT32 m_nState;
    };

    class Replaces final
    {
    public:
        explicit Replaces() :
                m_strCallId(AString::ConstNull()),
                m_strLocalTag(AString::ConstNull()),
                m_strRemoteTag(AString::ConstNull())
        {
        }
        ~Replaces(){};
        Replaces(IN const Replaces&) = delete;
        Replaces& operator=(IN const Replaces&) = delete;

        void Update(IN const IElement* piElementReplaces);

    private:
        friend class DialogInfo;

        AString m_strCallId;
        AString m_strLocalTag;
        AString m_strRemoteTag;
    };

    class NameAddr final
    {
    public:
        explicit NameAddr() :
                m_strDisplay(AString::ConstNull()),
                m_strUri(AString::ConstNull())
        {
        }
        ~NameAddr(){};
        NameAddr(IN const NameAddr&) = delete;
        NameAddr& operator=(IN const NameAddr&) = delete;

        void Update(IN const IElement* piElementNameaddr);

    private:
        friend class DialogInfo;

        AString m_strDisplay;
        AString m_strUri;
    };

    class Target final
    {
    public:
        explicit Target() :
                m_objParamMap(ImsMap<AString, AString>()),
                m_strUri(AString::ConstNull())
        {
        }
        ~Target(){};
        Target(IN const Target&) = delete;
        Target& operator=(IN const Target&) = delete;

        void Update(IN const IElement* piElementTarget);

    private:
        friend class DialogInfo;

        ImsMap<AString, AString> m_objParamMap;
        AString m_strUri;
        // session-description
        // cseq
    };

    class Participant final
    {
    public:
        explicit Participant() :
                m_objIdentity(NameAddr()),
                m_objTarget(Target())
        {
        }
        ~Participant(){};
        Participant(IN const Participant&) = delete;
        Participant& operator=(IN const Participant&) = delete;

        void Update(IN const IElement* piElementParticipant);

    private:
        friend class DialogInfo;

        NameAddr m_objIdentity;
        Target m_objTarget;
        // session-description
        // cseq
    };

    class ExtraInfo final
    {
    public:
        explicit ExtraInfo() :
                m_strExclusive(AString::ConstNull()),
                m_objMediaInfo(MediaInfo())
        {
        }
        ~ExtraInfo(){};
        ExtraInfo(IN const ExtraInfo&) = delete;
        ExtraInfo& operator=(IN const ExtraInfo&) = delete;

        void Update(IN const IElement* piElementDialog);

    private:
        void HandleMediaInfo(IN const IElement* piElementDialog);
        IMS_SINT32 ConvertMediaDirection(IN const AString& strMediaDirection);

    private:
        friend class DialogInfo;

        AString m_strExclusive;
        MediaInfo m_objMediaInfo;
    };

public:
    explicit Dialog() :
            m_objState(State()),
            m_nDuration(0),
            m_objReplaces(Replaces()),
            m_objReferredBy(NameAddr()),
            m_objLocal(Participant()),
            m_objRemote(Participant()),
            m_objExtraInfo(ExtraInfo()),
            m_strId(AString::ConstNull()),
            m_strCallId(AString::ConstNull()),
            m_strLocalTag(AString::ConstNull()),
            m_strRemoteTag(AString::ConstNull()),
            m_nDirection(DIRECTION_IDLE)
    {
    }
    ~Dialog(){};
    Dialog(IN const Dialog&) = delete;
    Dialog& operator=(IN const Dialog&) = delete;

    IMS_RESULT Update(IN IElement* piElementDialog);

private:
    IMS_UINT32 ConvertDirection(IN const AString& strState);

public:
    enum
    {
        DIRECTION_IDLE = 0,
        DIRECTION_INITIATOR = 1,
        DIRECTION_RECIPIENT = 2,
    };

private:
    friend class DialogInfo;

    State m_objState;
    IMS_UINT32 m_nDuration;
    Replaces m_objReplaces;
    NameAddr m_objReferredBy;
    // route-set
    Participant m_objLocal;
    Participant m_objRemote;

    ExtraInfo m_objExtraInfo;

    AString m_strId;
    AString m_strCallId;
    AString m_strLocalTag;
    AString m_strRemoteTag;
    IMS_UINT32 m_nDirection;
};

#endif
