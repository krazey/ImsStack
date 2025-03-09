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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "DocumentBuilder.h"
#include "DocumentBuilderFactory.h"
#include "DomDocumentBuilderFactory.h"
#include "IDocument.h"
#include "IElement.h"

#include "Ims3gpp.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC GLOBAL const IMS_CHAR Ims3gpp::ELEMENT_ACTION[] = "action";
PUBLIC GLOBAL const IMS_CHAR Ims3gpp::ELEMENT_ALTERNATIVE_SERVICE[] = "alternative-service";
PUBLIC GLOBAL const IMS_CHAR Ims3gpp::ELEMENT_IMS_3GPP[] = "ims-3gpp";
PUBLIC GLOBAL const IMS_CHAR Ims3gpp::ELEMENT_REASON[] = "reason";
PUBLIC GLOBAL const IMS_CHAR Ims3gpp::ELEMENT_SERVICE_INFO[] = "service-info";
PUBLIC GLOBAL const IMS_CHAR Ims3gpp::ELEMENT_TYPE[] = "type";
PUBLIC GLOBAL const IMS_CHAR Ims3gpp::ATTR_VERSION[] = "version";

PUBLIC
Ims3gpp::Ims3gpp() :
        m_nType(TYPE_UNKNOWN)
{
}

PUBLIC
Ims3gpp::Ims3gpp(IN const AString& str3gppIms) :
        m_nType(TYPE_UNKNOWN)
{
    Parse(str3gppIms);
}

PUBLIC
IMS_BOOL Ims3gpp::Parse(IN const AString& str3gppIms)
{
    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    if (pDocumentBuilder == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IDocument* piDocument = pDocumentBuilder->Parse(str3gppIms);

    if (piDocument == IMS_NULL)
    {
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

        IMS_TRACE_E(0, "Parsing a '3gpp-ims' XML failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IElement* piElement = piDocument->GetDocumentElement();

    if (piElement == IMS_NULL)
    {
        piDocument->DestroyDocument();
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

        IMS_TRACE_E(0, "No root element", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strIms3gpp = piElement->GetTagName();

    if (!strIms3gpp.EqualsIgnoreCase(ELEMENT_IMS_3GPP))
    {
        IMS_TRACE_E(0, "Root element (%s) is not matched in '3gpp-ims'", strIms3gpp.GetStr(), 0, 0);

        piDocument->DestroyDocument();
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);
        return IMS_FALSE;
    }

    // "version" attribute
    const AString& strVersion = piElement->GetAttribute(ATTR_VERSION);

    IMS_TRACE_D("'ims-3gpp' version is %s", strVersion.GetStr(), 0, 0);

    // "service-info" / "alternative-service" element
    INode* piNode = piElement->GetFirstChild();

    if (piNode != IMS_NULL)
    {
        const AString& strName = piNode->GetLocalName();

        if (strName.EqualsIgnoreCase(ELEMENT_ALTERNATIVE_SERVICE))
        {
            m_nType = TYPE_ALTERNATIVE_SERVICE;
            CreateAlternativeService(piNode);
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_SERVICE_INFO))
        {
            m_nType = TYPE_SERVICE_INFO;
            CreateServiceInfo(piNode);
        }
        else
        {
            IMS_TRACE_D("ims-3gpp :: unknown element (%s)", strName.GetStr(), 0, 0);
        }
    }

    piDocument->DestroyDocument();
    pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

    return IMS_TRUE;
}

PRIVATE
void Ims3gpp::CreateAlternativeService(IN INode* piNode)
{
    // Value of type
    const IMS_CHAR VALUE_EMERGENCY[] = "emergency";
    const IMS_CHAR VALUE_RESTORATION[] = "restoration";
    // Value of action
    const IMS_CHAR VALUE_EMERGENCY_REGISTRATION[] = "emergency-registration";
    const IMS_CHAR VALUE_INITIAL_REGISTRATION[] = "initial-registration";
    const IMS_CHAR VALUE_ANONYMOUS_EMERGENCYCALL[] = "anonymous-emergencycall";

    if (piNode == IMS_NULL)
    {
        return;
    }

    INode* piElement = piNode->GetFirstChild();

    while (piElement != IMS_NULL)
    {
        const AString& strName = piElement->GetLocalName();

        if (strName.EqualsIgnoreCase(ELEMENT_TYPE))
        {
            INode* piNode_Value = piElement->GetFirstChild();

            if (piNode_Value != IMS_NULL)
            {
                m_objAlternativeService.m_strType = piNode_Value->GetNodeValue();

                if (m_objAlternativeService.m_strType.Equals(VALUE_EMERGENCY))
                {
                    m_objAlternativeService.m_nType = AlternativeService::TYPE_EMERGENCY;
                }
                else if (m_objAlternativeService.m_strType.Equals(VALUE_RESTORATION))
                {
                    m_objAlternativeService.m_nType = AlternativeService::TYPE_RESTORATION;
                }
            }
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_ACTION))
        {
            INode* piNode_Value = piElement->GetFirstChild();

            if (piNode_Value != IMS_NULL)
            {
                m_objAlternativeService.m_strAction = piNode_Value->GetNodeValue();

                if (m_objAlternativeService.m_strAction.Equals(VALUE_EMERGENCY_REGISTRATION))
                {
                    m_objAlternativeService.m_nAction =
                            AlternativeService::ACTION_EMERGENCY_REGISTRATION;
                }
                else if (m_objAlternativeService.m_strAction.Equals(VALUE_INITIAL_REGISTRATION))
                {
                    m_objAlternativeService.m_nAction =
                            AlternativeService::ACTION_INITIAL_REGISTRATION;
                }
                else if (m_objAlternativeService.m_strAction.Equals(VALUE_ANONYMOUS_EMERGENCYCALL))
                {
                    m_objAlternativeService.m_nAction =
                            AlternativeService::ACTION_ANONYMOUS_EMERGENCYCALL;
                }
            }
        }
        else if (strName.EqualsIgnoreCase(ELEMENT_REASON))
        {
            INode* piNode_Value = piElement->GetFirstChild();

            if (piNode_Value != IMS_NULL)
            {
                m_objAlternativeService.m_strReason = piNode_Value->GetNodeValue();
            }
        }

        piElement = piElement->GetNextSibling();
    }

    IMS_TRACE_I("ims-3gpp :: alternative-service :: type=%s, action=%s, reason=%s",
            m_objAlternativeService.m_strType.GetStr(),
            m_objAlternativeService.m_strAction.GetStr(),
            m_objAlternativeService.m_strReason.GetStr());
    IMS_TRACE_D("ims-3gpp :: alternative-service :: type=%d, action=%d",
            m_objAlternativeService.m_nType, m_objAlternativeService.m_nAction, 0);
}

PRIVATE
void Ims3gpp::CreateServiceInfo(IN INode* piNode)
{
    if (piNode == IMS_NULL)
    {
        return;
    }

    INode* piNode_Value = piNode->GetFirstChild();

    if (piNode_Value != IMS_NULL)
    {
        m_objServiceInfo.m_strServiceInfo = piNode_Value->GetNodeValue();
    }

    IMS_TRACE_I("ims-3gpp :: service-info :: service-info=%s",
            m_objServiceInfo.m_strServiceInfo.GetStr(), 0, 0);
}
