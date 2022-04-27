/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120119  hyunho.shin@               Created
    </table>

    Description

*/

#include "ServiceTrace.h"
#include "ServiceThread.h"
#include "ServiceMessage.h"

#include "SIPParameter.h"
#include "SIPParsingHelper.h"
#include "ISIPHeader.h"

#include "INodeList.h"
#include "IXmlTransaction.h"
#include "IXmlRequest.h"
#include "IXmlResponse.h"
#include "IXmlTransactionProvider.h"
#include "IDocument.h"
#include "IElement.h"
#include "XmlFactory.h"

#include "def/UceDef.h"
#include "subscribe/UceXmlDocumentHelperThread.h"
#include "subscribe/UceNonCapabilityUser.h"
#include "subscribe/UceNotifyMessageBody.h"
#include "IUUceService.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

PUBLIC
UceXmlDocumentHelperThread::UceXmlDocumentHelperThread(IN CONST AString &strQueryName,
    IN IMS_SINT32 nSimSlot)
    : m_nSimSlot(nSimSlot)
    , m_nIndex(10)
    , m_strQueryName(strQueryName)
    , m_strThreadName(AString::ConstNull())
    , m_piThread(IMS_NULL)
    , m_pUceNotifyMessageBody(IMS_NULL)
{

    m_pXMLTransactionProvider = XmlFactory::GetInstance()->CreateTransactionProvider();
    if(m_pXMLTransactionProvider != IMS_NULL) {
        m_pXMLTransactionProvider->SetStateListener(this);
    }
    IMS_TRACE_MEM("UCE_MEM","UCE_M : UceXmlDocumentHelperThread = %" PFLS_u,
            sizeof(UceXmlDocumentHelperThread), 0, 0);
    IMS_TRACE_D("UceXmlDocumentHelperThread - Create", 0, 0, 0);
}

PUBLIC VIRTUAL
UceXmlDocumentHelperThread::~UceXmlDocumentHelperThread()
{
    //---------------------------------------------------------------------------------------------
    Terminate();
    IMS_TRACE_MEM("UCE_MEM","UCE_F : UceXmlDocumentHelperThread = %" PFLS_u,
            sizeof(UceXmlDocumentHelperThread), 0, 0);
    IMS_TRACE_I("~UceXmlDocumentHelperThread - delete", 0, 0, 0);
}

