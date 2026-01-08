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

#include "Feature.h"

#include "FeatureCaps.h"
#include "ISipMessage.h"
#include "SipMethod.h"
#include "util/CallerCapability.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
FeatureCaps::FeatureCaps() :
        m_pExcludedFeaturesForRegCaps(IMS_NULL),
        m_pFeaturesForAllMessage(IMS_NULL),
        m_pFeaturesForRequest(IMS_NULL),
        m_pFeaturesForResponse(IMS_NULL),
        m_pRegCaps(new CallerCapability(0))
{
}

PUBLIC VIRTUAL FeatureCaps::~FeatureCaps()
{
    if (m_pExcludedFeaturesForRegCaps != IMS_NULL)
    {
        delete m_pExcludedFeaturesForRegCaps;
    }

    if (m_pFeaturesForAllMessage != IMS_NULL)
    {
        delete m_pFeaturesForAllMessage;
    }

    if (m_pRegCaps != IMS_NULL)
    {
        delete m_pRegCaps;
    }

    if (m_pFeaturesForRequest != IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < m_pFeaturesForRequest->GetSize(); ++i)
        {
            CallerCapability* pCc = m_pFeaturesForRequest->GetValueAt(i);

            if (pCc != IMS_NULL)
            {
                delete pCc;
            }
        }

        delete m_pFeaturesForRequest;
    }

    if (m_pFeaturesForResponse != IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < m_pFeaturesForResponse->GetSize(); ++i)
        {
            CallerCapability* pCc = m_pFeaturesForResponse->GetValueAt(i);

            if (pCc != IMS_NULL)
            {
                delete pCc;
            }
        }

        delete m_pFeaturesForResponse;
    }
}

PUBLIC VIRTUAL void FeatureCaps::AddFeature(IN const AString& strName, IN const AString& strValue)
{
    CallerCapability* pFeatures = GetFeaturesForAllMessage(IMS_TRUE);

    if (pFeatures != IMS_NULL)
    {
        Feature objFeature(strName, strValue);

        pFeatures->AddFeature(&objFeature);
    }
}

PUBLIC VIRTUAL void FeatureCaps::AddFeature(IN const AString& strName, IN const AString& strValue,
        IN IMS_SINT32 nSipMethod, IN IMS_SINT32 nMessageType /*= ISipMessage::TYPE_ANY*/)
{
    if ((nSipMethod != SipMethod::INVITE) && (nSipMethod != SipMethod::SUBSCRIBE) &&
            (nSipMethod != SipMethod::REFER) && (nSipMethod != SipMethod::NOTIFY) &&
            (nSipMethod != SipMethod::OPTIONS) && (nSipMethod != SipMethod::PUBLISH))
    {
        IMS_TRACE_E(0, "FeatureCaps :: Method(%d) is not allowed", nSipMethod, 0, 0);
        return;
    }

    if ((nMessageType < ISipMessage::TYPE_REQUEST) || (nMessageType > ISipMessage::TYPE_ANY))
    {
        IMS_TRACE_E(0, "FeatureCaps :: Message type(%d) is not allowed", nMessageType, 0, 0);
        return;
    }

    Feature objFeature(strName, strValue);

    if (nMessageType == ISipMessage::TYPE_ANY)
    {
        // REQUEST
        CallerCapability* pFeatures = GetFeaturesForRequest(nSipMethod, IMS_TRUE);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->AddFeature(&objFeature);
        }

        // RESPONSE
        pFeatures = GetFeaturesForResponse(nSipMethod, IMS_TRUE);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->AddFeature(&objFeature);
        }
    }
    else if (nMessageType == ISipMessage::TYPE_REQUEST)
    {
        CallerCapability* pFeatures = GetFeaturesForRequest(nSipMethod, IMS_TRUE);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->AddFeature(&objFeature);
        }
    }
    else  // ISipMessage::TYPE_RESPONSE
    {
        CallerCapability* pFeatures = GetFeaturesForResponse(nSipMethod, IMS_TRUE);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->AddFeature(&objFeature);
        }
    }
}

PUBLIC VIRTUAL void FeatureCaps::RemoveFeature(
        IN const AString& strName, IN const AString& strValue)
{
    CallerCapability* pFeatures = GetFeaturesForAllMessage();

    if (pFeatures != IMS_NULL)
    {
        Feature objFeature(strName, strValue);

        pFeatures->RemoveFeature(&objFeature);
    }
}

