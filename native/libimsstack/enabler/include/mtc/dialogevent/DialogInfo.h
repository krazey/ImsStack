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

/**
 * @brief Represents the top-level element in a dialog event package.
 *
 * This class parses and holds information from the <dialog-info> element
 * as defined in RFC 4235.
 */
class DialogInfo
{
public:
    DialogInfo();
    ~DialogInfo();
    DialogInfo(IN const DialogInfo&) = delete;
    DialogInfo& operator=(IN const DialogInfo&) = delete;

    /**
     * @brief Updates the DialogInfo object by parsing the <dialog-info> XML element.
     *
     * @param piElementDialogInfo A pointer to the IElement representing the <dialog-info> element.
     * @return IMS_SUCCESS if the update is successful, IMS_FAILURE otherwise.
     */
    IMS_RESULT Update(IN IElement* piElementDialogInfo);

    /**
     * @brief Gets the list of dialogs.
     *
     * @return A const reference to the list of Dialog pointers.
     */
    inline const ImsList<Dialog*>& GetDialogs() const { return m_objDialogs; }

    /**
     * @brief Gets the state of the dialog-info.
     *
     * @return The state, which can be STATE_FULL or STATE_PARTIAL.
     */
    inline IMS_UINT32 GetState() const { return m_nState; }

    /**
     * @brief Gets the version of the dialog-info.
     *
     * @return The version number.
     */
    inline IMS_UINT32 GetVersion() const { return m_nVersion; }

    /**
     * @brief Gets the entity URI that generated the dialog package.
     *
     * @return A const reference to the entity URI string.
     */
    inline const AString& GetEntity() const { return m_strEntity; }

    static IElement* GetSubElement(IN const IElement* piElement, IN const IMS_CHAR* pszElement);
    static AString& GetSubElementValue(IN const IElement* piElement, IN const IMS_CHAR* pszElement,
            OUT AString& strElementValue);
    static AString& GetElementValue(IN const IElement* piElement, OUT AString& strElementValue);
    static IMS_BOOL IsElementExist(IN const IElement* piElement, IN const IMS_CHAR* pszElement);
    static IMS_BOOL IsAttrExist(IN const IElement* piElement, IN const IMS_CHAR* pszAttrName);

private:
    static IMS_UINT32 ConvertState(IN const AString& strState);
    IMS_SLONG GetIndexOfKeyHasSameId(IN const AString& strDialogId);

public:
    /**
     * @brief Enum for the state of the dialog-info package.
     */
    enum
    {
        STATE_INVALID = 0,
        STATE_FULL = 1,
        STATE_PARTIAL = 2,
    };

private:
    ImsList<Dialog*> m_objDialogs;

    IMS_UINT32 m_nVersion;
    IMS_UINT32 m_nState;
    AString m_strEntity;
};

/**
 * @brief Represents a <dialog> element within a <dialog-info> package.
 *
 * This class holds the state and attributes of a single SIP dialog.
 */
class Dialog
{
public:
    /**
     * @brief Represents the <state> element within a <dialog> element.
     *
     * It provides information about the current state of the dialog.
     */
    class State
    {
    public:
        State() :
                m_nEvent(EVENT_IDLE),
                m_nCode(0),
                m_nState(STATE_IDLE)
        {
        }
        ~State(){};
        State(IN const State&) = delete;
        State& operator=(IN const State& objRhs)
        {
            if (this != &objRhs)
            {
                m_nEvent = objRhs.m_nEvent;
                m_nCode = objRhs.m_nCode;
                m_nState = objRhs.m_nState;
            }
            return *this;
        }

        /**
         * @brief Updates the State object by parsing the <state> XML element.
         *
         * @param piElementState A pointer to the IElement representing the <state> element.
         */
        void Update(IN const IElement* piElementState);

        /**
         * @brief Gets the event that caused the state change.
         *
         * @return The state event.
         */
        inline IMS_UINT32 GetEvent() const { return m_nEvent; }

        /**
         * @brief Gets the SIP status code associated with the state.
         *
         * @return The SIP status code.
         */
        inline IMS_UINT32 GetCode() const { return m_nCode; }

        /**
         * @brief Gets the current state of the dialog (e.g., trying, confirmed, terminated).
         *
         * @return The dialog state.
         */
        inline IMS_UINT32 GetState() const { return m_nState; }

    private:
        static IMS_UINT32 ConvertDialogState(IN const AString& strState);
        static IMS_UINT32 ConvertDialogStateEvent(IN const AString& strStateEvent);

