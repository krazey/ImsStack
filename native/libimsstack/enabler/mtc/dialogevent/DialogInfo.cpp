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

#include "IElement.h"
#include "INodeList.h"
#include "ServiceTrace.h"
#include "dialogevent/DialogInfo.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_DIALOG_INFO[] = "dialog-info";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_DIALOG_INFO_VERSION[] = "version";  // mandatory
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_DIALOG_INFO_STATE[] = "state";      // mandatory
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_DIALOG_INFO_ENTITY[] = "entity";    // mandatory
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_DIALOG[] = "dialog";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_DIALOG_ID[] = "id";  // mandatory
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_DIALOG_CALL_ID[] = "call-id";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_DIALOG_LOCAL_TAG[] = "local-tag";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_DIALOG_REMOTE_TAG[] = "remote-tag";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_DIALOG_DIRECTION[] = "direction";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_STATE[] = "state";  // mandatory
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_STATE_EVENT[] = "event";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_STATE_CDOE[] = "code";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_DURATIOIN[] = "duration";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_REPLACES[] = "replaces";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_REPLACES_CALL_ID[] = "call-id";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_REPLACES_LOCAL_TAG[] = "local-tag";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_REPLACES_REMOTE_TAG[] = "remote-tag";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_REFERRED_BY[] = "referred-by";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_NAMEADDR_DISPLAY_NAME[] = "display-name";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_LOCAL[] = "local";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_REMOTE[] = "remote";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_IDENTITY[] = "identity";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_TARGET[] = "target";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_TARGET_URI[] = "uri";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ELEMENT_PARAM[] = "param";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_PARAM_PNAME[] = "pname";
PUBLIC GLOBAL const IMS_CHAR DialogInfo::ATTR_PARAM_PVAL[] = "pval";

PUBLIC
DialogInfo::DialogInfo() :
        m_objDialogs(ImsList<Dialog*>()),
        m_nVersion(0),
        m_nState(STATE_FULL),
        m_strEntity(AString::ConstNull())
{
    IMS_TRACE_I("+DialogInfo", 0, 0, 0);
}

PUBLIC
DialogInfo::~DialogInfo()
{
    IMS_TRACE_I("~DialogInfo", 0, 0, 0);
    Clear();
}

PUBLIC
IMS_RESULT DialogInfo::Update(IN IElement* piElementDialogInfo)
{
    IMS_TRACE_I("+Update", 0, 0, 0);

    if (!IsMandatoryAttrExist(piElementDialogInfo, ATTR_DIALOG_INFO_VERSION) ||
            !IsMandatoryAttrExist(piElementDialogInfo, ATTR_DIALOG_INFO_STATE) ||
            !IsMandatoryAttrExist(piElementDialogInfo, ATTR_DIALOG_INFO_ENTITY))
    {
        IMS_TRACE_E(0, "mandatory value is not existed", 0, 0, 0);
        return IMS_FAILURE;
    }

    m_nVersion = piElementDialogInfo->GetAttribute(ATTR_DIALOG_INFO_VERSION).ToInt32();
    m_nState = ConvertState(piElementDialogInfo->GetAttribute(ATTR_DIALOG_INFO_STATE));
    m_strEntity = piElementDialogInfo->GetAttribute(ATTR_DIALOG_INFO_ENTITY);

    INodeList* piNodeListdialog =
            piElementDialogInfo->GetElementsByTagName(DialogInfo::ELEMENT_DIALOG);

    if (piNodeListdialog->GetLength() == 0)
    {
        return IMS_FAILURE;
    }

    for (IMS_SINT32 Index = 0; Index < piNodeListdialog->GetLength(); Index++)
    {
        INode* piNodeDialog = piNodeListdialog->Item(Index);
        if (piNodeDialog == IMS_NULL)
        {
            IMS_TRACE_E(0, "There is no 'dialog' Node", 0, 0, 0);
            continue;
        }

        IElement* piElementDialog = DYNAMIC_CAST(IElement*, piNodeDialog);

        Dialog* pNewDialog = new Dialog();
        if (pNewDialog->Update(piElementDialog) == IMS_FAILURE)
        {
            IMS_TRACE_E(0, "UpdateDialog failed", 0, 0, 0);
            delete pNewDialog;
            return IMS_FAILURE;
        }

        IMS_SLONG nIndex = GetIndexOfKeyHasSameId(pNewDialog->m_strId);

        if (nIndex != -1)
        {
            delete m_objDialogs.GetValueAt(nIndex);
            m_objDialogs.RemoveAt(nIndex);
        }

        m_objDialogs.Append(pNewDialog);
    }
    piElementDialogInfo->DestroyNodeList(piNodeListdialog);

    IMS_TRACE_I("Update : done", 0, 0, 0);
    return IMS_SUCCESS;
}

PUBLIC GLOBAL AString& DialogInfo::GetElementValue(
        IN const IElement* piElement, OUT AString& strElementValue)
{
    INode* piNode = piElement->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        if (piNode->GetNodeType() == INode::TEXT_NODE)
        {
            strElementValue = piNode->GetNodeValue();
            IMS_TRACE_I("GetElementValue : value=(%s)", strElementValue.GetStr(), 0, 0);
            return strElementValue;
        }

        piNode = piNode->GetNextSibling();
    }
    return strElementValue;
}

