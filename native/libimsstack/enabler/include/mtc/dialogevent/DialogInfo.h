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

class Dialog;
class IElement;

class DialogInfo
{
public:
    explicit DialogInfo();
    ~DialogInfo();
    DialogInfo(IN const DialogInfo&) = delete;
    DialogInfo& operator=(IN const DialogInfo&) = delete;

    IMS_RESULT Update(IN IElement* piElementDialogInfo);

    inline IMS_UINT32 GetState() const { return m_nState; }
    inline IMS_UINT32 GetVersion() const { return m_nVersion; }
    inline AString& GetEntity() { return m_strEntity; }

    static AString& GetElementValue(IN const IElement* piElement, OUT AString& strElementValue);
    static IMS_BOOL IsMandatoryElementExist(
            IN const IElement* piElement, IN const IMS_CHAR* pszElement);
    static IMS_BOOL IsMandatoryAttrExist(IN const IElement* piElement, IN const IMS_CHAR* pszAttr);

private:
    IMS_UINT32 ConvertState(IN const AString& strState);
    void Clear();
    IMS_SLONG GetIndexOfKeyHasSameId(IN const AString& strDialogId);

public:
    enum
    {
        STATE_FULL = 1,
        STATE_PARTIAL = 2,
    };

    static const IMS_CHAR ELEMENT_DIALOG_INFO[];
    static const IMS_CHAR ATTR_DIALOG_INFO_VERSION[];
    static const IMS_CHAR ATTR_DIALOG_INFO_STATE[];
    static const IMS_CHAR ATTR_DIALOG_INFO_ENTITY[];
    static const IMS_CHAR ELEMENT_DIALOG[];
    static const IMS_CHAR ATTR_DIALOG_ID[];
    static const IMS_CHAR ATTR_DIALOG_CALL_ID[];
    static const IMS_CHAR ATTR_DIALOG_LOCAL_TAG[];
    static const IMS_CHAR ATTR_DIALOG_REMOTE_TAG[];
    static const IMS_CHAR ATTR_DIALOG_DIRECTION[];
    static const IMS_CHAR ELEMENT_STATE[];
    static const IMS_CHAR ATTR_STATE_EVENT[];
    static const IMS_CHAR ATTR_STATE_CDOE[];
    static const IMS_CHAR ELEMENT_DURATIOIN[];
    static const IMS_CHAR ELEMENT_REPLACES[];
    static const IMS_CHAR ATTR_REPLACES_CALL_ID[];
    static const IMS_CHAR ATTR_REPLACES_LOCAL_TAG[];
    static const IMS_CHAR ATTR_REPLACES_REMOTE_TAG[];
    static const IMS_CHAR ELEMENT_REFERRED_BY[];
    static const IMS_CHAR ATTR_NAMEADDR_DISPLAY_NAME[];
    static const IMS_CHAR ELEMENT_LOCAL[];
    static const IMS_CHAR ELEMENT_REMOTE[];
    static const IMS_CHAR ELEMENT_IDENTITY[];
    static const IMS_CHAR ELEMENT_TARGET[];
    static const IMS_CHAR ATTR_TARGET_URI[];
    static const IMS_CHAR ELEMENT_PARAM[];
    static const IMS_CHAR ATTR_PARAM_PNAME[];
    static const IMS_CHAR ATTR_PARAM_PVAL[];

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
                m_strCode(0),
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

            STATE_ONHOLD = 6,
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
        IMS_UINT32 m_strCode;

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
                m_objTarget(ImsMap<AString, AString>()),
                m_strUri(AString::ConstNull())
        {
        }
        ~Target(){};
        Target(IN const Target&) = delete;
        Target& operator=(IN const Target&) = delete;

        IMS_RESULT Update(IN const IElement* piElementTarget);

    private:
        friend class DialogInfo;

        ImsMap<AString, AString> m_objTarget;
        AString m_strUri;
        // session-description
        // cseq
    };

    class Participant final
    {
    public:
        explicit Participant() :
                m_objIdentity(NameAddr()),
                m_objtartget(Target())
        {
        }
        ~Participant(){};
        Participant(IN const Participant&) = delete;
        Participant& operator=(IN const Participant&) = delete;

        void Update(IN const IElement* piElementParticipant);

    private:
        friend class DialogInfo;

        NameAddr m_objIdentity;
        Target m_objtartget;
        // session-description
        // cseq
    };

public:
    explicit Dialog() :
            m_objState(State()),
            m_nDuration(0),
            m_objReplaces(Replaces()),
            m_objReferredBy(NameAddr()),
            m_objLocal(Participant()),
            m_objRemote(Participant()),
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

    AString m_strId;
    AString m_strCallId;
    AString m_strLocalTag;
    AString m_strRemoteTag;
    IMS_UINT32 m_nDirection;
};

#endif