    public:
        /** Enum for the dialog state. */
        enum
        {
            STATE_IDLE = 0,
            STATE_TRYING = 1,
            STATE_PROCEEDING = 2,
            STATE_EARLY = 3,
            STATE_CONFIRMED = 4,
            STATE_TERMINATED = 5,
        };

        /** Enum for the event that triggered a state change. */
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

    protected:
        IMS_UINT32 m_nEvent;
        IMS_UINT32 m_nCode;
        IMS_UINT32 m_nState;
    };

    /**
     * @brief Represents the <replaces> element within a <dialog> element.
     *
     * This is used for call replacement scenarios.
     */
    class Replaces final
    {
    public:
        Replaces() :
                m_strCallId(AString::ConstNull()),
                m_strLocalTag(AString::ConstNull()),
                m_strRemoteTag(AString::ConstNull())
        {
        }
        ~Replaces(){};
        Replaces(IN const Replaces&) = delete;
        Replaces& operator=(IN const Replaces&) = delete;

        /**
         * @brief Updates the Replaces object by parsing the <replaces> XML element.
         *
         * @param piElementReplaces A pointer to the IElement representing the <replaces> element.
         */
        void Update(IN const IElement* piElementReplaces);

        /**
         * @brief Gets the Call-ID of the dialog being replaced.
         *
         * @return A const reference to the Call-ID string.
         */
        inline const AString& GetCallId() const { return m_strCallId; }

        /**
         * @brief Gets the local-tag of the dialog being replaced.
         *
         * @return A const reference to the local-tag string.
         */
        inline const AString& GetLocalTag() const { return m_strLocalTag; }

        /**
         * @brief Gets the remote-tag of the dialog being replaced.
         *
         * @return A const reference to the remote-tag string.
         */
        inline const AString& GetRemoteTag() const { return m_strRemoteTag; }

    private:
        AString m_strCallId;
        AString m_strLocalTag;
        AString m_strRemoteTag;
    };

    /** Represents a name-address element (e.g., <identity>, <referred-by>). */
    class NameAddr
    {
    public:
        NameAddr() :
                m_strDisplay(AString::ConstNull()),
                m_strUri(AString::ConstNull())
        {
        }
        ~NameAddr(){};
        NameAddr(IN const NameAddr&) = delete;
        NameAddr& operator=(IN const NameAddr& objRhs)
        {
            if (this != &objRhs)
            {
                m_strDisplay = objRhs.m_strDisplay;
                m_strUri = objRhs.m_strUri;
            }
            return *this;
        }

        /**
         * @brief Updates the NameAddr object by parsing its corresponding XML element.
         *
         * @param piElementNameaddr A pointer to the IElement to parse.
         */
        void Update(IN const IElement* piElementNameaddr);

        /**
         * @brief Gets the display name.
         *
         * @return A const reference to the display name string.
         */
        inline const AString& GetDisplay() const { return m_strDisplay; }

        /**
         * @brief Gets the URI.
         *
         * @return A const reference to the URI string.
         */
        inline const AString& GetUri() const { return m_strUri; }

    protected:
        AString m_strDisplay;
        AString m_strUri;
    };

    /** Represents the <target> element within a <participant> element. */
    class Target
    {
    public:
        Target() :
                m_objParamMap(ImsMap<AString, AString>()),
                m_strUri(AString::ConstNull())
        {
        }
        ~Target(){};
        Target(IN const Target&) = delete;
        Target& operator=(IN const Target& objRhs)
        {
            if (this != &objRhs)
            {
                m_objParamMap = objRhs.m_objParamMap;
                m_strUri = objRhs.m_strUri;
            }
            return *this;
        }

        /**
         * @brief Updates the Target object by parsing the <target> XML element.
         *
         * @param piElementTarget A pointer to the IElement representing the <target> element.
         */
        void Update(IN const IElement* piElementTarget);

        /**
         * @brief Gets the parameters of the target.
         *
         * @return A const reference to the map of parameters.
         */
        inline const ImsMap<AString, AString>& GetParams() const { return m_objParamMap; }

        /** Gets the URI of the target. */
        inline const AString& GetUri() const { return m_strUri; }

    protected:
        ImsMap<AString, AString> m_objParamMap;
        AString m_strUri;
        // session-description
        // cseq
    };

    /** Represents a participant in the dialog (<local> or <remote>). */
    class Participant
    {
    public:
        Participant() :
                m_objIdentity(NameAddr()),
                m_objTarget(Target())
        {
        }
        ~Participant(){};
        Participant(IN const Participant&) = delete;
        Participant& operator=(IN const Participant& objRhs)
        {
            if (this != &objRhs)
            {
                m_objIdentity = objRhs.m_objIdentity;
                m_objTarget = objRhs.m_objTarget;
            }
            return *this;
        }