PUBLIC
IMS_BOOL UceXmlDocumentHelperThread::Start(IN CONST AString &strName, IN IMS_UINT32 nIndex)
{
    m_nIndex = nIndex;
    IMS_CHAR szIndex[10];
    IMS_Itoa(szIndex, nIndex, 10);

    m_strThreadName = strName;
    m_strThreadName += szIndex;
    if (m_piThread != IMS_NULL) {
        IMS_TRACE_I("Start : thread (%s) is already running.", m_strThreadName.GetStr(), 0, 0);
        return IMS_TRUE;
    }

    m_piThread = ThreadService::GetThreadService()->Create(m_strThreadName, m_nSimSlot);

    if (m_piThread == IMS_NULL) {
        IMS_TRACE_I("Start : creating a thread (%s) failed.", m_strThreadName.GetStr(), 0, 0);
        return IMS_FALSE;
    }
    Initialize();
    m_piThread->SetRunnable(this);

    if (m_piThread->Activate() == IMS_FALSE) {
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
    if (m_piThread == IMS_NULL) {
        return;
    }
    Uninitialize();

    m_piThread->Deactivate();
    ThreadService::GetThreadService()->Destroy(m_piThread);
    m_piThread = IMS_NULL;
}

void UceXmlDocumentHelperThread::SendMsg(IN IMS_UINT32 nMSG, IN IMS_UINTP nWparam,
        IN IMS_UINTP nLparam)
{
    m_piThread->PostMessageI(nMSG, nWparam, nLparam);
}

PUBLIC VIRTUAL
IMS_RESULT UceXmlDocumentHelperThread::XmlTransaction_NotifyParsingCompleted(
        IN IXmlTransaction* piXMLTransaction)
{
    //---------------------------------------------------------------------------------------------
    IMS_RESULT eResult = IMS_FAILURE;
    IXmlTransaction* piXMLTxn = m_objTransactionQueue.GetFront( );
    XMLInfo eXMLInfoType = XMLINFO_INVALID;
    m_objTransactionQueue.Pop();

    if (piXMLTxn == IMS_NULL || piXMLTxn != piXMLTransaction) {
        IMS_TRACE_E(0, "[ERROR]XML_ParseCompleted - No matched XML transaction", 0, 0, 0);
        return eResult;
    }

    IXmlResponse* piResponse = piXMLTxn->GetResponse();
    if (piResponse->GetResponseCode() == IXmlResponse::RESPONSE_CODE_SUCCESS) {
        IDocument* piDocument = piResponse->GetDocument();
        if (piDocument == IMS_NULL) {
            m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
            piXMLTransaction = IMS_NULL;
            return eResult;
        }
        AString strRootName(AString::ConstNull());
        IElement* piRootElement = piDocument->GetDocumentElement();
        if (piRootElement == IMS_NULL) {
            m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
            piXMLTransaction = IMS_NULL;
            piDocument->DestroyDocument();
            return eResult;
        }
        strRootName = piRootElement->GetTagName();
        if (strRootName.GetLength() == 0) {
            IMS_TRACE_E(0, "[ERROR]strRootName is empty.", 0, 0, 0);
            m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
            piXMLTransaction = IMS_NULL;
            piDocument->DestroyDocument();
            return eResult;
        }
        IMS_TRACE_D("XML_ParseCompleted:strRootName [%s]", strRootName.GetStr(), 0, 0 );
        if (strRootName.Contains("list")) {
            if (ParseRLMIList(piDocument) != IMS_SUCCESS) {
                m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
                piXMLTransaction = IMS_NULL;
                piDocument->DestroyDocument();
                return eResult;
            }
            eXMLInfoType = XMLINFO_RLMI_LIST;
            eResult = IMS_SUCCESS;
        }
    }
    m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
    piXMLTransaction = IMS_NULL;
    SendParseCompletedMsg(eXMLInfoType);
    return eResult;
}

void UceXmlDocumentHelperThread::XmlState_NotifyStateChanged()
{
    IMS_SINT32 nXMLState = 0;
    if (m_pXMLTransactionProvider == IMS_NULL) {
        return;
    }
    nXMLState = m_pXMLTransactionProvider->GetState();
    IMS_TRACE_D("NotifyXMLState:state [%d]", nXMLState, 0, 0);
}

PUBLIC VIRTUAL
IMS_BOOL UceXmlDocumentHelperThread::Initialize()
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_D("Initialize", 0, 0, 0);
    m_objMessageMap.Clear();
    m_objMessageMap.Add(IMS_MSG_START,
            reinterpret_cast<msgHandler>(&UceXmlDocumentHelperThread::StartMessageHandler));
    m_objMessageMap.Add(IMS_MSG_TERMINATE,
            reinterpret_cast<msgHandler>(&UceXmlDocumentHelperThread::TerminateMessageHandler));
    m_objMessageMap.Add(MSG_THREAD_RECEIVED_NOTIFY_RLMI,
            reinterpret_cast<msgHandler>
            (&UceXmlDocumentHelperThread::ReceivedRlmiNotifyMessageHandler));
    m_objMessageMap.Add(MSG_THREAD_PARSERED_XML_RLMI,
            reinterpret_cast<msgHandler>(&UceXmlDocumentHelperThread::ParsedRlmiXmlMessageHandler));
    return IMS_FALSE;
}

PUBLIC VIRTUAL
void UceXmlDocumentHelperThread::Uninitialize()
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_D("Uninitialize", 0, 0, 0);
    while (!m_objTransactionQueue.IsEmpty()) {
        IXmlTransaction* piXMLTransaction = m_objTransactionQueue.GetFront();
        m_objTransactionQueue.Pop();

        if (piXMLTransaction != IMS_NULL) {
            m_pXMLTransactionProvider->DestroyTransaction(piXMLTransaction);
            piXMLTransaction = IMS_NULL;
        }
    }
    m_objTransactionQueue.Clear();

    if (m_pXMLTransactionProvider != IMS_NULL) {
        XmlFactory::GetInstance()->DestroyTransactionProvider(m_pXMLTransactionProvider);
        m_pXMLTransactionProvider = IMS_NULL;
    }
    m_objRlmiCidList.Clear();
    m_objPidfXmls.Clear();
    m_objNonCapabilities.Clear();
    m_objBodyParts.Clear();
    m_objMessageMap.Clear();

    if(m_pUceNotifyMessageBody != IMS_NULL) {
        delete m_pUceNotifyMessageBody;
        m_pUceNotifyMessageBody = IMS_NULL;
    }
}

