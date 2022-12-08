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
#include "JniExternalCall.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "dialogevent/DialogInfo.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR ATTR_DIALOG_INFO_VERSION[] = "version";  // mandatory
LOCAL const IMS_CHAR ATTR_DIALOG_INFO_STATE[] = "state";      // mandatory
LOCAL const IMS_CHAR ATTR_DIALOG_INFO_ENTITY[] = "entity";    // mandatory
LOCAL const IMS_CHAR ELEMENT_DIALOG[] = "dialog";
LOCAL const IMS_CHAR ATTR_DIALOG_ID[] = "id";  // mandatory
LOCAL const IMS_CHAR ATTR_DIALOG_CALL_ID[] = "call-id";
LOCAL const IMS_CHAR ATTR_DIALOG_LOCAL_TAG[] = "local-tag";
LOCAL const IMS_CHAR ATTR_DIALOG_REMOTE_TAG[] = "remote-tag";
LOCAL const IMS_CHAR ATTR_DIALOG_DIRECTION[] = "direction";
LOCAL const IMS_CHAR ELEMENT_STATE[] = "state";  // mandatory
LOCAL const IMS_CHAR ATTR_STATE_EVENT[] = "event";
LOCAL const IMS_CHAR ATTR_STATE_CDOE[] = "code";
LOCAL const IMS_CHAR ELEMENT_DURATIOIN[] = "duration";
LOCAL const IMS_CHAR ELEMENT_REPLACES[] = "replaces";
LOCAL const IMS_CHAR ATTR_REPLACES_CALL_ID[] = "call-id";
LOCAL const IMS_CHAR ATTR_REPLACES_LOCAL_TAG[] = "local-tag";
LOCAL const IMS_CHAR ATTR_REPLACES_REMOTE_TAG[] = "remote-tag";
LOCAL const IMS_CHAR ELEMENT_REFERRED_BY[] = "referred-by";
LOCAL const IMS_CHAR ATTR_NAMEADDR_DISPLAY_NAME[] = "display-name";
LOCAL const IMS_CHAR ELEMENT_LOCAL[] = "local";
LOCAL const IMS_CHAR ELEMENT_REMOTE[] = "remote";
LOCAL const IMS_CHAR ELEMENT_IDENTITY[] = "identity";
LOCAL const IMS_CHAR ELEMENT_TARGET[] = "target";
LOCAL const IMS_CHAR ATTR_TARGET_URI[] = "uri";
LOCAL const IMS_CHAR ELEMENT_PARAM[] = "param";
LOCAL const IMS_CHAR ATTR_PARAM_PNAME[] = "pname";
LOCAL const IMS_CHAR ATTR_PARAM_PVAL[] = "pval";
LOCAL const IMS_CHAR TARGET_PARAM_SIP_RENDERING[] = "+sip.rendering";

LOCAL const IMS_CHAR EXTRA_ELEMENT_EXCLUSIVE[] = "exclusive";
LOCAL const IMS_CHAR EXTRA_ELEMENT_MEDIAATTRIBUTES[] = "mediaAttributes";
LOCAL const IMS_CHAR EXTRA_ELEMENT_MEDIATYPE[] = "mediaType";
LOCAL const IMS_CHAR EXTRA_ELEMENT_MEDIADIRECTION[] = "mediaDirection";
LOCAL const IMS_CHAR EXTRA_ELEMENT_PORT0[] = "port0";

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

    if (!IsAttrExist(piElementDialogInfo, ATTR_DIALOG_INFO_VERSION) ||
            !IsAttrExist(piElementDialogInfo, ATTR_DIALOG_INFO_STATE) ||
            !IsAttrExist(piElementDialogInfo, ATTR_DIALOG_INFO_ENTITY))
    {
        IMS_TRACE_E(0, "mandatory value is not existed", 0, 0, 0);
        return IMS_FAILURE;
    }

    m_nVersion = piElementDialogInfo->GetAttribute(ATTR_DIALOG_INFO_VERSION).ToInt32();
    m_nState = ConvertState(piElementDialogInfo->GetAttribute(ATTR_DIALOG_INFO_STATE));
    m_strEntity = piElementDialogInfo->GetAttribute(ATTR_DIALOG_INFO_ENTITY);

    INodeList* piNodeListdialog = piElementDialogInfo->GetElementsByTagName(ELEMENT_DIALOG);

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
            delete m_objDialogs.GetAt(nIndex);
            m_objDialogs.RemoveAt(nIndex);
        }

        m_objDialogs.Append(pNewDialog);
    }
    piElementDialogInfo->DestroyNodeList(piNodeListdialog);

    IMS_TRACE_I("Update : done", 0, 0, 0);
    return IMS_SUCCESS;
}

