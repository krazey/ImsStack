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

PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ELEMENT_CONFERENCE_INFO[] = "conference-info";
PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ELEMENT_CONFERENCE_DESCRIPTION[] =
        "conference-description";
PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ELEMENT_HOST_INFO[] = "host-info";
PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ELEMENT_CONFERENCE_STATE[] = "conference-state";
PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ELEMENT_USERS[] = "users";

PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ELEMENT_DISPLAY_TEXT[] = "display-text";
PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ELEMENT_ENTRY[] = "entry";
PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ELEMENT_URI[] = "uri";
PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ELEMENT_STATUS[] = "status";

PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ATTR_VERSION[] = "version";
PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ATTR_STATE[] = "state";
PUBLIC GLOBAL const IMS_CHAR ConferenceInfo::ATTR_ENTITY[] = "entity";

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
        m_objHostInfo(HostInfo()),
        m_objConferenceState(ConferenceState()),
        m_objUsers(IMSList<User*>()),
        m_nState(STATE_INVALID),
        m_nVersion(0)
{
    IMS_TRACE_I("+ConferenceInfo", 0, 0, 0);
}

PUBLIC
ConferenceInfo::ConferenceInfo(IN const AString& strConferenceInfo) :
        m_objConferenceDescription(ConferenceDescription()),
        m_objHostInfo(HostInfo()),
        m_objConferenceState(ConferenceState()),
        m_objUsers(IMSList<User*>()),
        m_nState(STATE_INVALID),
        m_nVersion(0)
{
    IMS_TRACE_I("+ConferenceInfo", 0, 0, 0);
    Parse(strConferenceInfo);
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
const ConferenceInfo::HostInfo& ConferenceInfo::GetHostInfo() const
{
    return m_objHostInfo;
}

PUBLIC
const ConferenceInfo::ConferenceState& ConferenceInfo::GetConferenceState() const
{
    return m_objConferenceState;
}

PUBLIC
const IMSList<ConferenceInfo::User*>& ConferenceInfo::GetUsers() const
{
    return m_objUsers;
}

PUBLIC
IMS_BOOL ConferenceInfo::Parse(IN const AString& strConferenceInfoPackage)
{
    IMS_TRACE_I("Parse", 0, 0, 0);

    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    if (pDocumentBuilder == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IDocument* piDocument = pDocumentBuilder->Parse(strConferenceInfoPackage);
    pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

    if (piDocument == IMS_NULL)
    {
        IMS_TRACE_E(0, "Parsing a 'conference-info' XML failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IElement* piElement = piDocument->GetDocumentElement();
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

    CreateConferenceInfo(piElement);

    piDocument->DestroyDocument();

    IMS_TRACE_I("Parse : done", 0, 0, 0);
    return IMS_TRUE;
}

PRIVATE
void ConferenceInfo::CreateConferenceInfo(IN const IElement* piElement)
{
    // "state" attribute
    m_nState = ConvertState(piElement->GetAttribute(ATTR_STATE));

    // "version" attribute
    m_nVersion = piElement->GetAttribute(ATTR_VERSION).ToInt32();

    INode* piNode = piElement->GetFirstChild();
    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();

        if (strName.EqualsIgnoreCase(ELEMENT_CONFERENCE_DESCRIPTION))
        {
            CreateConferenceDescription(piNode);
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_HOST_INFO))
        {
            CreateHostInfo(piNode);
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_CONFERENCE_STATE))
        {
            CreateConferenceState(piNode);
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_USERS))
        {
            CreateUsers(piNode);
        }
        else
        {
            IMS_TRACE_E(0, "invalid element", 0, 0, 0);
        }
        piNode = piNode->GetNextSibling();
    }

    IMS_TRACE_I("CreateConferenceInfo : version=[%d] state=[%d]", m_nVersion, m_nState, 0);
}