PROTECTED VIRTUAL
IMS_BOOL UceXmlDocumentHelperThread::OnStart(IN IMSMSG &objMSG)
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_I("OnStart:[%d]", objMSG.GetName(), 0, 0);
    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL UceXmlDocumentHelperThread::OnTerminate(IN IMSMSG &objMSG)
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_I("OnTerminate:[%d]", objMSG.GetName(), 0, 0);
    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL UceXmlDocumentHelperThread::OnMessage(IN IMSMSG &objMSG)
{
    //---------------------------------------------------------------------------------------------
    IMS_TRACE_I("OnMessage:[%d]", objMSG.GetName(), 0, 0);
    return IMS_TRUE;
}

PROTECTED
IThread* UceXmlDocumentHelperThread::GetThread() const
{
    //---------------------------------------------------------------------------------------------
    return m_piThread;
}

PRIVATE VIRTUAL
IMS_BOOL UceXmlDocumentHelperThread::Runnable_Run(IN IMSMSG &objMSG)
{
    IMS_TRACE_D("Runnable_Run:MessageName[%d]", objMSG.GetName(), 0, 0);
    IMS_SLONG nIndexOfFunc = m_objMessageMap.GetIndexOfKey(objMSG.GetName());
    if (nIndexOfFunc < 0) {
        IMS_TRACE_I( "Runnable_Run:This message is not supported.", objMSG.GetName(), 0, 0);
        return IMS_TRUE;
    }
    msgHandler func = m_objMessageMap.GetValueAt(nIndexOfFunc);
    return (this->*(func))(objMSG);
}

IMS_RESULT UceXmlDocumentHelperThread::XMLDataTokenization(IN CONST ByteArray& objBytes)
{
    IMS_TRACE_D( "XMLDataTokenization()", 0, 0, 0);
    IXmlRequest* piXMLRequest = IMS_NULL;
    //---------------------------------------------------------------------------------------------
    IXmlTransaction* piXMLTransaction = m_pXMLTransactionProvider->CreateTransaction();

    if (piXMLTransaction != IMS_NULL) {
        piXMLRequest = piXMLTransaction->GetRequest();

        if (piXMLRequest != IMS_NULL) {
            piXMLRequest->SetRawXml(objBytes.ToString());
        }
        piXMLTransaction->SetListener(this);
        /* Push XML Transaction to Queue */
        m_objTransactionQueue.Push(piXMLTransaction);
        return piXMLTransaction->Send();
    }
    return IMS_FAILURE;
}

IMS_BOOL UceXmlDocumentHelperThread::StartMessageHandler(IMSMSG &objMsg)
{
    if(!Initialize()) {
        return IMS_FALSE;
    }
    return OnStart(objMsg);
}

IMS_BOOL UceXmlDocumentHelperThread::TerminateMessageHandler(IMSMSG &objMsg)
{
    OnTerminate(objMsg);
    return IMS_TRUE;
}