PUBLIC
ImsList<JniExternalCall*> DialogInfo::GetJniExternalCalls() const
{
    ImsList<JniExternalCall*> objJniExternalCalls;

    for (IMS_UINT32 index = 0; index < m_objDialogs.GetSize(); index++)
    {
        Dialog* pDialog = m_objDialogs.GetAt(index);

        if (pDialog == IMS_NULL)
        {
            continue;
        }

        JniExternalCall* pJniExternalCall = new JniExternalCall();

        pJniExternalCall->m_strCallId = GetDialogId(pDialog);
        pJniExternalCall->m_strAddress = GetDialogRemoteAddress(pDialog);
        pJniExternalCall->m_strLocalAddress = GetDialogLocalAddress(pDialog);
        pJniExternalCall->m_bIsPullable = IsPullableDialog(pDialog);
        pJniExternalCall->m_nCallState = GetDialogCallState(pDialog);
        pJniExternalCall->m_nCallType = GetDialogCallType(pDialog);
        pJniExternalCall->m_bIsHeld = IsHeldDialog(pDialog);

        objJniExternalCalls.Append(pJniExternalCall);
    }

    return objJniExternalCalls;
}

PUBLIC GLOBAL IElement* DialogInfo::GetSubElement(
        IN const IElement* piElement, IN const IMS_CHAR* pszElement)
{
    INode* piNode = piElement->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();
        IElement* piSubElement = DYNAMIC_CAST(IElement*, piNode);

        if (strName.EqualsIgnoreCase(pszElement))
        {
            return piSubElement;
        }
        piNode = piNode->GetNextSibling();
    }
    return IMS_NULL;
}

