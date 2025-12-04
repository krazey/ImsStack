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
#include "IDocument.h"
#include "IElement.h"
#include "INodeList.h"
#include "ISipHeader.h"
#include "IThread.h"
#include "IUce.h"
#include "IXmlRequest.h"
#include "IXmlResponse.h"
#include "IXmlTransaction.h"
#include "IXmlTransactionProvider.h"
#include "ServiceMessage.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "XmlFactory.h"
#include "def/UceDef.h"
#include "subscribe/UceNonCapabilityUser.h"
#include "subscribe/UceNotifyMessageBody.h"
#include "subscribe/UceXmlDocumentHelperThread.h"

__IMS_TRACE_TAG_UCE__;

PUBLIC
UceXmlDocumentHelperThread::UceXmlDocumentHelperThread(
        IN const AString& strQueryName, IN IMS_SINT32 nSimSlot) :
        m_piThread(IMS_NULL),
        m_nSimSlot(nSimSlot),
        m_nIndex(10),
        m_strQueryName(strQueryName),
        m_strThreadName(AString::ConstNull()),
        m_pUceNotifyMessageBody(IMS_NULL)
{
    m_pXMLTransactionProvider = XmlFactory::GetInstance()->CreateTransactionProvider();
    if (m_pXMLTransactionProvider != IMS_NULL)
    {
        m_pXMLTransactionProvider->SetStateListener(this);
    }
    IMS_TRACE_MEM("UCE_MEM", "UCE_M : UceXmlDocumentHelperThread = %" PFLS_u,
            sizeof(UceXmlDocumentHelperThread), 0, 0);
    IMS_TRACE_D("UceXmlDocumentHelperThread - Create", 0, 0, 0);
}

PUBLIC VIRTUAL UceXmlDocumentHelperThread::~UceXmlDocumentHelperThread()
{
    //---------------------------------------------------------------------------------------------
    Terminate();
    IMS_TRACE_MEM("UCE_MEM", "UCE_F : UceXmlDocumentHelperThread = %" PFLS_u,
            sizeof(UceXmlDocumentHelperThread), 0, 0);
    IMS_TRACE_I("~UceXmlDocumentHelperThread - delete", 0, 0, 0);
}

PUBLIC
IMS_BOOL UceXmlDocumentHelperThread::Start(IN const AString& strName, IN IMS_UINT32 nIndex)
{
    m_nIndex = nIndex;
    IMS_CHAR szIndex[10];
    IMS_Itoa(szIndex, nIndex, 10);

    m_strThreadName = strName;
    m_strThreadName += szIndex;
    if (m_piThread != IMS_NULL)
    {
        IMS_TRACE_I("Start : thread (%s) is already running.", m_strThreadName.GetStr(), 0, 0);
        return IMS_TRUE;
    }

    m_piThread = ThreadService::GetThreadService()->CreateThread(m_strThreadName, m_nSimSlot);

    if (m_piThread == IMS_NULL)
    {
        IMS_TRACE_I("Start : creating a thread (%s) failed.", m_strThreadName.GetStr(), 0, 0);
        return IMS_FALSE;
    }
    Initialize();
    m_piThread->SetRunnable(this);

    if (m_piThread->Activate() == IMS_FALSE)
    {
        IMS_TRACE_I("Start : starting a thread [%s] failed.", m_strThreadName.GetStr(), 0, 0);
        return IMS_FALSE;
    }
    IMS_TRACE_D("Start : Start is success [%s]", m_strThreadName.GetStr(), 0, 0);
    return IMS_TRUE;
}

PUBLIC
void UceXmlDocumentHelperThread::Terminate()
{
    //---------------------------------------------------------------------------------------------
    if (m_piThread == IMS_NULL)
    {
        return;
    }
    Uninitialize();

    m_piThread->Deactivate();
    ThreadService::GetThreadService()->DestroyThread(m_piThread);
    m_piThread = IMS_NULL;
}