PUBLIC GLOBAL IMS_BOOL DialogInfo::IsMandatoryElementExist(
        IN const IElement* piElement, IN const IMS_CHAR* pszElement)
{
    INode* piNode = piElement->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();

        if (strName.EqualsIgnoreCase(pszElement))
        {
            return IMS_TRUE;
        }
        piNode = piNode->GetNextSibling();
    }
    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL DialogInfo::IsMandatoryAttrExist(
        IN const IElement* piElement, IN const IMS_CHAR* pszAttr)
{
    if (piElement->GetAttribute(pszAttr).GetLength() > 0)
    {
        return IMS_TRUE;
    }
    return IMS_FALSE;
}

PRIVATE
IMS_UINT32 DialogInfo::ConvertState(IN const AString& strState)
{
    if (strState.Equals("partial"))
    {
        return STATE_PARTIAL;
    }
    else
    {
        // default value
        return STATE_FULL;
    }
}

PRIVATE
void DialogInfo::Clear()
{
    IMS_UINT32 nSize = m_objDialogs.GetSize();

    IMS_TRACE_I("Clear : Size[%d]", nSize, 0, 0);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        delete m_objDialogs.GetValueAt(index);
    }

    m_objDialogs.Clear();
}

PRIVATE
IMS_SLONG DialogInfo::GetIndexOfKeyHasSameId(IN const AString& strDialogId)
{
    IMS_UINT32 nSize = m_objDialogs.GetSize();

    IMS_TRACE_I("GetIndexOfKeyHasSameId : Size[%d]", nSize, 0, 0);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        if (strDialogId.Equals(m_objDialogs.GetValueAt(index)->m_strId))
        {
            return (IMS_SLONG)index;
        }
    }

    return -1;
}

PUBLIC
IMS_RESULT Dialog::Update(IN IElement* piElementDialog)
{
    IMS_TRACE_I("+Update", 0, 0, 0);

    if (!DialogInfo::IsMandatoryAttrExist(piElementDialog, DialogInfo::ATTR_DIALOG_ID) ||
            !DialogInfo::IsMandatoryElementExist(piElementDialog, DialogInfo::ELEMENT_STATE))
    {
        IMS_TRACE_I("mandatory value is not existed", 0, 0, 0);
        return IMS_FAILURE;
    }

    m_strId = piElementDialog->GetAttribute(DialogInfo::ATTR_DIALOG_ID);
    m_strCallId = piElementDialog->GetAttribute(DialogInfo::ATTR_DIALOG_CALL_ID);
    m_strLocalTag = piElementDialog->GetAttribute(DialogInfo::ATTR_DIALOG_LOCAL_TAG);
    m_strRemoteTag = piElementDialog->GetAttribute(DialogInfo::ATTR_DIALOG_REMOTE_TAG);
    m_nDirection =
            ConvertDirection(piElementDialog->GetAttribute(DialogInfo::ATTR_DIALOG_DIRECTION));

    INode* piNode = piElementDialog->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();
        IElement* piElement = DYNAMIC_CAST(IElement*, piNode);

        if (strName.EqualsIgnoreCase(DialogInfo::ELEMENT_STATE))
        {
            m_objState.Update(piElement);
        }
        else if (strName.EqualsIgnoreCase(DialogInfo::ELEMENT_DURATIOIN))
        {
            AString strDuration;
            if (DialogInfo::GetElementValue(piElement, strDuration).GetLength() > 0)
            {
                m_nDuration = strDuration.ToInt32();
            }
        }
        else if (strName.EqualsIgnoreCase(DialogInfo::ELEMENT_REPLACES))
        {
            m_objReplaces.Update(piElement);
        }
        else if (strName.EqualsIgnoreCase(DialogInfo::ELEMENT_REFERRED_BY))
        {
            m_objReferredBy.Update(piElement);
        }
        else if (strName.EqualsIgnoreCase(DialogInfo::ELEMENT_LOCAL))
        {
            m_objLocal.Update(piElement);
        }
        else if (strName.EqualsIgnoreCase(DialogInfo::ELEMENT_REMOTE))
        {
            m_objRemote.Update(piElement);
        }
        else
        {
            IMS_TRACE_E(0, "invalid element", 0, 0, 0);
        }
        piNode = DYNAMIC_CAST(INode*, piElement);
        piNode = piNode->GetNextSibling();
    }

    IMS_TRACE_I("Update : done", 0, 0, 0);
    return IMS_SUCCESS;
}

PRIVATE
IMS_UINT32 Dialog::ConvertDirection(IN const AString& strState)
{
    if (strState.Equals("initiator"))
    {
        return DIRECTION_INITIATOR;
    }
    else if (strState.Equals("recipient"))
    {
        return DIRECTION_RECIPIENT;
    }
    else
    {
        // default value
        return DIRECTION_IDLE;
    }
}