PUBLIC GLOBAL AString& DialogInfo::GetSubElementValue(
        IN const IElement* piElement, IN const IMS_CHAR* pszElement, OUT AString& strElementValue)
{
    return GetElementValue(GetSubElement(piElement, pszElement), strElementValue);
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

PUBLIC GLOBAL IMS_BOOL DialogInfo::IsElementExist(
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

PUBLIC GLOBAL IMS_BOOL DialogInfo::IsAttrExist(
        IN const IElement* piElement, IN const IMS_CHAR* pszAttrName)
{
    if (piElement->GetAttribute(pszAttrName).GetLength() > 0)
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
        delete m_objDialogs.GetAt(index);
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
        if (strDialogId.Equals(m_objDialogs.GetAt(index)->m_strId))
        {
            return (IMS_SLONG)index;
        }
    }

    return -1;
}

PRIVATE
AString DialogInfo::GetDialogId(Dialog* pDialog)
{
    return pDialog->m_strId;
}

PRIVATE
AString DialogInfo::GetDialogRemoteAddress(Dialog* pDialog)
{
    return pDialog->m_objRemote.m_objIdentity.m_strUri;
}

PRIVATE
AString DialogInfo::GetDialogLocalAddress(Dialog* pDialog)
{
    return pDialog->m_objLocal.m_objIdentity.m_strUri;
}

PRIVATE
IMS_BOOL DialogInfo::IsPullableDialog(Dialog* pDialog) const
{
    if (GetDialogCallState(pDialog) == Dialog::State::STATE_CONFIRMED &&
            pDialog->m_objExtraInfo.m_strExclusive.EqualsIgnoreCase("false") &&
            (pDialog->m_objExtraInfo.m_objMediaInfo.eAudioQuality == AUDIO_QUALITY_AMR_WB &&
                    pDialog->m_objExtraInfo.m_objMediaInfo.eAudioDirection ==
                            DIRECTION_SEND_RECEIVE))
    {
        if (pDialog->m_objExtraInfo.m_objMediaInfo.eVideoQuality == VIDEO_QUALITY_NONE ||
                pDialog->m_objExtraInfo.m_objMediaInfo.eVideoQuality == VIDEO_QUALITY_NOTUSED)
        {
            return IMS_TRUE;
        }
        else if (pDialog->m_objExtraInfo.m_objMediaInfo.eVideoQuality == VIDEO_QUALITY_QVGA_PR &&
                pDialog->m_objExtraInfo.m_objMediaInfo.eVideoDirection == DIRECTION_SEND_RECEIVE)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_UINT32 DialogInfo::GetDialogCallState(Dialog* pDialog)
{
    return pDialog->m_objState.m_nState;
}

PRIVATE
IMS_UINT32 DialogInfo::GetDialogCallType(Dialog* pDialog)
{
    if (pDialog->m_objExtraInfo.m_objMediaInfo.eAudioQuality == AUDIO_QUALITY_AMR_WB)
    {
        if (pDialog->m_objExtraInfo.m_objMediaInfo.eVideoQuality == VIDEO_QUALITY_QVGA_PR)
        {
            return static_cast<IMS_UINT32>(CallType::VT);
        }
        else
        {
            return static_cast<IMS_UINT32>(CallType::VOIP);
        }
    }

    return static_cast<IMS_UINT32>(CallType::UNKNOWN);
}

PRIVATE
IMS_BOOL DialogInfo::IsHeldDialog(Dialog* pDialog)
{
    AString strPval = pDialog->m_objLocal.m_objTarget.m_objParamMap.GetValue(
            AString(TARGET_PARAM_SIP_RENDERING));
    return strPval.EqualsIgnoreCase("false");
}

PUBLIC
IMS_RESULT Dialog::Update(IN IElement* piElementDialog)
{
    IMS_TRACE_I("+Update", 0, 0, 0);

    if (!DialogInfo::IsAttrExist(piElementDialog, ATTR_DIALOG_ID) ||
            !DialogInfo::IsElementExist(piElementDialog, ELEMENT_STATE))
    {
        IMS_TRACE_I("Update : mandatory value is not existed", 0, 0, 0);
        return IMS_FAILURE;
    }

    m_strId = piElementDialog->GetAttribute(ATTR_DIALOG_ID);
    m_strCallId = piElementDialog->GetAttribute(ATTR_DIALOG_CALL_ID);
    m_strLocalTag = piElementDialog->GetAttribute(ATTR_DIALOG_LOCAL_TAG);
    m_strRemoteTag = piElementDialog->GetAttribute(ATTR_DIALOG_REMOTE_TAG);
    m_nDirection = ConvertDirection(piElementDialog->GetAttribute(ATTR_DIALOG_DIRECTION));

    INode* piNode = piElementDialog->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();
        IElement* piElement = DYNAMIC_CAST(IElement*, piNode);

        if (strName.EqualsIgnoreCase(ELEMENT_STATE))
        {
            m_objState.Update(piElement);
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_DURATIOIN))
        {
            AString strDuration;
            if (DialogInfo::GetElementValue(piElement, strDuration).GetLength() > 0)
            {
                m_nDuration = strDuration.ToInt32();
            }
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_REPLACES))
        {
            m_objReplaces.Update(piElement);
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_REFERRED_BY))
        {
            m_objReferredBy.Update(piElement);
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_LOCAL))
        {
            m_objLocal.Update(piElement);
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_REMOTE))
        {
            m_objRemote.Update(piElement);
        }
        else
        {
            IMS_TRACE_E(0, "Update : remain element [%s]", strName.GetStr(), 0, 0);
        }
        piNode = DYNAMIC_CAST(INode*, piElement);
        piNode = piNode->GetNextSibling();
    }

    m_objExtraInfo.Update(piElementDialog);

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
    m_nEvent = ConvertDialogStateEvent(piElementState->GetAttribute(ATTR_STATE_EVENT));
    m_nCode = piElementState->GetAttribute(ATTR_STATE_CDOE).ToInt32();

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
    m_strCallId = piElementReplaces->GetAttribute(ATTR_REPLACES_CALL_ID);
    m_strLocalTag = piElementReplaces->GetAttribute(ATTR_REPLACES_LOCAL_TAG);
    m_strRemoteTag = piElementReplaces->GetAttribute(ATTR_REPLACES_REMOTE_TAG);
}

PUBLIC
void Dialog::NameAddr::Update(IN const IElement* piElementNameaddr)
{
    m_strDisplay = piElementNameaddr->GetAttribute(ATTR_NAMEADDR_DISPLAY_NAME);

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

        if (strName.EqualsIgnoreCase(ELEMENT_IDENTITY))
        {
            m_objIdentity.Update(piElement);
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_TARGET))
        {
            m_objTarget.Update(piElement);
        }
        else
        {
            IMS_TRACE_E(0, "invalid element", 0, 0, 0);
        }
        piNode = piNode->GetNextSibling();
    }
}