PRIVATE
void ConferenceInfo::CreateConferenceDescription(IN const INode* piNode)
{
    const IMS_CHAR ELEMENT_MAX_USER_COUNT[] = "maximum-user-count";
    const IMS_CHAR ELEMENT_CONF_URIS[] = "conf-uris";
    const IMS_CHAR ELEMENT_PURPOSE[] = "purpose";

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_I("CreateConferenceDescription : piNode is null", 0, 0, 0);
        return;
    }

    IElement* piElement = DYNAMIC_CAST(IElement*, piNode);
    AString strMaxUserCount;
    if (GetSubElementValue(piElement, ELEMENT_MAX_USER_COUNT, strMaxUserCount).GetLength() > 0)
    {
        m_objConferenceDescription.nMaxUserCount = strMaxUserCount.ToInt32();
        IMS_TRACE_I("CreateConferenceDescription : maximum-user-count=%d",
                m_objConferenceDescription.nMaxUserCount, 0, 0);
    }

    const IElement* pConfUrisElement = GetSubElement(piElement, ELEMENT_CONF_URIS);
    if (pConfUrisElement == IMS_NULL)
    {
        return;
    }

    // TEMP parse conf-uris element. this information is useless.
    IMSList<IElement*> objEntries;
    if (GetSubElements(pConfUrisElement, ELEMENT_ENTRY, objEntries).IsEmpty())
    {
        IMS_TRACE_I("CreateConferenceDescription : no Entry elements", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < objEntries.GetSize(); i++)
    {
        AString strUri;
        AString strDisplayText;
        AString strPurpose;

        IElement* piTemp = objEntries.GetAt(i);
        GetSubElementValue(piTemp, ELEMENT_URI, strUri);
        GetSubElementValue(piTemp, ELEMENT_DISPLAY_TEXT, strDisplayText);
        GetSubElementValue(piTemp, ELEMENT_PURPOSE, strPurpose);

        IMS_TRACE_I("CreateConferenceDescription : uri=[%s] display=[%s] purpose=[%s]",
                strUri.GetStr(), strDisplayText.GetStr(), strPurpose.GetStr());
    }
}

PRIVATE
void ConferenceInfo::CreateHostInfo(IN const INode* piNode)
{
    // IMS_TRACE_I("CreateHostInfo", 0, 0, 0);
    const IMS_CHAR ELEMET_URIS[] = "uris";

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_I("CreateHostInfo : piNode is null", 0, 0, 0);
        return;
    }

    IElement* piElement = DYNAMIC_CAST(IElement*, piNode);
    GetSubElementValue(piElement, ELEMENT_DISPLAY_TEXT, m_objHostInfo.strDisplayText);
    IMS_TRACE_I("CreateHostInfo : display-text=[%s]", m_objHostInfo.strDisplayText.GetStr(), 0, 0);

    const IElement* pUrisElement = GetSubElement(piElement, ELEMET_URIS);
    if (pUrisElement == IMS_NULL)
    {
        return;
    }

    // TEMP parse conf-uris element. this information is useless.
    IMSList<IElement*> objEntries;
    if (GetSubElements(pUrisElement, ELEMENT_ENTRY, objEntries).IsEmpty())
    {
        IMS_TRACE_I("CreateHostInfo : no Entry elements", 0, 0, 0);
        return;
    }

    AString strUri;
    IElement* piTemp = objEntries.GetAt(0);  // only the first host uri is required.
    GetSubElementValue(piTemp, ELEMENT_URI, strUri);
    m_objHostInfo.objUris.Append(strUri);

    IMS_TRACE_I("CreateHostInfo : uri=[%s]", m_objHostInfo.objUris.GetAt(0).GetStr(), 0, 0);
}

PRIVATE
void ConferenceInfo::CreateConferenceState(IN const INode* piNode)
{
    const IMS_CHAR ELEMENT_USER_COUNT[] = "user-count";

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_I("CreateConferenceState : piNode is null", 0, 0, 0);
        return;
    }

    IElement* piElement = DYNAMIC_CAST(IElement*, piNode);
    AString strUserCount;
    if (GetSubElementValue(piElement, ELEMENT_USER_COUNT, strUserCount).GetLength() > 0)
    {
        m_objConferenceState.nUserCount = strUserCount.ToInt32();
        IMS_TRACE_I(
                "CreateConferenceState : user-count=[%d]", m_objConferenceState.nUserCount, 0, 0);
    }
}

