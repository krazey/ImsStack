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

#include "DocumentBuilder.h"
#include "DomDocumentBuilderFactory.h"
#include "IDocument.h"
#include "IElement.h"
#include "ServiceTrace.h"
#include "conferencecall/ConferenceInfo.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR ELEMENT_CONFERENCE_INFO[] = "conference-info";
LOCAL const IMS_CHAR ELEMENT_CONFERENCE_DESCRIPTION[] = "conference-description";
LOCAL const IMS_CHAR ELEMENT_USERS[] = "users";
LOCAL const IMS_CHAR ELEMENT_DISPLAY_TEXT[] = "display-text";
LOCAL const IMS_CHAR ELEMENT_STATUS[] = "status";

LOCAL const IMS_CHAR ATTR_VERSION[] = "version";
LOCAL const IMS_CHAR ATTR_STATE[] = "state";
LOCAL const IMS_CHAR ATTR_ENTITY[] = "entity";

PUBLIC
ConferenceInfo::User::~User()
{
    IMS_TRACE_I("~User", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < objEndPoints.GetSize(); i++)
    {
        delete objEndPoints.GetAt(i);
    }
    objEndPoints.Clear();
}

PUBLIC
ConferenceInfo::ConferenceInfo() :
        m_objConferenceDescription(ConferenceDescription()),
        m_objUsers(ImsList<User*>()),
        m_nState(STATE_INVALID),
        m_nVersion(0)
{
    IMS_TRACE_I("+ConferenceInfo", 0, 0, 0);
}

PUBLIC
ConferenceInfo::~ConferenceInfo()
{
    IMS_TRACE_I("~ConferenceInfo", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objUsers.GetSize(); i++)
    {
        delete m_objUsers.GetAt(i);
    }
    m_objUsers.Clear();
}

PUBLIC
const ConferenceInfo::ConferenceDescription& ConferenceInfo::GetConferenceDescription() const
{
    return m_objConferenceDescription;
}

PUBLIC
const ImsList<ConferenceInfo::User*>& ConferenceInfo::GetUsers() const
{
    return m_objUsers;
}

PUBLIC
IMS_BOOL ConferenceInfo::Parse(IN const AString& strConferenceInfoPackage)
{
    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    IDocument* piDocument = pDocumentBuilder->Parse(strConferenceInfoPackage);
    pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

    if (piDocument == IMS_NULL)
    {
        IMS_TRACE_E(0, "Parsing a 'conference-info' XML failed", 0, 0, 0);
        return IMS_FALSE;
    }

    const IElement* piElement = piDocument->GetDocumentElement();
    if (piElement == IMS_NULL)
    {
        piDocument->DestroyDocument();

        IMS_TRACE_E(0, "No root element", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strConferenceInfo = piElement->GetTagName();
    if (!strConferenceInfo.EqualsIgnoreCase(ELEMENT_CONFERENCE_INFO))
    {
        IMS_TRACE_E(0, "Root element (%s) is not matched in 'conference-info'",
                strConferenceInfo.GetStr(), 0, 0);

        piDocument->DestroyDocument();
        return IMS_FALSE;
    }

    CreateConferenceInfo(*piElement);

    piDocument->DestroyDocument();

    IMS_TRACE_I("Parse : done", 0, 0, 0);
    return IMS_TRUE;
}

PRIVATE
void ConferenceInfo::CreateConferenceInfo(IN const IElement& objElement)
{
    // "state" attribute
    m_nState = ConvertState(objElement.GetAttribute(ATTR_STATE));

    // "version" attribute
    m_nVersion = objElement.GetAttribute(ATTR_VERSION).ToInt32();

    const INode* piNode = objElement.GetFirstChild();
    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();

        if (strName.EqualsIgnoreCase(ELEMENT_CONFERENCE_DESCRIPTION))
        {
            CreateConferenceDescription(*piNode);
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_USERS))
        {
            CreateUsers(*piNode);
        }

        piNode = piNode->GetNextSibling();
    }

    IMS_TRACE_I("CreateConferenceInfo : version=[%d] state=[%d]", m_nVersion, m_nState, 0);
}

PRIVATE
void ConferenceInfo::CreateConferenceDescription(IN const INode& objNode)
{
    const IMS_CHAR ELEMENT_MAX_USER_COUNT[] = "maximum-user-count";

    IElement& objElement = DYNAMIC_CAST(IElement&, objNode);
    AString strMaxUserCount;
    if (GetSubElementValue(objElement, ELEMENT_MAX_USER_COUNT, strMaxUserCount).GetLength() > 0)
    {
        m_objConferenceDescription.nMaxUserCount = strMaxUserCount.ToInt32();
        IMS_TRACE_I("CreateConferenceDescription : maximum-user-count=%d",
                m_objConferenceDescription.nMaxUserCount, 0, 0);
    }
}