PUBLIC
void Dialog::Target::Update(IN const IElement* piElementTarget)
{
    INode* piNode = piElementTarget->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();

        if (strName.EqualsIgnoreCase(ELEMENT_PARAM))
        {
            IElement* piElement = DYNAMIC_CAST(IElement*, piNode);
            AString strPname = piElement->GetAttribute(ATTR_PARAM_PNAME);
            AString strval = piElement->GetAttribute(ATTR_PARAM_PVAL);
            m_objParamMap.Add(strPname, strval);
        }
        else
        {
            IMS_TRACE_E(0, "invalid element", 0, 0, 0);
        }
        piNode = piNode->GetNextSibling();
    }

    m_strUri = piElementTarget->GetAttribute(ATTR_TARGET_URI);
}

PUBLIC
void Dialog::ExtraInfo::Update(IN const IElement* piElementDialog)
{
    INode* piNode = piElementDialog->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();
        IElement* piElement = DYNAMIC_CAST(IElement*, piNode);

        if (strName.Contains(EXTRA_ELEMENT_EXCLUSIVE))
        {
            AString strExclusive;
            if (DialogInfo::GetElementValue(piElement, strExclusive).GetLength() > 0)
            {
                m_strExclusive = strExclusive;
            }
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_LOCAL))
        {
            HandleMediaInfo(piElement);
        }

        piNode = DYNAMIC_CAST(INode*, piElement);
        piNode = piNode->GetNextSibling();
    }
}

PRIVATE
void Dialog::ExtraInfo::HandleMediaInfo(IN const IElement* piElementDialog)
{
    INode* piNode = piElementDialog->GetFirstChild();

    while (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();
        IElement* piElement = DYNAMIC_CAST(IElement*, piNode);

        if (strName.EqualsIgnoreCase(EXTRA_ELEMENT_MEDIAATTRIBUTES))
        {
            AString strMediaType;
            if (DialogInfo::GetSubElementValue(piElement, EXTRA_ELEMENT_MEDIATYPE, strMediaType)
                            .GetLength() > 0)
            {
                AString strMediaDirection;
                if (strMediaType.EqualsIgnoreCase("audio"))
                {
                    if (DialogInfo::IsElementExist(piElement, EXTRA_ELEMENT_PORT0))
                    {
                        m_objMediaInfo.eAudioQuality = AUDIO_QUALITY_NOTUSED;
                    }
                    else
                    {
                        m_objMediaInfo.eAudioQuality = AUDIO_QUALITY_AMR_WB;
                    }

                    if (DialogInfo::GetSubElementValue(
                                piElement, EXTRA_ELEMENT_MEDIADIRECTION, strMediaDirection)
                                    .GetLength() > 0)
                    {
                        m_objMediaInfo.eAudioDirection = ConvertMediaDirection(strMediaDirection);
                    }
                }
                else if (strMediaType.EqualsIgnoreCase("video"))
                {
                    if (DialogInfo::IsElementExist(piElement, EXTRA_ELEMENT_PORT0))
                    {
                        m_objMediaInfo.eVideoQuality = VIDEO_QUALITY_NOTUSED;
                    }
                    else
                    {
                        m_objMediaInfo.eVideoQuality = VIDEO_QUALITY_QVGA_PR;
                    }

                    if (DialogInfo::GetSubElementValue(
                                piElement, EXTRA_ELEMENT_MEDIADIRECTION, strMediaDirection)
                                    .GetLength() > 0)
                    {
                        m_objMediaInfo.eVideoDirection = ConvertMediaDirection(strMediaDirection);
                    }
                }
            }
        }

        piNode = DYNAMIC_CAST(INode*, piElement);
        piNode = piNode->GetNextSibling();
    }
}

PRIVATE
IMS_SINT32 Dialog::ExtraInfo::ConvertMediaDirection(IN const AString& strMediaDirection)
{
    if (strMediaDirection.EqualsIgnoreCase("sendrecv"))
    {
        return DIRECTION_SEND_RECEIVE;
    }
    else if (strMediaDirection.EqualsIgnoreCase("sendonly"))
    {
        return DIRECTION_SEND;
    }
    else if (strMediaDirection.EqualsIgnoreCase("recvonly"))
    {
        return DIRECTION_RECEIVE;
    }
    else if (strMediaDirection.EqualsIgnoreCase("inactive"))
    {
        return DIRECTION_INACTIVE;
    }
    else
    {
        return DIRECTION_INVALID;
    }
}
