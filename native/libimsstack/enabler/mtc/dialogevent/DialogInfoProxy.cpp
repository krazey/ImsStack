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
#include "dialogevent/DialogInfo.h"
#include "dialogevent/DialogInfoProxy.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR ELEMENT_DIALOG_INFO[] = "dialog-info";

PUBLIC
DialogInfoProxy::DialogInfoProxy() :
        m_objDialogInfos(ImsList<DialogInfo*>())
{
    IMS_TRACE_I("+DialogInfoProxy", 0, 0, 0);
}

PUBLIC
DialogInfoProxy::~DialogInfoProxy()
{
    IMS_TRACE_I("~DialogInfoProxy", 0, 0, 0);
    Clear();
}

PUBLIC
const AString& DialogInfoProxy::Update(IN const AString& strEventPackage)
{
    IMS_TRACE_I("Update", 0, 0, 0);

    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    if (pDocumentBuilder == IMS_NULL)
    {
        return AString::ConstNull();
    }

    IDocument* piDocument = pDocumentBuilder->Parse(strEventPackage);
    pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

    if (piDocument == IMS_NULL)
    {
        IMS_TRACE_E(0, "Parsing a 'dialog-info' XML failed", 0, 0, 0);
        return AString::ConstNull();
    }

    IElement* piElement = piDocument->GetDocumentElement();
    if (piElement == IMS_NULL)
    {
        piDocument->DestroyDocument();

        IMS_TRACE_E(0, "No root element", 0, 0, 0);
        return AString::ConstNull();
    }

    const AString& strDialogInfo = piElement->GetTagName();
    if (!strDialogInfo.EqualsIgnoreCase(ELEMENT_DIALOG_INFO))
    {
        IMS_TRACE_E(0, "Root element (%s) is not matched in 'dialog-info'", strDialogInfo.GetStr(),
                0, 0);

        piDocument->DestroyDocument();
        return AString::ConstNull();
    }

    DialogInfo* pNewDialogInfo = new DialogInfo();

    if (pNewDialogInfo->Update(piElement) == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "Update DialogInfo failed", 0, 0, 0);
        delete pNewDialogInfo;
        return AString::ConstNull();
    }

    const AString& strDialogInfoEntity = pNewDialogInfo->GetEntity();
    IMS_SLONG nIndex = GetIndexOfKeyHasSameId(strDialogInfoEntity);

    if (nIndex != -1)
    {
        DialogInfo* pOldDialogInfo = m_objDialogInfos.GetAt(nIndex);
        delete pOldDialogInfo;
        m_objDialogInfos.RemoveAt(nIndex);
    }

    m_objDialogInfos.Append(pNewDialogInfo);

    piDocument->DestroyDocument();

    // dialog state check?.
    // dialog version check required?
    return strDialogInfoEntity;
}

PUBLIC ImsList<JniExternalCall*> DialogInfoProxy::GetJniExternalCalls() const
{
    ImsList<JniExternalCall*> objJniExternalCalls;

    for (IMS_UINT32 index = 0; index < m_objDialogInfos.GetSize(); index++)
    {
        DialogInfo* pDialogInfo = m_objDialogInfos.GetAt(index);

        if (pDialogInfo == IMS_NULL)
        {
            continue;
        }

        objJniExternalCalls.AppendList(pDialogInfo->GetJniExternalCalls());
    }

    return objJniExternalCalls;
}

PRIVATE
void DialogInfoProxy::Clear()
{
    IMS_UINT32 nSize = m_objDialogInfos.GetSize();

    IMS_TRACE_I("Clear : Size[%d]", nSize, 0, 0);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        delete m_objDialogInfos.GetAt(index);
    }

    m_objDialogInfos.Clear();
}

PRIVATE
IMS_SLONG DialogInfoProxy::GetIndexOfKeyHasSameId(IN const AString& strDialogInfoEntity)
{
    IMS_UINT32 nSize = m_objDialogInfos.GetSize();

    IMS_TRACE_I("GetIndexOfKeyHasSameId : Size[%d]", nSize, 0, 0);

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        if (strDialogInfoEntity.Equals(m_objDialogInfos.GetAt(index)->GetEntity()))
        {
            return (IMS_SLONG)index;
        }
    }

    return -1;
}
