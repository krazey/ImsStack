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
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "ServiceTrace.h"
#include "dialogevent/DialogInfo.h"
#include "dialogevent/DialogInfoManager.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR ELEMENT_DIALOG_INFO[] = "dialog-info";

PUBLIC
DialogInfoManager::DialogInfoManager() :
        m_pLastDialogInfo(nullptr),
        m_objDialogs(ImsList<Dialog*>())
{
    IMS_TRACE_I("+DialogInfoManager", 0, 0, 0);
}

PUBLIC
DialogInfoManager::~DialogInfoManager()
{
    IMS_TRACE_I("~DialogInfoManager", 0, 0, 0);
}

PUBLIC
IMS_RESULT DialogInfoManager::Update(IN const AString& strEventPackage)
{
    IMS_TRACE_I("Update", 0, 0, 0);

    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    if (pDocumentBuilder == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    IDocument* piDocument = pDocumentBuilder->Parse(strEventPackage);
    pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

    if (piDocument == IMS_NULL)
    {
        IMS_TRACE_E(0, "Parsing a 'dialog-info' XML failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    IElement* piElement = piDocument->GetDocumentElement();
    if (piElement == IMS_NULL)
    {
        IMS_TRACE_E(0, "No root element", 0, 0, 0);
        piDocument->DestroyDocument();
        return IMS_FAILURE;
    }

    const AString& strDialogInfo = piElement->GetTagName();
    if (!strDialogInfo.EqualsIgnoreCase(ELEMENT_DIALOG_INFO))
    {
        IMS_TRACE_E(0, "Root element (%s) is not matched in 'dialog-info'", strDialogInfo.GetStr(),
                0, 0);

        piDocument->DestroyDocument();
        return IMS_FAILURE;
    }

    m_pLastDialogInfo = std::make_unique<DialogInfo>(m_objDialogs);

    if (m_pLastDialogInfo->Update(piElement) == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "Update DialogInfo failed", 0, 0, 0);
        piDocument->DestroyDocument();
        return IMS_FAILURE;
    }

    piDocument->DestroyDocument();
    return IMS_SUCCESS;
}

VIRTUAL PUBLIC IMS_UINT32 DialogInfoManager::GetState() const
{
    return m_pLastDialogInfo ? m_pLastDialogInfo->GetState() : DialogInfo::STATE_INVALID;
}

VIRTUAL PUBLIC IMS_UINT32 DialogInfoManager::GetVersion() const
{
    return m_pLastDialogInfo ? m_pLastDialogInfo->GetVersion() : 0;
}

VIRTUAL PUBLIC const AString& DialogInfoManager::GetEntity() const
{
    return m_pLastDialogInfo ? m_pLastDialogInfo->GetEntity() : AString::ConstNull();
}