void UceXmlDocumentHelperThread::SendMsg(
        IN IMS_UINT32 nMSG, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    if (m_piThread == IMS_NULL)
    {
        IMS_TRACE_D("SendMsg: m_piThread is null", 0, 0, 0);
        return;
    }
    m_piThread->PostMessageI(nMSG, nWparam, nLparam);
}

PUBLIC VIRTUAL IMS_RESULT UceXmlDocumentHelperThread::XmlTransaction_NotifyParsingCompleted(
        IN IXmlTransaction* piXMLTransaction)
{
    //---------------------------------------------------------------------------------------------
    IMS_RESULT eResult = IMS_FAILURE;
    if (m_objTransactionQueue.IsEmpty())
    {
        IMS_TRACE_E(0, "[ERROR]NotifyParsingCompleted - m_objTransactionQueue is empty", 0, 0, 0);
        return eResult;
    }
    IXmlTransaction* piXMLTxn = m_objTransactionQueue.GetFront();
    XMLInfo eXMLInfoType = XMLINFO_INVALID;
    m_objTransactionQueue.Pop();

    if (piXMLTxn == IMS_NULL || piXMLTxn != piXMLTransaction)
    {
        IMS_TRACE_E(0, "[ERROR]NotifyParsingCompleted - No matched XML transaction", 0, 0, 0);
        return eResult;
    }

    IXmlResponse* piResponse = piXMLTxn->GetResponse();
    if (piResponse == IMS_NULL)
    {
        IMS_TRACE_E(0, "[ERROR]NotifyParsingCompleted - IXmlResponse is null", 0, 0, 0);
        return eResult;
    }

    if (piResponse->GetResponseCode() == IXmlResponse::RESPONSE_CODE_SUCCESS)
    {
        IDocument* piDocument = piResponse->GetDocument();
        if (piDocument == IMS_NULL)
        {
            m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
            return eResult;
        }
        AString strRootName(AString::ConstNull());
        const IElement* piRootElement = piDocument->GetDocumentElement();
        if (piRootElement == IMS_NULL)
        {
            m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
            piDocument->DestroyDocument();
            return eResult;
        }
        strRootName = piRootElement->GetTagName();
        if (strRootName.GetLength() == 0)
        {
            IMS_TRACE_E(0, "[ERROR]NotifyParsingCompleted - strRootName is empty.", 0, 0, 0);
            m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
            piDocument->DestroyDocument();
            return eResult;
        }
        IMS_TRACE_D("NotifyParsingCompleted:strRootName [%s]", strRootName.GetStr(), 0, 0);
        if (strRootName.Contains("list"))
        {
            if (ParseRLMIList(piDocument) != IMS_SUCCESS)
            {
                m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
                piDocument->DestroyDocument();
                return eResult;
            }
            eXMLInfoType = XMLINFO_RLMI_LIST;
            eResult = IMS_SUCCESS;
        }
    }
    m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
    SendParseCompletedMsg(eXMLInfoType);
    return eResult;
}

void UceXmlDocumentHelperThread::XmlState_NotifyStateChanged()
{
    IMS_SINT32 nXMLState = 0;
    if (m_pXMLTransactionProvider == IMS_NULL)
    {
        return;
    }
    nXMLState = m_pXMLTransactionProvider->GetState();
    IMS_TRACE_D("NotifyXMLState:state [%d]", nXMLState, 0, 0);
}

PROTECTED
IThread* UceXmlDocumentHelperThread::GetThread() const
{
    //---------------------------------------------------------------------------------------------
    return m_piThread;
}

IMS_BOOL UceXmlDocumentHelperThread::OnStart(IN const IMSMSG& objMSG)
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_I("OnStart:[%d]", objMSG.GetName(), 0, 0);
    return IMS_TRUE;
}

