/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140714  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "Feature.h"
#include "ISipMessage.h"
#include "SipMethod.h"
#include "util/CallerCapability.h"
#include "FeatureCaps.h"

__IMS_TRACE_TAG_IMS__;



PUBLIC
FeatureCaps::FeatureCaps()
    : pExcludedFeaturesForRegCaps(IMS_NULL)
    , pFeaturesForAllMessage(IMS_NULL)
    , pFeaturesForRequest(IMS_NULL)
    , pFeaturesForResponse(IMS_NULL)
    , pRegCaps(new CallerCapability(0))
{
}

PUBLIC VIRTUAL
FeatureCaps::~FeatureCaps()
{
    if (pExcludedFeaturesForRegCaps != IMS_NULL)
    {
        delete pExcludedFeaturesForRegCaps;
    }

    if (pFeaturesForAllMessage != IMS_NULL)
    {
        delete pFeaturesForAllMessage;
    }

    if (pRegCaps != IMS_NULL)
    {
        delete pRegCaps;
    }

    if (pFeaturesForRequest != IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < pFeaturesForRequest->GetSize(); ++i)
        {
            CallerCapability *pCC = pFeaturesForRequest->GetValueAt(i);

            if (pCC != IMS_NULL)
            {
                delete pCC;
            }
        }

        delete pFeaturesForRequest;
    }

    if (pFeaturesForResponse != IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < pFeaturesForResponse->GetSize(); ++i)
        {
            CallerCapability *pCC = pFeaturesForResponse->GetValueAt(i);

            if (pCC != IMS_NULL)
            {
                delete pCC;
            }
        }

        delete pFeaturesForResponse;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void FeatureCaps::AddFeature(IN CONST AString &strName, IN CONST AString &strValue)
{
    CallerCapability *pFeatures = GetFeaturesForAllMessage(IMS_TRUE);

    //---------------------------------------------------------------------------------------------

    if (pFeatures != IMS_NULL)
    {
        Feature objFeature(strName, strValue);

        pFeatures->AddFeature(&objFeature);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void FeatureCaps::AddFeature(IN CONST AString &strName, IN CONST AString &strValue,
        IN IMS_SINT32 nSIPMethod, IN IMS_SINT32 nMessageType /* = 2 (ANY) */)
{
    //---------------------------------------------------------------------------------------------

    if ((nSIPMethod != SIPMethod::INVITE)
            && (nSIPMethod != SIPMethod::SUBSCRIBE)
            && (nSIPMethod != SIPMethod::REFER)
            && (nSIPMethod != SIPMethod::NOTIFY)
            && (nSIPMethod != SIPMethod::OPTIONS)
            && (nSIPMethod != SIPMethod::PUBLISH))
    {
        IMS_TRACE_E(0, "FeatureCaps :: Method(%d) is not allowed", nSIPMethod, 0, 0);
        return;
    }

    if ((nMessageType < ISIPMessage::TYPE_REQUEST) || (nMessageType > ISIPMessage::TYPE_ANY))
    {
        IMS_TRACE_E(0, "FeatureCaps :: Message type(%d) is not allowed", nMessageType, 0, 0);
        return;
    }

    CallerCapability *pFeatures = IMS_NULL;
    Feature objFeature(strName, strValue);

    if (nMessageType == ISIPMessage::TYPE_ANY)
    {
        // REQUEST
        pFeatures = GetFeaturesForRequest(nSIPMethod, IMS_TRUE);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->AddFeature(&objFeature);
        }

        // RESPONSE
        pFeatures = GetFeaturesForResponse(nSIPMethod, IMS_TRUE);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->AddFeature(&objFeature);
        }
    }
    else if (nMessageType == ISIPMessage::TYPE_REQUEST)
    {
        pFeatures = GetFeaturesForRequest(nSIPMethod, IMS_TRUE);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->AddFeature(&objFeature);
        }
    }
    else if (nMessageType == ISIPMessage::TYPE_RESPONSE)
    {
        pFeatures = GetFeaturesForResponse(nSIPMethod, IMS_TRUE);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->AddFeature(&objFeature);
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void FeatureCaps::RemoveFeature(IN CONST AString &strName, IN CONST AString &strValue)
{
    CallerCapability *pFeatures = GetFeaturesForAllMessage();

    //---------------------------------------------------------------------------------------------

    if (pFeatures != IMS_NULL)
    {
        Feature objFeature(strName, strValue);

        pFeatures->RemoveFeature(&objFeature);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void FeatureCaps::RemoveFeature(IN CONST AString &strName, IN CONST AString &strValue,
        IN IMS_SINT32 nSIPMethod, IN IMS_SINT32 nMessageType /* = 2 (ANY) */)
{
    //---------------------------------------------------------------------------------------------

    if ((nSIPMethod != SIPMethod::INVITE)
            && (nSIPMethod != SIPMethod::SUBSCRIBE)
            && (nSIPMethod != SIPMethod::REFER)
            && (nSIPMethod != SIPMethod::NOTIFY)
            && (nSIPMethod != SIPMethod::OPTIONS)
            && (nSIPMethod != SIPMethod::PUBLISH))
    {
        IMS_TRACE_E(0, "FeatureCaps :: Method(%d) is not allowed", nSIPMethod, 0, 0);
        return;
    }

    if ((nMessageType < ISIPMessage::TYPE_REQUEST) || (nMessageType > ISIPMessage::TYPE_ANY))
    {
        IMS_TRACE_E(0, "FeatureCaps :: Message type(%d) is not allowed", nMessageType, 0, 0);
        return;
    }

    CallerCapability *pFeatures = IMS_NULL;
    Feature objFeature(strName, strValue);

    if (nMessageType == ISIPMessage::TYPE_ANY)
    {
        // REQUEST
        pFeatures = GetFeaturesForRequest(nSIPMethod);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->RemoveFeature(&objFeature);
        }

        // RESPONSE
        pFeatures = GetFeaturesForResponse(nSIPMethod);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->RemoveFeature(&objFeature);
        }
    }
    else if (nMessageType == ISIPMessage::TYPE_REQUEST)
    {
        pFeatures = GetFeaturesForRequest(nSIPMethod);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->RemoveFeature(&objFeature);
        }
    }
    else if (nMessageType == ISIPMessage::TYPE_RESPONSE)
    {
        pFeatures = GetFeaturesForResponse(nSIPMethod);

        if (pFeatures != IMS_NULL)
        {
            pFeatures->RemoveFeature(&objFeature);
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void FeatureCaps::RemoveAllFeatures()
{
    //---------------------------------------------------------------------------------------------

    if (pFeaturesForAllMessage != IMS_NULL)
    {
        pFeaturesForAllMessage->Clear();
    }

    if (pFeaturesForRequest != IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < pFeaturesForRequest->GetSize(); ++i)
        {
            CallerCapability *pCC = pFeaturesForRequest->GetValueAt(i);

            if (pCC != IMS_NULL)
            {
                pCC->Clear();
            }
        }
    }

    if (pFeaturesForResponse != IMS_NULL)
    {
        for (IMS_UINT32 i = 0; i < pFeaturesForResponse->GetSize(); ++i)
        {
            CallerCapability *pCC = pFeaturesForResponse->GetValueAt(i);

            if (pCC != IMS_NULL)
            {
                pCC->Clear();
            }
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void FeatureCaps::AddExcludedFeatureForRegCaps(IN CONST AString &strName,
        IN CONST AString &strValue)
{
    CallerCapability *pFeatures = GetExcludedFeaturesForRegCaps(IMS_TRUE);

    //---------------------------------------------------------------------------------------------

    if (pFeatures != IMS_NULL)
    {
        Feature objFeature(strName, strValue);

        pFeatures->AddFeature(&objFeature);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void FeatureCaps::RemoveExcludedFeatureForRegCaps(IN CONST AString &strName,
        IN CONST AString &strValue)
{
    CallerCapability *pFeatures = GetExcludedFeaturesForRegCaps();

    //---------------------------------------------------------------------------------------------

    if (pFeatures != IMS_NULL)
    {
        Feature objFeature(strName, strValue);

        pFeatures->RemoveFeature(&objFeature);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void FeatureCaps::RemoveAllExcludedFeaturesForRegCaps()
{
    //---------------------------------------------------------------------------------------------

    if (pExcludedFeaturesForRegCaps != IMS_NULL)
    {
        pExcludedFeaturesForRegCaps->Clear();
    }
}

/*

Remarks

*/
PUBLIC
IMS_BOOL FeatureCaps::FormContactFeatures(IN IMS_SINT32 nSIPMethod, IN IMS_BOOL bRequest,
        OUT AString &strContactFeatures)
{
    //---------------------------------------------------------------------------------------------

    if (!HasAdditionalFeatures(nSIPMethod, bRequest))
    {
        if ((pRegCaps != IMS_NULL) && !pRegCaps->IsEmpty())
        {
            strContactFeatures = pRegCaps->ToString();
        }

        return (strContactFeatures.GetLength() > 0) ? IMS_TRUE : IMS_FALSE;
    }

    CallerCapability *pCC = new CallerCapability(0);

    // REG-CAPS
    if ((pRegCaps != IMS_NULL) && !pRegCaps->IsEmpty())
    {
        pCC->AddFeatures(pRegCaps);
    }

    // REG-EXCLUDED-CAPS
    if ((pExcludedFeaturesForRegCaps != IMS_NULL)
            && !pExcludedFeaturesForRegCaps->IsEmpty())
    {
        pCC->RemoveFeatures(pExcludedFeaturesForRegCaps, IMS_FALSE);
    }

    // ALL-MESSAGE-CAPS
    if ((pFeaturesForAllMessage != IMS_NULL)
            && !pFeaturesForAllMessage->IsEmpty())
    {
        pCC->AddFeatures(pFeaturesForAllMessage);
    }

    CallerCapability *pFeatures = IMS_NULL;

    // REQUEST-CAPS
    if (bRequest)
    {
        pFeatures = GetFeaturesForRequest(nSIPMethod);
    }
    // RESPONSE-CAPS
    else
    {
        pFeatures = GetFeaturesForResponse(nSIPMethod);
    }

    if ((pFeatures != IMS_NULL) && !pFeatures->IsEmpty())
    {
        pCC->AddFeatures(pFeatures);
    }

    if (!pCC->IsEmpty())
    {
        strContactFeatures = pCC->ToString();
    }

    delete pCC;

    return (strContactFeatures.GetLength() > 0) ? IMS_TRUE : IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
void FeatureCaps::UpdateRegCaps(IN CallerCapability *pRegCaps)
{
    //---------------------------------------------------------------------------------------------

    this->pRegCaps->Clear();
    this->pRegCaps->AddFeatures(pRegCaps);
}

/*

Remarks

*/
PRIVATE
CallerCapability* FeatureCaps::GetExcludedFeaturesForRegCaps(
        IN IMS_BOOL bCreate /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if (pExcludedFeaturesForRegCaps == IMS_NULL)
    {
        if (bCreate)
        {
            pExcludedFeaturesForRegCaps = new CallerCapability(0);
        }
    }

    return pExcludedFeaturesForRegCaps;
}

/*

Remarks

*/
PRIVATE
CallerCapability* FeatureCaps::GetFeaturesForAllMessage(
        IN IMS_BOOL bCreate /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if (pFeaturesForAllMessage == IMS_NULL)
    {
        if (bCreate)
        {
            pFeaturesForAllMessage = new CallerCapability(0);
        }
    }

    return pFeaturesForAllMessage;
}

/*

Remarks

*/
PRIVATE
CallerCapability* FeatureCaps::GetFeaturesForRequest(IN IMS_SINT32 nSIPMethod,
        IN IMS_BOOL bCreate /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if (pFeaturesForRequest == IMS_NULL)
    {
        if (bCreate)
        {
            pFeaturesForRequest = new IMSMap<IMS_SINT32, CallerCapability*>();
        }
    }

    if (pFeaturesForRequest == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_SLONG nIndex = pFeaturesForRequest->GetIndexOfKey(nSIPMethod);

    if (nIndex < 0)
    {
        if (bCreate)
        {
            CallerCapability *pCC = new CallerCapability(0);

            if (!pFeaturesForRequest->Add(nSIPMethod, pCC))
            {
                delete pCC;
                return IMS_NULL;
            }

            return pCC;
        }
        else
        {
            return IMS_NULL;
        }
    }
    else
    {
        return pFeaturesForRequest->GetValueAt(nIndex);
    }
}

/*

Remarks

*/
PRIVATE
CallerCapability* FeatureCaps::GetFeaturesForResponse(IN IMS_SINT32 nSIPMethod,
        IN IMS_BOOL bCreate /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if (pFeaturesForResponse == IMS_NULL)
    {
        if (bCreate)
        {
            pFeaturesForResponse = new IMSMap<IMS_SINT32, CallerCapability*>();
        }
    }

    if (pFeaturesForResponse == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_SLONG nIndex = pFeaturesForResponse->GetIndexOfKey(nSIPMethod);

    if (nIndex < 0)
    {
        if (bCreate)
        {
            CallerCapability *pCC = new CallerCapability(0);

            if (!pFeaturesForResponse->Add(nSIPMethod, pCC))
            {
                delete pCC;
                return IMS_NULL;
            }

            return pCC;
        }
        else
        {
            return IMS_NULL;
        }
    }
    else
    {
        return pFeaturesForResponse->GetValueAt(nIndex);
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL FeatureCaps::HasAdditionalFeatures(IN IMS_SINT32 nSIPMethod, IN IMS_BOOL bRequest)
{
    //---------------------------------------------------------------------------------------------

    // REG-EXCLUDED-CAPS
    if ((pExcludedFeaturesForRegCaps != IMS_NULL)
            && !pExcludedFeaturesForRegCaps->IsEmpty())
    {
        return IMS_TRUE;
    }

    // ALL-MESSAGE-CAPS
    if ((pFeaturesForAllMessage != IMS_NULL)
            && !pFeaturesForAllMessage->IsEmpty())
    {
        return IMS_TRUE;
    }

    CallerCapability *pFeatures = IMS_NULL;

    // REQUEST-CAPS
    if (bRequest)
    {
        pFeatures = GetFeaturesForRequest(nSIPMethod);
    }
    // RESPONSE-CAPS
    else
    {
        pFeatures = GetFeaturesForResponse(nSIPMethod);
    }

    if ((pFeatures != IMS_NULL) && !pFeatures->IsEmpty())
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
