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
#include "IXmlStateListener.h"
#include "IXmlTransaction.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "XmlApp.h"
#include "XmlResponse.h"
#include "XmlTransaction.h"
#include "XmlTransactionProvider.h"

__IMS_TRACE_TAG_XML__;

PUBLIC
XmlTransactionProvider::XmlTransactionProvider() :
        m_nState(STATE_IDLE),
        m_piListener(IMS_NULL),
        m_pXmlApp(IMS_NULL)
{
    m_pXmlApp = new XmlApp(this);
    Attach();
}

PUBLIC VIRTUAL XmlTransactionProvider::~XmlTransactionProvider()
{
    Detach();

    IMS_UINT32 nSize = m_objTransactions.GetSize();

    for (IMS_UINT32 i = 0; i < nSize; i++)
    {
        IXmlTransaction* piTransaction = m_objTransactions.GetFront();
        XmlTransaction* pTransaction = reinterpret_cast<XmlTransaction*>(piTransaction);
        delete pTransaction;
        m_objTransactions.Pop();
    }

    delete m_pXmlApp;
    m_pXmlApp = IMS_NULL;
}

PUBLIC VIRTUAL IXmlTransaction* XmlTransactionProvider::CreateTransaction()
{
    return new XmlTransaction(m_pXmlApp);
}

PUBLIC VIRTUAL void XmlTransactionProvider::DestroyTransaction(IN IXmlTransaction*& piTransaction)
{
    XmlTransaction* pTransaction = reinterpret_cast<XmlTransaction*>(piTransaction);

    if (pTransaction != IMS_NULL)
    {
        delete pTransaction;
    }

    piTransaction = IMS_NULL;
}

PUBLIC VIRTUAL IMS_RESULT XmlTransactionProvider::Push(IN IXmlTransaction* piTransaction)
{
    if (piTransaction == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    m_objTransactions.Push(piTransaction);

    IMS_TRACE_D("Push :: size=%d", m_objTransactions.GetSize(), 0, 0);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IXmlTransaction* XmlTransactionProvider::Pop()
{
    IXmlTransaction* piTransaction = m_objTransactions.GetFront();

    m_objTransactions.Pop();

    IMS_TRACE_D("Pop :: size=%d", m_objTransactions.GetSize(), 0, 0);

    return piTransaction;
}

PRIVATE VIRTUAL IMS_BOOL XmlTransactionProvider::OnMessage(IN IMSMSG& objMsg)
{
    IMS_TRACE_D("OnMessage :: msg=%d", objMsg.GetName(), 0, 0);

    if (m_objTransactions.GetSize() < 1)
    {
        switch (objMsg.GetName())
        {
            case XmlApp::AMSG_XML_ATTACH_RESPONSE:
            {
                XmlApp::AttachResponseParam* pParam =
                        reinterpret_cast<XmlApp::AttachResponseParam*>(objMsg.nLparam);

                if (pParam != IMS_NULL)
                {
                    delete pParam;
                    pParam = IMS_NULL;
                }
                break;
            }
            case XmlApp::AMSG_XML_PARSE_RESPONSE:
            {
                XmlApp::ParseResponseParam* pParam =
                        reinterpret_cast<XmlApp::ParseResponseParam*>(objMsg.nLparam);

                if (pParam != IMS_NULL)
                {
                    delete pParam;
                    pParam = IMS_NULL;
                }

                break;
            }
            default:
                break;
        }

        return IMS_FALSE;
    }

    switch (objMsg.GetName())
    {
        case XmlApp::AMSG_XML_ATTACH_RESPONSE:
        {
            XmlApp::AttachResponseParam* pParam =
                    reinterpret_cast<XmlApp::AttachResponseParam*>(objMsg.nLparam);
            IXmlTransaction* piTransaction = Pop();
            XmlTransaction* pTransaction = reinterpret_cast<XmlTransaction*>(piTransaction);

            if (pParam != IMS_NULL)
            {
                if (pParam->eResult == XmlApp::XmlResult::XML_RESULT_SUCCESS)
                {
                    SetState(STATE_ATTACHED);
                }
                else
                {
                    SetState(STATE_IDLE);
                }

                delete pParam;
            }

            if (pTransaction != IMS_NULL)
            {
                delete pTransaction;
            }
            break;
        }
        case XmlApp::AMSG_XML_PARSE_RESPONSE:
        {
            XmlApp::ParseResponseParam* pParam =
                    reinterpret_cast<XmlApp::ParseResponseParam*>(objMsg.nLparam);
            IXmlTransaction* piTransaction = Pop();
            XmlTransaction* pTransaction = reinterpret_cast<XmlTransaction*>(piTransaction);
            XmlResponse* pResponse = pTransaction->CreateResponse();

            if (pParam != IMS_NULL)
            {
                pTransaction->SetDocumentBuilder(pParam->pDocumentBuilder);

                if (pParam->eResult == XmlApp::XmlResult::XML_RESULT_SUCCESS)
                {
                    pResponse->SetResponseCode(IXmlResponse::RESPONSE_CODE_SUCCESS);
                    pResponse->SetDocument(pParam->piDocument);
                }
                else
                {
                    pResponse->SetResponseCode(IXmlResponse::RESPONSE_CODE_FAILURE);
                }

                delete pParam;
            }

            pTransaction->NotifyParsingCompleted();
            break;
        }
        default:
            break;
    }

    return IMS_TRUE;
}

PRIVATE
void XmlTransactionProvider::Attach()
{
    IMS_TRACE_D("Attach", 0, 0, 0);

    XmlTransaction* pTransaction = new XmlTransaction(m_pXmlApp);

    Push(pTransaction);

    SetState(STATE_ATTACHING);

    m_pXmlApp->Attach(GetName());
}

PRIVATE
void XmlTransactionProvider::Detach()
{
    IMS_TRACE_D("Detach", 0, 0, 0);

    SetState(STATE_DETACHED);

    m_pXmlApp->Detach();
}

PRIVATE
void XmlTransactionProvider::SetState(IN IMS_SINT32 nState)
{
    if (m_nState != nState)
    {
        IMS_TRACE_I("XmlState :: %d >> %d", m_nState, nState, 0);

        m_nState = nState;

        if (m_piListener != IMS_NULL)
        {
            m_piListener->XmlState_NotifyStateChanged();
        }
    }
}