IMS_BOOL UceXmlDocumentHelperThread::OnTerminate(IN const IMSMSG& objMSG)
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_I("OnTerminate:[%d]", objMSG.GetName(), 0, 0);
    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL UceXmlDocumentHelperThread::Runnable_Run(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("Runnable_Run:MessageName[%d]", objMSG.GetName(), 0, 0);
    IMS_SLONG nIndexOfFunc = m_objMessageMap.GetIndexOfKey(objMSG.GetName());
    if (nIndexOfFunc < 0)
    {
        IMS_TRACE_I("Runnable_Run:This message is not supported.", objMSG.GetName(), 0, 0);
        return IMS_TRUE;
    }
    msgHandler func = m_objMessageMap.GetValueAt(nIndexOfFunc);
    return (this->*(func))(objMSG);
}

PRIVATE IMS_BOOL UceXmlDocumentHelperThread::Initialize()
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_D("Initialize", 0, 0, 0);
    m_objMessageMap.Clear();
    m_objMessageMap.Add(IMS_MSG_START,
            reinterpret_cast<msgHandler>(&UceXmlDocumentHelperThread::StartMessageHandler));
    m_objMessageMap.Add(IMS_MSG_TERMINATE,
            reinterpret_cast<msgHandler>(&UceXmlDocumentHelperThread::TerminateMessageHandler));
    m_objMessageMap.Add(MSG_THREAD_RECEIVED_NOTIFY_RLMI,
            reinterpret_cast<msgHandler>(
                    &UceXmlDocumentHelperThread::ReceivedRlmiNotifyMessageHandler));
    m_objMessageMap.Add(MSG_THREAD_PARSERED_XML_RLMI,
            reinterpret_cast<msgHandler>(&UceXmlDocumentHelperThread::ParsedRlmiXmlMessageHandler));
    m_pNonCapabilities = new UceNonCapabilityUsers();
    m_pPidfXmls = new UcePidfXmls();
    return IMS_FALSE;
}

VIRTUAL void UceXmlDocumentHelperThread::Uninitialize()
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_D("Uninitialize", 0, 0, 0);
    while (!m_objTransactionQueue.IsEmpty())
    {
        IXmlTransaction* piXMLTransaction = m_objTransactionQueue.GetFront();
        m_objTransactionQueue.Pop();

        if (piXMLTransaction != IMS_NULL)
        {
            m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
        }
    }
    m_objTransactionQueue.Clear();

    if (m_pXMLTransactionProvider != IMS_NULL)
    {
        XmlFactory::GetInstance()->DestroyTransactionProvider(m_pXMLTransactionProvider);
        m_pXMLTransactionProvider = IMS_NULL;
    }
    m_objRlmiCidList.Clear();
    if (m_pNonCapabilities != IMS_NULL)
    {
        delete m_pNonCapabilities;
    }
    if (m_pPidfXmls != IMS_NULL)
    {
        delete m_pPidfXmls;
    }
    m_objBodyParts.Clear();
    m_objMessageMap.Clear();

    if (m_pUceNotifyMessageBody != IMS_NULL)
    {
        delete m_pUceNotifyMessageBody;
        m_pUceNotifyMessageBody = IMS_NULL;
    }
}

IMS_RESULT UceXmlDocumentHelperThread::XMLDataTokenization(IN const ByteArray& objBytes)
{
    IMS_TRACE_D("XMLDataTokenization()", 0, 0, 0);
    //---------------------------------------------------------------------------------------------
    IXmlTransaction* piXMLTransaction = m_pXMLTransactionProvider->CreateTransaction();

    if (piXMLTransaction != IMS_NULL)
    {
        IXmlRequest* piXMLRequest = piXMLTransaction->GetRequest();

        if (piXMLRequest != IMS_NULL)
        {
            piXMLRequest->SetRawXml(objBytes.ToString());
        }
        piXMLTransaction->SetListener(this);
        /* Push XML Transaction to Queue */
        m_objTransactionQueue.Push(piXMLTransaction);
        return piXMLTransaction->Send();
    }
    return IMS_FAILURE;
}

IMS_BOOL UceXmlDocumentHelperThread::StartMessageHandler(const IMSMSG& objMsg)
{
    return OnStart(objMsg);
}

IMS_BOOL UceXmlDocumentHelperThread::TerminateMessageHandler(const IMSMSG& objMsg)
{
    OnTerminate(objMsg);
    return IMS_TRUE;
}