PUBLIC VIRTUAL void FeatureCaps::RemoveFeature(IN const AString& strName,
        IN const AString& strValue, IN IMS_SINT32 nSipMethod,
        IN IMS_SINT32 nMessageType /*= ISipMessage::TYPE_ANY*/)
{
    if ((nSipMethod != SipMethod::INVITE) && (nSipMethod != SipMethod::SUBSCRIBE) &&
            (nSipMethod != SipMethod::REFER) && (nSipMethod != SipMethod::NOTIFY) &&
            (nSipMethod != SipMethod::OPTIONS) && (nSipMethod != SipMethod::PUBLISH))
    {
        IMS_TRACE_E(0, "FeatureCaps :: Method(%d) is not allowed", nSipMethod, 0, 0);
        return;
    }

    if ((nMessageType < ISipMessage::TYPE_REQUEST) || (nMessageType > ISipMessage::TYPE_ANY))
    {
        IMS_TRACE_E(0, "FeatureCaps :: Message type(%d) is not allowed", nMessageType, 0, 0);
        return;
    }

    Feature objFeature(strName, strValue);

    if (nMessageType == ISipMessage::TYPE_ANY)
    {
        // REQUEST
        CallerCapability* pFeatures = GetFeaturesForRequest(nSipMethod);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->RemoveFeature(&objFeature);
        }

        // RESPONSE
        pFeatures = GetFeaturesForResponse(nSipMethod);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->RemoveFeature(&objFeature);
        }
    }
    else if (nMessageType == ISipMessage::TYPE_REQUEST)
    {
        CallerCapability* pFeatures = GetFeaturesForRequest(nSipMethod);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->RemoveFeature(&objFeature);
        }
    }
    else  // ISipMessage::TYPE_RESPONSE
    {
        CallerCapability* pFeatures = GetFeaturesForResponse(nSipMethod);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->RemoveFeature(&objFeature);
        }
    }
}

PUBLIC VIRTUAL void FeatureCaps::RemoveAllFeatures()
{
    if (m_pFeaturesForAllMessage != IMS_NULL)
    {
        m_pFeaturesForAllMessage->Clear();
    }

    if (m_pFeaturesForRequest != IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < m_pFeaturesForRequest->GetSize(); ++i)
        {
            CallerCapability* pCc = m_pFeaturesForRequest->GetValueAt(i);

            if (pCc != IMS_NULL)
            {
                pCc->Clear();
            }
        }
    }

    if (m_pFeaturesForResponse != IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < m_pFeaturesForResponse->GetSize(); ++i)
        {
            CallerCapability* pCc = m_pFeaturesForResponse->GetValueAt(i);

            if (pCc != IMS_NULL)
            {
                pCc->Clear();
            }
        }
    }
}

PUBLIC VIRTUAL void FeatureCaps::AddExcludedFeatureForRegCaps(
        IN const AString& strName, IN const AString& strValue)
{
    CallerCapability* pFeatures = GetExcludedFeaturesForRegCaps(IMS_TRUE);

    if (pFeatures != IMS_NULL)
    {
        Feature objFeature(strName, strValue);

        pFeatures->AddFeature(&objFeature);
    }
}

PUBLIC VIRTUAL void FeatureCaps::RemoveExcludedFeatureForRegCaps(
        IN const AString& strName, IN const AString& strValue)
{
    CallerCapability* pFeatures = GetExcludedFeaturesForRegCaps();

    if (pFeatures != IMS_NULL)
    {
        Feature objFeature(strName, strValue);

        pFeatures->RemoveFeature(&objFeature);
    }
}

PUBLIC VIRTUAL void FeatureCaps::RemoveAllExcludedFeaturesForRegCaps()
{
    if (m_pExcludedFeaturesForRegCaps != IMS_NULL)
    {
        m_pExcludedFeaturesForRegCaps->Clear();
    }
}

