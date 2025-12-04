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

#include "AString.h"
#include "ImsMap.h"
#include "ImsMessage.h"
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
class UceNonCapabilityUsers;
class UcePidfXmls;
class UceNotifyBodyPartData;

class UceXmlDocumentHelperThread :
        public IRunnable,
        public IXmlTransactionListener,
        public IXmlStateListener
{
    typedef IMS_BOOL (UceXmlDocumentHelperThread::*msgHandler)(IMSMSG& objMsg);

public:
    explicit UceXmlDocumentHelperThread(IN const AString& strQueryName, IN IMS_SINT32 nSimSlot = 0);
    virtual ~UceXmlDocumentHelperThread() override;
    IMS_BOOL Start(IN const AString& strName, IN IMS_UINT32 nIndex = 10);
    void Terminate();
    void SendMsg(IN IMS_UINT32 nMSG, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam);
    virtual IMS_RESULT XmlTransaction_NotifyParsingCompleted(
            IN IXmlTransaction* piXMLTransaction) override;
    virtual void XmlState_NotifyStateChanged() override;

protected:
    IThread* GetThread() const;

private:
    IMS_BOOL Initialize();
    void Uninitialize();
    IMS_BOOL OnStart(IN const IMSMSG& objMSG);
    IMS_BOOL OnTerminate(IN const IMSMSG& objMSG);
    virtual IMS_BOOL Runnable_Run(IN IMSMSG& objMSG) override;
    IMS_RESULT XMLDataTokenization(IN const ByteArray& objBytes);
    IMS_BOOL StartMessageHandler(const IMSMSG& objMsg);
    IMS_BOOL TerminateMessageHandler(const IMSMSG& objMsg);
    IMS_BOOL ReceivedRlmiNotifyMessageHandler(const IMSMSG& objMsg);
    IMS_BOOL ParsedRlmiXmlMessageHandler(const IMSMSG& objMsg);
    void SendParseCompletedMsg(IMS_SINT32 eXMLInfoType);
    IMS_RESULT ParseRLMIList(IN const IDocument* piDocument);

public:
    typedef enum _XMLINFO
    {
        XMLINFO_INVALID = 0,
        XMLINFO_RLMI_LIST,
    } XMLInfo;
    ImsMap<IMS_SINT32, msgHandler> m_objMessageMap;

protected:
    IThread* m_piThread;
    ImsQueue<IXmlTransaction*> m_objTransactionQueue;

private:
    IMS_SINT32 m_nSimSlot;
    IMS_UINT32 m_nIndex;
    AString m_strQueryName;
    AString m_strThreadName;
    IXmlTransactionProvider* m_pXMLTransactionProvider;
    ImsList<AString> m_objRlmiCidList;
    UcePidfXmls* m_pPidfXmls;
    UceNonCapabilityUsers* m_pNonCapabilities;
    UceNotifyMessageBody* m_pUceNotifyMessageBody;
    ImsList<UceNotifyBodyPartData*> m_objBodyParts;
};

#endif  // _UCE_XML_DOCUMENT_HELPER_THREAD_H_
