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

#ifndef CONFERENCE_INFO_H_
#define CONFERENCE_INFO_H_

#include "AString.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "conferencecall/ConferenceDef.h"

class INode;
class IElement;

class ConferenceInfo
{
public:
    class ConferenceDescription
    {
    public:
        inline explicit ConferenceDescription() :
                nMaxUserCount(DEFAULT_MAX_USER_COUNT)
        {
        }
        inline virtual ~ConferenceDescription() {}
        ConferenceDescription(IN const ConferenceDescription&) = delete;
        ConferenceDescription& operator=(IN const ConferenceDescription&) = delete;

    public:
        inline virtual IMS_UINT32 GetMaxUserCount() const { return nMaxUserCount; }

    private:
        friend class ConferenceInfo;
        static const IMS_UINT32 DEFAULT_MAX_USER_COUNT = 6;

        IMS_UINT32 nMaxUserCount;
    };

    class User
    {
    public:
        class EndPoint
        {
        public:
            inline explicit EndPoint() :
                    strEntity(AString::ConstNull()),
                    nState(STATE_INVALID),
                    strDisplayText(AString::ConstNull()),
                    nStatus(STATUS_IDLE)
            {
            }
            inline virtual ~EndPoint() {}
            EndPoint(IN const EndPoint&) = delete;
            EndPoint& operator=(IN const EndPoint&) = delete;

        public:
            inline virtual const AString& GetEntity() const { return strEntity; }
            inline virtual IMS_UINT32 GetState() const { return nState; }
            inline virtual const AString& GetDisplayText() const { return strDisplayText; }
            inline virtual IMS_UINT32 GetStatus() const { return nStatus; }

        private:
            friend class ConferenceInfo;

            AString strEntity;
            IMS_UINT32 nState;
            AString strDisplayText;
            IMS_UINT32 nStatus;
        };

    public:
        inline explicit User() :
                strEntity(AString::ConstNull()),
                nState(STATE_INVALID),
                strDisplayText(AString::ConstNull())
        {
        }
        virtual ~User();
        User(IN const User&) = delete;
        User& operator=(IN const User&) = delete;

    public:
        inline virtual const AString& GetEntity() const { return strEntity; }
        inline virtual IMS_UINT32 GetState() const { return nState; }
        inline virtual const AString& GetDisplayText() const { return strDisplayText; }
        inline virtual const ImsList<EndPoint*>& GetEndPoints() const { return objEndPoints; }

    private:
        friend class ConferenceInfo;

        AString strEntity;
        IMS_UINT32 nState;
        AString strDisplayText;
        ImsList<EndPoint*> objEndPoints;
    };

public:
    explicit ConferenceInfo();
    virtual ~ConferenceInfo();
    ConferenceInfo(IN const ConferenceInfo&) = delete;
    ConferenceInfo& operator=(IN const ConferenceInfo&) = delete;

public:
    virtual const ConferenceDescription& GetConferenceDescription() const;
    virtual const ImsList<User*>& GetUsers() const;

    virtual inline IMS_UINT32 GetState() const { return m_nState; }
    virtual inline IMS_SINT32 GetVersion() const { return m_nVersion; }

    virtual IMS_BOOL Parse(IN const AString& strConferenceInfoPackage);

private:
    void CreateConferenceInfo(IN const IElement& objElement);
    void CreateConferenceDescription(IN const INode& objNode);
    void CreateUsers(IN const INode& objNode);

    void CreateEndPointEntity(IN const IElement& objElement, IN User& objUser);

    static const ImsList<IElement*>& GetSubElements(IN const IElement& objElement,
            IN const IMS_CHAR* pszSubElementName, OUT ImsList<IElement*>& objSubElements);
    static const AString& GetSubElementValue(IN const IElement& objElement,
            IN const IMS_CHAR* pszSubElementName, OUT AString& strSubElementValue);

    static IMS_UINT32 ConvertState(IN const AString& strState);
    static IMS_UINT32 ConvertStatus(IN const AString& strStatus);

public:
    enum
    {
        STATE_INVALID = 0,
        STATE_FULL = 1,
        STATE_PARTIAL = 2,
        STATE_DELETED = 3
    };

private:
    ConferenceDescription m_objConferenceDescription;
    ImsList<User*> m_objUsers;
    IMS_UINT32 m_nState;
    IMS_SINT32 m_nVersion;
};

#endif