PUBLIC
void Dialog::State::Update(IN const IElement* piElementState)
{
    m_nEvent = ConvertDialogStateEvent(piElementState->GetAttribute(DialogInfo::ATTR_STATE_EVENT));
    m_strCode = piElementState->GetAttribute(DialogInfo::ATTR_STATE_CDOE).ToInt32();

    AString strState;
    if (DialogInfo::GetElementValue(piElementState, strState).GetLength() > 0)
    {
        m_nState = ConvertDialogState(strState);
    }
}

PRIVATE
IMS_UINT32 Dialog::State::ConvertDialogState(IN const AString& strState)
{
    IMS_UINT32 nState = STATE_IDLE;

    if (strState.EqualsIgnoreCase("trying"))
    {
        nState = STATE_TRYING;
    }
    else if (strState.EqualsIgnoreCase("proceeding"))
    {
        nState = STATE_PROCEEDING;
    }
    else if (strState.EqualsIgnoreCase("early"))
    {
        nState = STATE_EARLY;
    }
    else if (strState.EqualsIgnoreCase("confirmed"))
    {
        nState = STATE_CONFIRMED;
    }
    else if (strState.EqualsIgnoreCase("terminated"))
    {
        nState = STATE_TERMINATED;
    }

    IMS_TRACE_I("ConvertDialogState : [%s] [%d]", strState.GetStr(), nState, 0);
    return nState;
}

PRIVATE
IMS_UINT32 Dialog::State::ConvertDialogStateEvent(IN const AString& strStateEvent)
{
    IMS_UINT32 nStateEvent = EVENT_IDLE;

    if (strStateEvent.EqualsIgnoreCase("cancelled"))
    {
        nStateEvent = EVENT_CANCELLED;
    }
    else if (strStateEvent.EqualsIgnoreCase("rejected"))
    {
        nStateEvent = EVENT_REJECTED;
    }
    else if (strStateEvent.EqualsIgnoreCase("replaced"))
    {
        nStateEvent = EVENT_REPLACED;
    }
    else if (strStateEvent.EqualsIgnoreCase("local-bye"))
    {
        nStateEvent = EVENT_LOCAL_BYE;
    }
    else if (strStateEvent.EqualsIgnoreCase("remote-bye"))
    {
        nStateEvent = EVENT_REMOTE_BYE;
    }
    else if (strStateEvent.EqualsIgnoreCase("error"))
    {
        nStateEvent = EVENT_ERROR;
    }
    else if (strStateEvent.EqualsIgnoreCase("timeout"))
    {
        nStateEvent = EVENT_TIMEOUT;
    }

    IMS_TRACE_I("ConvertDialogStateEvent : [%s] [%d]", strStateEvent.GetStr(), nStateEvent, 0);
    return nStateEvent;
}

PUBLIC
void Dialog::Replaces::Update(IN const IElement* piElementReplaces)
{
    m_strCallId = piElementReplaces->GetAttribute(DialogInfo::ATTR_REPLACES_CALL_ID);
    m_strLocalTag = piElementReplaces->GetAttribute(DialogInfo::ATTR_REPLACES_LOCAL_TAG);
    m_strRemoteTag = piElementReplaces->GetAttribute(DialogInfo::ATTR_REPLACES_REMOTE_TAG);
}

PUBLIC
void Dialog::NameAddr::Update(IN const IElement* piElementNameaddr)
{
    m_strDisplay = piElementNameaddr->GetAttribute(DialogInfo::ATTR_NAMEADDR_DISPLAY_NAME);

    AString strUri;
    if (DialogInfo::GetElementValue(piElementNameaddr, strUri).GetLength() > 0)
    {
        m_strUri = strUri;
    }
}

PUBLIC
void Dialog::Participant::Update(IN const IElement* piElementParticipant)
{
    INode* piNode = piElementParticipant->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();
        IElement* piElement = DYNAMIC_CAST(IElement*, piNode);

        if (strName.EqualsIgnoreCase(DialogInfo::ELEMENT_IDENTITY))
        {
            m_objIdentity.Update(piElement);
        }
        else if (strName.EqualsIgnoreCase(DialogInfo::ELEMENT_TARGET))
        {
            m_objtartget.Update(piElement);
        }
        else
        {
            IMS_TRACE_E(0, "invalid element", 0, 0, 0);
        }
        piNode = piNode->GetNextSibling();
    }
}

PUBLIC
IMS_RESULT Dialog::Target::Update(IN const IElement* piElementTarget)
{
    INode* piNode = piElementTarget->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();
        IElement* piElement = DYNAMIC_CAST(IElement*, piNode);

        if (strName.EqualsIgnoreCase(DialogInfo::ELEMENT_PARAM))
        {
            m_objTarget.Add(piElement->GetAttribute(DialogInfo::ATTR_PARAM_PNAME),
                    piElement->GetAttribute(DialogInfo::ATTR_PARAM_PVAL));
        }
        else
        {
            IMS_TRACE_E(0, "invalid element", 0, 0, 0);
        }
        piNode = piNode->GetNextSibling();
    }

    m_strUri = piElementTarget->GetAttribute(DialogInfo::ATTR_TARGET_URI);

    return IMS_SUCCESS;
}