IMS_BOOL UceXmlDocumentHelperThread::ReceivedRlmiNotifyMessageHandler(IMSMSG &objMsg)
{
    m_pUceNotifyMessageBody = (UceNotifyMessageBody*)objMsg.nLparam;
    if (m_pUceNotifyMessageBody == IMS_NULL) {
        IMS_TRACE_I("ReceivedRlmiNotifyMessageHandler:UceNotifyMessageBody is null",0,0,0);
        IMS_MSG_CreateNPostActivityMessageByName(m_strQueryName,
                IUUceService::UCE_XML_PARSE_COMPLETED_IND, &m_objNonCapabilities, &m_objPidfXmls);
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

    ISIPHeader* piHeader = SIPParsingHelper::CreateHeader(ISIPHeader::CONTENT_TYPE, strContentType);
    if (piHeader != IMS_NULL) {
        const SIPParameter* pParameter = piHeader->GetParameter("start");
        if (pParameter != IMS_NULL) {
            strStart = pParameter->GetValue();
            if(strStart.StartsWith('\"') == IMS_TRUE && strStart.EndsWith('\"') == IMS_TRUE) {
                strStart = strStart.GetSubStr(1, strStart.GetLength()-2);
            }
            else if(strStart.StartsWith('<') == IMS_TRUE && strStart.EndsWith('>') == IMS_TRUE) {
                strStart = strStart.GetSubStr(1, strStart.GetLength()-2);
            }
            IMS_TRACE_D("ReceivedRlmiNotifyMessageHandler:start=[%s]", strStart.GetStr(), 0, 0);
            bFoundStartParameter = IMS_TRUE;
        } else {
            bFoundStartParameter = IMS_FALSE;
        }
        piHeader->Destroy();
    }

    m_objBodyParts = m_pUceNotifyMessageBody->GetNotifyBodyPartDatas();
    for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); i++) {
        UceNotifyBodyPartData *piBodyPart = m_objBodyParts.GetAt(i);
        const ByteArray &objContent = piBodyPart->GetBodyContent();
        strContentType = piBodyPart->GetContentType();
        if (bFoundStartParameter == IMS_TRUE) {
            AString strContentId = piBodyPart->GetContentId();
            if(strContentId.StartsWith('\"') == IMS_TRUE && strContentId.EndsWith('\"')
                    == IMS_TRUE) {
                strContentId = strContentId.GetSubStr(1, strContentId.GetLength()-2);
            }
            else if(strContentId.StartsWith('<') == IMS_TRUE && strContentId.EndsWith('>')
                    == IMS_TRUE) {
                strContentId = strContentId.GetSubStr(1, strContentId.GetLength()-2);
            }
            IMS_TRACE_D("ReceivedRlmiNotifyMessageHandler:contentId[%s]",
                    strContentId.GetStr(), 0, 0);
            if (strStart.Contains(strContentId) == IMS_TRUE) {
                XMLDataTokenization(objContent);
                m_objBodyParts.RemoveAt(i);
                bMatchingTheResult = IMS_TRUE;
                return IMS_TRUE;
            }
        } else {
            IMS_TRACE_D( "ReceivedRlmiNotifyMessageHandler: Number[%02d] : [%s]",
                    i, strContentType.GetStr(),0);
            if (strContentType.Contains("application/rlmi+xml")) {
                XMLDataTokenization(objContent);
                m_objBodyParts.RemoveAt(i);
                bMatchingTheResult = IMS_TRUE;
                return IMS_TRUE;
            }
        }
    }
    if (bMatchingTheResult == IMS_FALSE) {
        IMS_TRACE_I("ReceivedRlmiNotifyMessageHandler:Don`t match Any content type or id", 0, 0, 0);
        IMS_MSG_CreateNPostActivityMessageByName(m_strQueryName,
                IUUceService::UCE_XML_PARSE_COMPLETED_IND, &m_objNonCapabilities, &m_objPidfXmls);
    }
    return IMS_TRUE;
}