IMS_BOOL UceXmlDocumentHelperThread::ReceivedRlmiNotifyMessageHandler(const IMSMSG& objMsg)
{
    m_pUceNotifyMessageBody = reinterpret_cast<UceNotifyMessageBody*>(objMsg.nLparam);
    if (m_pUceNotifyMessageBody == IMS_NULL)
    {
        IMS_TRACE_I("ReceivedRlmiNotifyMessageHandler:UceNotifyMessageBody is null", 0, 0, 0);
        IMS_MSG_CreateNPostActivityMessageByName(m_strQueryName,
                IUUceService::UCE_XML_PARSE_COMPLETED_IND,
                reinterpret_cast<IMS_UINTP>(m_pNonCapabilities),
                reinterpret_cast<IMS_UINTP>(m_pPidfXmls));
        return IMS_TRUE;
    }
    /*
    if (!m_objNonCapabilities.IsEmpty()) {
        for (IMS_UINT32 i = 0; i < m_objNonCapabilities.GetSize(); i++) {
            UceNonCapabilityUser* pData = m_objNonCapabilities.GetValueAt(i);
            if (pData != IMS_NULL) {
                delete pData;
            }
        }
    }
    m_objPidfXmls.Clear();
    m_objNonCapabilities.Clear();
    */

    IMS_BOOL bMatchingTheResult = IMS_FALSE;
    IMS_BOOL bFoundStartParameter = IMS_FALSE;
    AString strContentType = m_pUceNotifyMessageBody->GetContentType();
    AString strStart;
    IMS_TRACE_D("ReceivedRlmiNotifyMessageHandler:Content-Type[%s]", strContentType.GetStr(), 0, 0);

    ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::CONTENT_TYPE, strContentType);
    if (piHeader != IMS_NULL)
    {
        const SipParameter* pParameter = piHeader->GetParameter("start");
        if (pParameter != IMS_NULL)
        {
            strStart = pParameter->GetValue();
            if ((strStart.StartsWith('\"') == IMS_TRUE && strStart.EndsWith('\"') == IMS_TRUE) ||
                    (strStart.StartsWith('<') == IMS_TRUE && strStart.EndsWith('>') == IMS_TRUE))
            {
                strStart = strStart.GetSubStr(1, strStart.GetLength() - 2);
            }
            IMS_TRACE_D("ReceivedRlmiNotifyMessageHandler:start=[%s]", strStart.GetStr(), 0, 0);
            bFoundStartParameter = IMS_TRUE;
        }
        else
        {
            bFoundStartParameter = IMS_FALSE;
        }
        piHeader->Destroy();
    }

    m_objBodyParts = m_pUceNotifyMessageBody->GetNotifyBodyPartDatas();
    for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); i++)
    {
        UceNotifyBodyPartData* piBodyPart = m_objBodyParts.GetAt(i);
        const ByteArray& objContent = piBodyPart->GetBodyContent();
        strContentType = piBodyPart->GetContentType();
        if (bFoundStartParameter == IMS_TRUE)
        {
            AString strContentId = piBodyPart->GetContentId();
            if ((strContentId.StartsWith('\"') == IMS_TRUE &&
                        strContentId.EndsWith('\"') == IMS_TRUE) ||
                    (strContentId.StartsWith('<') == IMS_TRUE &&
                            strContentId.EndsWith('>') == IMS_TRUE))
            {
                strContentId = strContentId.GetSubStr(1, strContentId.GetLength() - 2);
            }
            IMS_TRACE_D(
                    "ReceivedRlmiNotifyMessageHandler:contentId[%s]", strContentId.GetStr(), 0, 0);
            if (strStart.Contains(strContentId) == IMS_TRUE)
            {
                XMLDataTokenization(objContent);
                m_objBodyParts.RemoveAt(i);
                return IMS_TRUE;
            }
        }
        else
        {
            IMS_TRACE_D("ReceivedRlmiNotifyMessageHandler: Number[%02d] : [%s]", i,
                    strContentType.GetStr(), 0);
            if (strContentType.Contains("application/rlmi+xml"))
            {
                XMLDataTokenization(objContent);
                m_objBodyParts.RemoveAt(i);
                return IMS_TRUE;
            }
        }
    }
    if (bMatchingTheResult == IMS_FALSE)
    {
        IMS_TRACE_I("ReceivedRlmiNotifyMessageHandler:Don`t match Any content type or id", 0, 0, 0);
        IMS_MSG_CreateNPostActivityMessageByName(m_strQueryName,
                IUUceService::UCE_XML_PARSE_COMPLETED_IND,
                reinterpret_cast<IMS_UINTP>(m_pNonCapabilities),
                reinterpret_cast<IMS_UINTP>(m_pPidfXmls));
    }
    return IMS_TRUE;
}