PRIVATE
void ConferenceInfo::CreateUsers(IN const INode& objNode)
{
    const IMS_CHAR ELEMENT_USER[] = "user";

    IElement& objElement = DYNAMIC_CAST(IElement&, objNode);

    ImsList<IElement*> objUserElements;
    GetSubElements(objElement, ELEMENT_USER, objUserElements);

    for (IMS_UINT32 i = 0; i < objUserElements.GetSize(); i++)
    {
        const IElement* piUserElement = objUserElements.GetAt(i);

        User* pUser = new User();
        pUser->strEntity = piUserElement->GetAttribute(ATTR_ENTITY);
        pUser->nState = ConvertState(piUserElement->GetAttribute(ATTR_STATE));
        GetSubElementValue(*piUserElement, ELEMENT_DISPLAY_TEXT, pUser->strDisplayText);

        IMS_TRACE_I("CreateUsers : entity=[%s] state=[%d] display-text=[%s]",
                pUser->strEntity.GetStr(), pUser->nState, pUser->strDisplayText.GetStr());

        CreateEndPointEntity(*piUserElement, *pUser);

        m_objUsers.Append(pUser);
    }
}

PRIVATE
void ConferenceInfo::CreateEndPointEntity(IN const IElement& objElement, IN User& objUser)
{
    const IMS_CHAR ELEMENT_ENDPOINT[] = "endpoint";

    ImsList<IElement*> objEndPointElements;
    GetSubElements(objElement, ELEMENT_ENDPOINT, objEndPointElements);

    for (IMS_UINT32 i = 0; i < objEndPointElements.GetSize(); i++)
    {
        const IElement* piEndPointElement = objEndPointElements.GetAt(i);

        User::EndPoint* pEndPoint = new User::EndPoint();

        pEndPoint->strEntity = piEndPointElement->GetAttribute(ATTR_ENTITY);
        pEndPoint->nState = ConvertState(piEndPointElement->GetAttribute(ATTR_STATE));

        AString strStatus;
        pEndPoint->nStatus =
                ConvertStatus(GetSubElementValue(*piEndPointElement, ELEMENT_STATUS, strStatus));

        GetSubElementValue(*piEndPointElement, ELEMENT_DISPLAY_TEXT, pEndPoint->strDisplayText);

        IMS_TRACE_I("CreateEndPointEntity : entity=[%s] state=[%d]", pEndPoint->strEntity.GetStr(),
                pEndPoint->nState, 0);
        IMS_TRACE_I("CreateEndPointEntity : status=[%d] display-text=[%s]", pEndPoint->nStatus,
                pEndPoint->strDisplayText.GetStr(), 0);

        objUser.objEndPoints.Append(pEndPoint);
    }
}

PRIVATE
const ImsList<IElement*>& ConferenceInfo::GetSubElements(const IN IElement& objElement,
        IN const IMS_CHAR* pszSubElementName, OUT ImsList<IElement*>& objSubElements)
{
    INode* piNode = objElement.GetFirstChild();

    while (piNode != IMS_NULL)
    {
        if (piNode->GetLocalName().EqualsIgnoreCase(pszSubElementName))
        {
            objSubElements.Append(DYNAMIC_CAST(IElement*, piNode));
        }

        piNode = piNode->GetNextSibling();
    }

    return objSubElements;
}

PRIVATE
const AString& ConferenceInfo::GetSubElementValue(IN const IElement& objElement,
        IN const IMS_CHAR* pszSubElementName, OUT AString& strSubElementValue)
{
    const INode* piNode = objElement.GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();

        if (strName.EqualsIgnoreCase(pszSubElementName))
        {
            const INode* piNode_Value = piNode->GetFirstChild();

            if (piNode_Value != IMS_NULL)
            {
                strSubElementValue = piNode_Value->GetNodeValue();
                IMS_TRACE_I("GetSubElementValue : value=(%s)", strSubElementValue.GetStr(), 0, 0);
                return strSubElementValue;
            }
        }

        piNode = piNode->GetNextSibling();
    }
    return strSubElementValue;
}

PRIVATE
IMS_UINT32 ConferenceInfo::ConvertState(IN const AString& strState)
{
    if (strState.Equals("partial"))
    {
        return STATE_PARTIAL;
    }
    else if (strState.Equals("deleted"))
    {
        return STATE_DELETED;
    }
    else
    {
        // default value
        return STATE_FULL;
    }
}

PRIVATE
IMS_UINT32 ConferenceInfo::ConvertStatus(IN const AString& strStatus)
{
    if (strStatus.Equals("connected"))
    {
        return STATUS_CONNECTED;
    }
    else if (strStatus.Equals("disconnected"))
    {
        return STATUS_DISCONNECTED;
    }
    else if (strStatus.Equals("on-hold"))
    {
        return STATUS_ON_HOLD;
    }
    else if (strStatus.Equals("muted-via-focus"))
    {
        return STATUS_MUTED_VIA_FOCUS;
    }
    else if (strStatus.Equals("pending"))
    {
        return STATUS_PENDING;
    }
    else if (strStatus.Equals("alerting"))
    {
        return STATUS_ALERTING;
    }
    else if (strStatus.Equals("dialing-in"))
    {
        return STATUS_DIALING_IN;
    }
    else if (strStatus.Equals("dialing-out"))
    {
        return STATUS_DIALING_OUT;
    }
    else if (strStatus.Equals("disconnecting"))
    {
        return STATUS_DISCONNECTING;
    }
    else if (strStatus.Equals("connect-fail"))  // in case participant rejects conference
    {
        return STATUS_FAIL;
    }
    else
    {
        // default value
        return STATUS_IDLE;
    }
}