IMS_BOOL UceXmlDocumentHelperThread::ParsedRlmiXmlMessageHandler(IMSMSG &objMsg)
{
    (void)objMsg;
    IMS_TRACE_D("ParsedRlmiXmlMessageHandler:remain bodyPart`s size[%d]",
            m_objBodyParts.GetSize(), 0, 0);
    for (IMS_UINT32 i = 0; i < m_objRlmiCidList.GetSize(); ++i) {
        for (IMS_UINT32 j = 0; j < m_objBodyParts.GetSize(); ++j) {
            UceNotifyBodyPartData* piBodyPart = m_objBodyParts.GetAt(j);
            AString strConID = piBodyPart->GetContentId();
            if(strConID.StartsWith('\"') == IMS_TRUE && strConID.EndsWith('\"') == IMS_TRUE) {
                strConID = strConID.GetSubStr(1, strConID.GetLength()-2);
            }
            else if(strConID.StartsWith('<') == IMS_TRUE && strConID.EndsWith('>') == IMS_TRUE) {
                strConID = strConID.GetSubStr(1, strConID.GetLength()-2);
            }
            IMS_TRACE_D( "ParsedRlmiXmlMessageHandler:content Id [%s]", strConID.GetStr(), 0, 0);
            if (strConID.Contains(m_objRlmiCidList.GetAt(i)) == IMS_TRUE) {
                IMS_TRACE_D("ParsedRlmiXmlMessageHandler:find content id.index[%d]", j, 0, 0);
                ByteArray &objContent = piBodyPart->GetBodyContent();
                m_objPidfXmls.Append(objContent.ToString());
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
            IUUceService::UCE_XML_PARSE_COMPLETED_IND, &m_objNonCapabilities, &m_objPidfXmls);
    m_objNonCapabilities.Clear();
    return IMS_TRUE;
}

void UceXmlDocumentHelperThread::SendParseCompletedMsg(IMS_SINT32 eXMLInfoType)
{
    IMS_TRACE_I("SendParseCompletedMsg:eXMLInfoType [%d]", eXMLInfoType, 0, 0 );
    if (eXMLInfoType == XMLINFO_RLMI_LIST) {
        m_piThread->PostMessageI(MSG_THREAD_PARSERED_XML_RLMI, 0, 0);
    } else {
        IMS_TRACE_D("SendParseCompletedMsg - unknown XMLInfoType", 0, 0, 0 );
    }
}

IMS_RESULT UceXmlDocumentHelperThread::ParseRLMIList( IN IDocument* piDocument )
{
    if (piDocument == IMS_NULL) {
        IMS_TRACE_E( 0, "piDocument is NULL", 0, 0, 0 );
        return IMS_FAILURE;
    }

    IElement* piRootElement = piDocument->GetDocumentElement();
    if (piRootElement == IMS_NULL) {
        IMS_TRACE_E( 0, "piRootElement is NULL", 0, 0, 0 );
        return IMS_FAILURE;
    }

    INodeList* piNodeList = piRootElement->GetElementsByTagName("resource");
    if (piNodeList == IMS_NULL) {
        IMS_TRACE_E( 0, "piNodeList is NULL", 0, 0, 0 );
        return IMS_FAILURE;
    }
    /* Parse RLMI List */
    for(IMS_UINT32 i = 0; i < piNodeList->GetLength(); i++) {
        IElement* piElement = IMS_NULL;
        AString strURI(AString::ConstNull());
        INodeList* piChildrenNodeList = IMS_NULL;
        // <resource ...>
        INode* piNode = piNodeList->Item(i);
        if (piNode == IMS_NULL) {
            IMS_TRACE_E( 0, "piNode is NULL", 0, 0, 0 );
            return IMS_FAILURE;
        }
        piElement = DYNAMIC_CAST(IElement*, piNode);
        // <resource uri.....>
        strURI = piElement->GetAttribute("uri");
        piChildrenNodeList = piNode->GetChildNodes();
        if (piChildrenNodeList == IMS_NULL) {
            IMS_TRACE_E( 0, "piChildrenNodeList is NULL", 0, 0, 0 );
            return IMS_FAILURE;
        }

        for (IMS_UINT32 j = 0; j < piChildrenNodeList->GetLength(); j++) {
            INode* piChildNode = piChildrenNodeList->Item(j);
            if (piChildNode->GetNodeType() == INode::ELEMENT_NODE) {
                IElement* piChildElement = DYNAMIC_CAST(IElement*, piChildNode);
                AString temp;
                // <resource uri=...>
                //    <instance id=... state=... cid=.../>
                if (piChildElement->GetTagName().Equals("instance")) {
                    // <cid>
                    temp = piChildElement->GetAttribute("cid");
                    IMS_TRACE_D("ParseRLMIList:cid=[%d]th is [%s]", j, temp.GetStr(), 0 );
                    if (temp.GetLength() == 0) {
                        IMS_TRACE_D( "ParseRLMIList:[%d]th cid is empty", j, 0, 0 );
                    } else {
                        if (temp.StartsWith('\"') == IMS_TRUE && temp.EndsWith('\"') == IMS_TRUE) {
                            temp = temp.GetSubStr(1, temp.GetLength()-2);
                        }
                        else if(temp.StartsWith('<') == IMS_TRUE && temp.EndsWith('>') == IMS_TRUE)
                        {
                            temp = temp.GetSubStr(1, temp.GetLength()-2);
                        }
                        IMS_TRACE_D( "ParseRLMIList:found cid [%s]", temp.GetStr(), 0, 0 );
                        m_objRlmiCidList.Append(temp);
                    }

                    AString strState(AString::ConstNull());
                    AString strReason(AString::ConstNull());
                    // <state>
                    strState = piChildElement->GetAttribute("state");
                    // <reason>
                    strReason = piChildElement->GetAttribute("reason");
                    IMS_TRACE_D( "ParseRLMIList:state [%s], reason[%s]",
                            strState.GetStr(), strReason.GetStr(), 0);
                    if(strReason.EqualsIgnoreCase("noresource") ||
                            strReason.EqualsIgnoreCase("deactivated") ||
                            strReason.EqualsIgnoreCase("rejected")) {
                        UceNonCapabilityUser* pNonCapabilityUser =
                                new UceNonCapabilityUser(strURI, strReason);
                        m_objNonCapabilities.Append(pNonCapabilityUser);
                    }
                }
            }
        }
        piRootElement->DestroyNodeList( piChildrenNodeList );
    }
    piRootElement->DestroyNodeList( piNodeList );
    return IMS_SUCCESS;
}