PRIVATE
void ConferenceInfo::CreateUsers(IN const INode* piNode)
{
    const IMS_CHAR ELEMENT_USER[] = "user";

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_I("CreateUsers : piNode is null", 0, 0, 0);
        return;
    }

    IElement* piElement = DYNAMIC_CAST(IElement*, piNode);

    IMSList<IElement*> objUserElements;
    if (GetSubElements(piElement, ELEMENT_USER, objUserElements).IsEmpty())
    {
        IMS_TRACE_I("CreateUsers : no User elements", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < objUserElements.GetSize(); i++)
    {
        IElement* piUserElement = objUserElements.GetAt(i);

        User* pUser = new User();
        pUser->strEntity = piUserElement->GetAttribute(ATTR_ENTITY);
        pUser->nState = ConvertState(piUserElement->GetAttribute(ATTR_STATE));
        GetSubElementValue(piUserElement, ELEMENT_DISPLAY_TEXT, pUser->strDisplayText);

        IMS_TRACE_I("CreateUsers : entity=[%s] state=[%d] display-text=[%s]",
                pUser->strEntity.GetStr(), pUser->nState, pUser->strDisplayText.GetStr());

        CreateEndPointEntity(piUserElement, pUser);

        m_objUsers.Append(pUser);
    }
}

PRIVATE
void ConferenceInfo::CreateEndPointEntity(IN const IElement* piUserElement, IN User* pUser)
{
    const IMS_CHAR ELEMENT_ENDPOINT[] = "endpoint";

    if (piUserElement == IMS_NULL)
    {
        IMS_TRACE_I("CreateEndPointEntity : no user elements", 0, 0, 0);
        return;
    }

    IMSList<IElement*> objEndPointElements;
    if (GetSubElements(piUserElement, ELEMENT_ENDPOINT, objEndPointElements).IsEmpty())
    {
        IMS_TRACE_I("CreateEndPointEntity : no endpoint elements", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < objEndPointElements.GetSize(); i++)
    {
        IElement* piEndPointElement = objEndPointElements.GetAt(i);

        User::EndPoint* pEndPoint = new User::EndPoint();

        pEndPoint->strEntity = piEndPointElement->GetAttribute(ATTR_ENTITY);
        pEndPoint->nState = ConvertState(piEndPointElement->GetAttribute(ATTR_STATE));

        AString strStatus;
        pEndPoint->nStatus =
                ConvertStatus(GetSubElementValue(piEndPointElement, ELEMENT_STATUS, strStatus));

        GetSubElementValue(piEndPointElement, ELEMENT_DISPLAY_TEXT, pEndPoint->strDisplayText);

        IMS_TRACE_I("CreateEndPointEntity : entity=[%s] state=[%d]", pEndPoint->strEntity.GetStr(),
                pEndPoint->nState, 0);
        IMS_TRACE_I("CreateEndPointEntity : status=[%d] display-text=[%s]", pEndPoint->nStatus,
                pEndPoint->strDisplayText.GetStr(), 0);

        CreateMedia(piEndPointElement, pEndPoint);
        CreateCallInfo(piEndPointElement, pEndPoint);

        pUser->objEndPoints.Append(pEndPoint);
    }
}

PRIVATE
void ConferenceInfo::CreateMedia(IN const IElement* piEndPointElement, IN User::EndPoint* pEndPoint)
{
    (void)pEndPoint;
    (void)piEndPointElement;
    // TODO: Create media.
}

PRIVATE
void ConferenceInfo::CreateCallInfo(
        IN const IElement* piEndPointElement, IN User::EndPoint* pEndPoint)
{
    (void)pEndPoint;
    (void)piEndPointElement;
    // TODO: Create Call Info
}

PRIVATE
const IElement* ConferenceInfo::GetSubElement(
        IN const IElement* piElement, IN const IMS_CHAR* pszSubElementName)
{
    INode* piNode = piElement->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        if (piNode->GetLocalName().EqualsIgnoreCase(pszSubElementName))
        {
            return DYNAMIC_CAST(IElement*, piNode);
        }

        piNode = piNode->GetNextSibling();
    }
    return IMS_NULL;
}

PRIVATE
const IMSList<IElement*>& ConferenceInfo::GetSubElements(const IN IElement* piElement,
        IN const IMS_CHAR* pszSubElementName, OUT IMSList<IElement*>& objSubElements)
{
    // IMS_TRACE_I("GetSubElements : (%s)", pszSubElementName, 0, 0);
    if (piElement == IMS_NULL)
    {
        IMS_TRACE_I("GetSubElements : element is null", 0, 0, 0);
        return objSubElements;
    }

    INode* piNode = piElement->GetFirstChild();

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
const AString& ConferenceInfo::GetSubElementValue(IN const IElement* piElement,
        IN const IMS_CHAR* pszSubElementName, OUT AString& strSubElementValue)
{
    INode* piNode = piElement->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();

        if (strName.EqualsIgnoreCase(pszSubElementName))
        {
            INode* piNode_Value = piNode->GetFirstChild();

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
