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

#ifndef _UCE_XML_DOCUMENT_HELPER_THREAD_H_
#define _UCE_XML_DOCUMENT_HELPER_THREAD_H_

#include "ImsMap.h"
#include "ImsQueue.h"
#include "IRunnable.h"
#include "IXmlStateListener.h"
#include "IXmlTransactionListener.h"

class ByteArray;
class IThread;
class IXmlTransactionProvider;
class IXmlTransaction;
class IDocument;
class UceNotifyMessageBody;
class UceNonCapabilityUser;
class UceNotifyBodyPartData;

class UceXmlDocumentHelperThread :
        public IRunnable,
        public IXmlTransactionListener,
        public IXmlStateListener
{
    typedef IMS_BOOL (UceXmlDocumentHelperThread::*msgHandler)(IMSMSG& objMsg);

public:
    UceXmlDocumentHelperThread(IN const AString& strQueryName_, IN IMS_SINT32 nSimSlot = 0);
    virtual ~UceXmlDocumentHelperThread();
    IMS_BOOL Start(IN const AString& strName, IN IMS_UINT32 _nIndex = 10);
    void Terminate();
    void SendMsg(IN IMS_UINT32 nMSG, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam);
    virtual IMS_RESULT XmlTransaction_NotifyParsingCompleted(IN IXmlTransaction* piXMLTransaction);
    virtual void XmlState_NotifyStateChanged();

protected:
    virtual IMS_BOOL Initialize();
    virtual void Uninitialize();
    virtual IMS_BOOL OnStart(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnTerminate(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);
    IThread* GetThread() const;

private:
    virtual IMS_BOOL Runnable_Run(IN IMSMSG& objMSG);
    IMS_RESULT XMLDataTokenization(IN const ByteArray& objBytes);
    IMS_BOOL StartMessageHandler(IMSMSG& objMsg);
    IMS_BOOL TerminateMessageHandler(IMSMSG& objMsg);
    IMS_BOOL ReceivedRlmiNotifyMessageHandler(IMSMSG& objMsg);
    IMS_BOOL ParsedRlmiXmlMessageHandler(IMSMSG& objMsg);
    void SendParseCompletedMsg(IMS_SINT32 eXMLInfoType);
    IMS_RESULT ParseRLMIList(IN IDocument* piDocument);

private:
    IMS_SINT32 m_nSimSlot;
    IMS_UINT32 m_nIndex;
    AString m_strQueryName;
    AString m_strThreadName;
    IThread* m_piThread;
    IXmlTransactionProvider* m_pXMLTransactionProvider;
    ImsQueue<IXmlTransaction*> m_objTransactionQueue;
    IMSList<AString> m_objRlmiCidList;
    IMSList<AString> m_objPidfXmls;
    IMSList<UceNonCapabilityUser*> m_objNonCapabilities;
    UceNotifyMessageBody* m_pUceNotifyMessageBody;
    IMSList<UceNotifyBodyPartData*> m_objBodyParts;

public:
    typedef enum _XMLINFO
    {
        XMLINFO_INVALID = 0,
        XMLINFO_RLMI_LIST,
    } XMLInfo;
    IMSMap<IMS_SINT32, msgHandler> m_objMessageMap;
};

#endif  // _UCE_XML_DOCUMENT_HELPER_THREAD_H_