        /**
         * @brief Updates the Participant object by parsing its corresponding XML element.
         *
         * @param piElementParticipant A pointer to the IElement to parse.
         */
        void Update(IN const IElement* piElementParticipant);

        /**
         * @brief Gets the identity of the participant.
         *
         * @return A const reference to the NameAddr object representing the identity.
         */
        inline const NameAddr& GetIdentity() const { return m_objIdentity; }

        /**
         * @brief Gets the target of the participant.
         *
         * @return A const reference to the Target object.
         */
        inline const Target& GetTarget() const { return m_objTarget; }

    protected:
        NameAddr m_objIdentity;
        Target m_objTarget;
        // session-description
        // cseq
    };

    /** Holds extra, often carrier-specific, information from the <dialog> element. */
    class ExtraInfo
    {
    public:
        ExtraInfo() :
                m_strExclusive(AString::ConstNull()),
                m_objMediaInfo(MediaInfo())
        {
        }
        ~ExtraInfo(){};
        ExtraInfo(IN const ExtraInfo&) = delete;
        ExtraInfo& operator=(IN const ExtraInfo& objRhs)
        {
            if (this != &objRhs)
            {
                m_strExclusive = objRhs.m_strExclusive;
                m_objMediaInfo = objRhs.m_objMediaInfo;
            }
            return *this;
        }

        /**
         * @brief Updates the ExtraInfo object by parsing extra elements from the <dialog> element.
         *
         * @param piElementDialog A pointer to the IElement representing the <dialog> element.
         */
        void Update(IN const IElement* piElementDialog);

        /** Gets the value of the 'exclusive' element, if present. */
        inline const AString& GetExclusive() const { return m_strExclusive; }

        /** Gets the media information parsed from media attributes. */
        inline const MediaInfo& GetMediaInfo() const { return m_objMediaInfo; }

    private:
        void HandleMediaInfo(IN const IElement* piElementDialog);
        static IMS_SINT32 ConvertMediaDirection(IN const AString& strMediaDirection);

    protected:
        AString m_strExclusive;
        MediaInfo m_objMediaInfo;
    };

public:
    Dialog() :
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

    /**
     * @brief Updates the Dialog object by parsing the <dialog> XML element.
     *
     * @param piElementDialog A pointer to the IElement representing the <dialog> element.
     * @return IMS_SUCCESS if the update is successful, IMS_FAILURE otherwise.
     */
    IMS_RESULT Update(IN const IElement* piElementDialog);

    /**
     * @brief Gets the state of the dialog.
     *
     * @return A const reference to the State object.
     */
    inline const State& GetState() const { return m_objState; }

    /**
     * @brief Gets the duration of the dialog in seconds.
     *
     * @return The duration.
     */
    inline IMS_UINT32 GetDuration() const { return m_nDuration; }

    /**
     * @brief Gets the 'replaces' information for the dialog.
     *
     * @return A const reference to the Replaces object.
     */
    inline const Replaces& GetReplaces() const { return m_objReplaces; }

    /**
     * @brief Gets the 'referred-by' information for the dialog.
     *
     * @return A const reference to the NameAddr object for the referrer.
     */
    inline const NameAddr& GetReferredBy() const { return m_objReferredBy; }

    /**
     * @brief Gets the local participant's information.
     *
     * @return A const reference to the Participant object for the local party.
     */
    inline const Participant& GetLocalParticipant() const { return m_objLocal; }

    /**
     * @brief Gets the remote participant's information.
     *
     * @return A const reference to the Participant object for the remote party.
     */
    inline const Participant& GetRemoteParticipant() const { return m_objRemote; }

    /**
     * @brief Gets extra information associated with the dialog.
     *
     * @return A const reference to the ExtraInfo object.
     */
    inline const ExtraInfo& GetExtraInfo() const { return m_objExtraInfo; }
    /** Gets the dialog identifier. */
    inline const AString& GetId() const { return m_strId; }
    /** Gets the SIP Call-ID of the dialog. */
    inline const AString& GetCallId() const { return m_strCallId; }
    /** Gets the local tag for the dialog. */
    inline const AString& GetLocalTag() const { return m_strLocalTag; }
    /** Gets the remote tag for the dialog. */
    inline const AString& GetRemoteTag() const { return m_strRemoteTag; }

private:
    static IMS_UINT32 ConvertDirection(IN const AString& strState);

public:
    /** Enum for the direction of the dialog (initiator or recipient). */
    enum
    {
        DIRECTION_IDLE = 0,
        DIRECTION_INITIATOR = 1,
        DIRECTION_RECIPIENT = 2,
    };

protected:
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