IMS_BOOL UceXmlDocumentHelperThread::ParsedRlmiXmlMessageHandler(const IMSMSG& objMsg)
{
    (void)objMsg;
    IMS_TRACE_D("ParsedRlmiXmlMessageHandler:remain bodyPart`s size[%d]", m_objBodyParts.GetSize(),
            0, 0);
    for (IMS_UINT32 i = 0; i < m_objRlmiCidList.GetSize(); ++i)
    {
        for (IMS_UINT32 j = 0; j < m_objBodyParts.GetSize(); ++j)
        {
            UceNotifyBodyPartData* piBodyPart = m_objBodyParts.GetAt(j);
            AString strConID = piBodyPart->GetContentId();
            if ((strConID.StartsWith('\"') == IMS_TRUE && strConID.EndsWith('\"') == IMS_TRUE) ||
                    (strConID.StartsWith('<') == IMS_TRUE && strConID.EndsWith('>') == IMS_TRUE))
            {
                strConID = strConID.GetSubStr(1, strConID.GetLength() - 2);
            }
            IMS_TRACE_D("ParsedRlmiXmlMessageHandler:content Id [%s]", strConID.GetStr(), 0, 0);
            if (strConID.Contains(m_objRlmiCidList.GetAt(i)) == IMS_TRUE)
            {
                IMS_TRACE_D("ParsedRlmiXmlMessageHandler:find content id.index[%d]", j, 0, 0);
                const ByteArray& objContent = piBodyPart->GetBodyContent();
                m_pPidfXmls->SetPidfXml(objContent.ToString());
                m_objBodyParts.RemoveAt(j);
                m_objRlmiCidList.RemoveAt(i);
                IMS_TRACE_D("ParsedRlmiXmlMessageHandler:current remain bodyPart`s size[%d]",
                        m_objBodyParts.GetSize(), 0, 0);
                return IMS_TRUE;
            }
        }
    }
    IMS_TRACE_I("ParsedRlmiXmlMessageHandler:RLMI parsing completed", 0, 0, 0);
    IMS_MSG_CreateNPostActivityMessageByName(m_strQueryName,
            IUUceService::UCE_XML_PARSE_COMPLETED_IND,
            reinterpret_cast<IMS_UINTP>(m_pNonCapabilities),
            reinterpret_cast<IMS_UINTP>(m_pPidfXmls));
    return IMS_TRUE;
}

void UceXmlDocumentHelperThread::SendParseCompletedMsg(IMS_SINT32 eXMLInfoType)
{
    IMS_TRACE_I("SendParseCompletedMsg:eXMLInfoType [%d]", eXMLInfoType, 0, 0);
    if (eXMLInfoType == XMLINFO_RLMI_LIST)
    {
        m_piThread->PostMessageI(MSG_THREAD_PARSERED_XML_RLMI, 0, 0);
    }
    else
    {
        IMS_TRACE_D("SendParseCompletedMsg - unknown XMLInfoType", 0, 0, 0);
    }
}