PUBLIC
IMS_BOOL FeatureCaps::FormContactFeatures(
        IN IMS_SINT32 nSipMethod, IN IMS_BOOL bRequest, OUT AString& strContactFeatures)
{
    if (!HasAdditionalFeatures(nSipMethod, bRequest))
    {
        if ((m_pRegCaps != IMS_NULL) && !m_pRegCaps->IsEmpty())
        {
            strContactFeatures = m_pRegCaps->ToString();
        }

        return (strContactFeatures.GetLength() > 0) ? IMS_TRUE : IMS_FALSE;
    }

    CallerCapability* pCc = new CallerCapability(0);

    // REG-CAPS
    if ((m_pRegCaps != IMS_NULL) && !m_pRegCaps->IsEmpty())
    {
        pCc->AddFeatures(m_pRegCaps);
    }

    // REG-EXCLUDED-CAPS
    if ((m_pExcludedFeaturesForRegCaps != IMS_NULL) && !m_pExcludedFeaturesForRegCaps->IsEmpty())
    {
        pCc->RemoveFeatures(m_pExcludedFeaturesForRegCaps, IMS_FALSE);
    }

    // ALL-MESSAGE-CAPS
    if ((m_pFeaturesForAllMessage != IMS_NULL) && !m_pFeaturesForAllMessage->IsEmpty())
    {
        pCc->AddFeatures(m_pFeaturesForAllMessage);
    }

    const CallerCapability* pFeatures = IMS_NULL;

    // REQUEST-CAPS
    if (bRequest)
    {
        pFeatures = GetFeaturesForRequest(nSipMethod);
    }
    // RESPONSE-CAPS
    else
    {
        pFeatures = GetFeaturesForResponse(nSipMethod);
    }

    if ((pFeatures != IMS_NULL) && !pFeatures->IsEmpty())
    {
        pCc->AddFeatures(pFeatures);
    }

    if (!pCc->IsEmpty())
    {
        strContactFeatures = pCc->ToString();
    }

    delete pCc;

    return (strContactFeatures.GetLength() > 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC
void FeatureCaps::UpdateRegCaps(IN const CallerCapability* pRegCaps)
{
    m_pRegCaps->Clear();
    m_pRegCaps->AddFeatures(pRegCaps);
}

PRIVATE
CallerCapability* FeatureCaps::GetExcludedFeaturesForRegCaps(IN IMS_BOOL bCreate /*= IMS_FALSE*/)
{
    if (m_pExcludedFeaturesForRegCaps == IMS_NULL)
    {
        if (bCreate)
        {
            m_pExcludedFeaturesForRegCaps = new CallerCapability(0);
        }
    }

    return m_pExcludedFeaturesForRegCaps;
}

PRIVATE
CallerCapability* FeatureCaps::GetFeaturesForAllMessage(IN IMS_BOOL bCreate /*= IMS_FALSE*/)
{
    if (m_pFeaturesForAllMessage == IMS_NULL)
    {
        if (bCreate)
        {
            m_pFeaturesForAllMessage = new CallerCapability(0);
        }
    }

    return m_pFeaturesForAllMessage;
}

PRIVATE
CallerCapability* FeatureCaps::GetFeaturesForRequest(
        IN IMS_SINT32 nSipMethod, IN IMS_BOOL bCreate /*= IMS_FALSE*/)
{
    if (m_pFeaturesForRequest == IMS_NULL)
    {
        if (bCreate)
        {
            m_pFeaturesForRequest = new ImsMap<IMS_SINT32, CallerCapability*>();
        }
    }

    if (m_pFeaturesForRequest == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_SLONG nIndex = m_pFeaturesForRequest->GetIndexOfKey(nSipMethod);

    if (nIndex < 0)
    {
        if (bCreate)
        {
            CallerCapability* pCc = new CallerCapability(0);
            m_pFeaturesForRequest->Add(nSipMethod, pCc);
            return pCc;
        }
        else
        {
            return IMS_NULL;
        }
    }
    else
    {
        return m_pFeaturesForRequest->GetValueAt(nIndex);
    }
}

PRIVATE
CallerCapability* FeatureCaps::GetFeaturesForResponse(
        IN IMS_SINT32 nSipMethod, IN IMS_BOOL bCreate /*= IMS_FALSE*/)
{
    if (m_pFeaturesForResponse == IMS_NULL)
    {
        if (bCreate)
        {
            m_pFeaturesForResponse = new ImsMap<IMS_SINT32, CallerCapability*>();
        }
    }

    if (m_pFeaturesForResponse == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_SLONG nIndex = m_pFeaturesForResponse->GetIndexOfKey(nSipMethod);

    if (nIndex < 0)
    {
        if (bCreate)
        {
            CallerCapability* pCc = new CallerCapability(0);
            m_pFeaturesForResponse->Add(nSipMethod, pCc);
            return pCc;
        }
        else
        {
            return IMS_NULL;
        }
    }
    else
    {
        return m_pFeaturesForResponse->GetValueAt(nIndex);
    }
}

PRIVATE
IMS_BOOL FeatureCaps::HasAdditionalFeatures(IN IMS_SINT32 nSipMethod, IN IMS_BOOL bRequest)
{
    // REG-EXCLUDED-CAPS
    if ((m_pExcludedFeaturesForRegCaps != IMS_NULL) && !m_pExcludedFeaturesForRegCaps->IsEmpty())
    {
        return IMS_TRUE;
    }

    // ALL-MESSAGE-CAPS
    if ((m_pFeaturesForAllMessage != IMS_NULL) && !m_pFeaturesForAllMessage->IsEmpty())
    {
        return IMS_TRUE;
    }

    const CallerCapability* pFeatures = IMS_NULL;

    // REQUEST-CAPS
    if (bRequest)
    {
        pFeatures = GetFeaturesForRequest(nSipMethod);
    }
    // RESPONSE-CAPS
    else
    {
        pFeatures = GetFeaturesForResponse(nSipMethod);
    }

    if ((pFeatures != IMS_NULL) && !pFeatures->IsEmpty())
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