IMS_RESULT UceXmlDocumentHelperThread::ParseRLMIList(IN const IDocument* piDocument)
{
    if (piDocument == IMS_NULL)
    {
        IMS_TRACE_E(0, "piDocument is NULL", 0, 0, 0);
        return IMS_FAILURE;
    }

    IElement* piRootElement = piDocument->GetDocumentElement();
    if (piRootElement == IMS_NULL)
    {
        IMS_TRACE_E(0, "piRootElement is NULL", 0, 0, 0);
        return IMS_FAILURE;
    }

    INodeList* piNodeList = piRootElement->GetElementsByTagName("resource");
    if (piNodeList == IMS_NULL)
    {
        IMS_TRACE_E(0, "piNodeList is NULL", 0, 0, 0);
        return IMS_FAILURE;
    }
    /* Parse RLMI List */
    for (IMS_UINT32 i = 0; i < piNodeList->GetLength(); i++)
    {
        const IElement* piElement = IMS_NULL;
        AString strURI(AString::ConstNull());
        INodeList* piChildrenNodeList = IMS_NULL;
        // <resource ...>
        INode* piNode = piNodeList->Item(i);
        if (piNode == IMS_NULL)
        {
            IMS_TRACE_E(0, "piNode is NULL", 0, 0, 0);
            return IMS_FAILURE;
        }
        piElement = DYNAMIC_CAST(IElement*, piNode);
        // <resource uri.....>
        strURI = piElement->GetAttribute("uri");
        piChildrenNodeList = piNode->GetChildNodes();
        if (piChildrenNodeList == IMS_NULL)
        {
            IMS_TRACE_E(0, "piChildrenNodeList is NULL", 0, 0, 0);
            return IMS_FAILURE;
        }

        for (IMS_UINT32 j = 0; j < piChildrenNodeList->GetLength(); j++)
        {
            INode* piChildNode = piChildrenNodeList->Item(j);
            if (piChildNode->GetNodeType() == INode::ELEMENT_NODE)
            {
                const IElement* piChildElement = DYNAMIC_CAST(IElement*, piChildNode);
                AString temp;
                // <resource uri=...>
                //    <instance id=... state=... cid=.../>
                if (piChildElement->GetTagName().Equals("instance"))
                {
                    // <cid>
                    temp = piChildElement->GetAttribute("cid");
                    IMS_TRACE_D("ParseRLMIList:cid=[%d]th is [%s]", j, temp.GetStr(), 0);
                    if (temp.GetLength() == 0)
                    {
                        IMS_TRACE_D("ParseRLMIList:[%d]th cid is empty", j, 0, 0);
                    }
                    else
                    {
                        if ((temp.StartsWith('\"') == IMS_TRUE &&
                                    temp.EndsWith('\"') == IMS_TRUE) ||
                                (temp.StartsWith('<') == IMS_TRUE &&
                                        temp.EndsWith('>') == IMS_TRUE))
                        {
                            temp = temp.GetSubStr(1, temp.GetLength() - 2);
                        }
                        IMS_TRACE_D("ParseRLMIList:found cid [%s]", temp.GetStr(), 0, 0);
                        m_objRlmiCidList.Append(temp);
                    }

                    AString strState(AString::ConstNull());
                    AString strReason(AString::ConstNull());
                    // <state>
                    strState = piChildElement->GetAttribute("state");
                    // <reason>
                    strReason = piChildElement->GetAttribute("reason");
                    IMS_TRACE_D("ParseRLMIList:state [%s], reason[%s]", strState.GetStr(),
                            strReason.GetStr(), 0);
                    if (strReason.EqualsIgnoreCase("noresource") ||
                            strReason.EqualsIgnoreCase("deactivated") ||
                            strReason.EqualsIgnoreCase("rejected"))
                    {
                        UceNonCapabilityUser* pNonCapabilityUser =
                                new UceNonCapabilityUser(strURI, strReason);
                        m_pNonCapabilities->SetNonCapabilityUser(pNonCapabilityUser);
                    }
                }
            }
        }
        piRootElement->DestroyNodeList(piChildrenNodeList);
    }
    piRootElement->DestroyNodeList(piNodeList);
    return IMS_SUCCESS;
}
