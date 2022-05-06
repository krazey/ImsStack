/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140714  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _FEATURE_CAPS_H_
#define _FEATURE_CAPS_H_

#include "IMSMap.h"
#include "IFeatureCaps.h"

class CallerCapability;

class FeatureCaps : public IFeatureCaps
{
public:
    FeatureCaps();
    virtual ~FeatureCaps();

private:
    FeatureCaps(IN CONST FeatureCaps& objRHS);
    FeatureCaps& operator=(IN CONST FeatureCaps& objRHS);

public:
    // IFeatureCaps class
    virtual void AddFeature(IN CONST AString& strName, IN CONST AString& strValue);
    virtual void AddFeature(IN CONST AString& strName, IN CONST AString& strValue,
            IN IMS_SINT32 nSIPMethod, IN IMS_SINT32 nMessageType = 2 /* ANY */);
    virtual void RemoveFeature(IN CONST AString& strName, IN CONST AString& strValue);
    virtual void RemoveFeature(IN CONST AString& strName, IN CONST AString& strValue,
            IN IMS_SINT32 nSIPMethod, IN IMS_SINT32 nMessageType = 2 /* ANY */);
    virtual void RemoveAllFeatures();

    virtual void AddExcludedFeatureForRegCaps(
            IN CONST AString& strName, IN CONST AString& strValue);
    virtual void RemoveExcludedFeatureForRegCaps(
            IN CONST AString& strName, IN CONST AString& strValue);
    virtual void RemoveAllExcludedFeaturesForRegCaps();

    IMS_BOOL FormContactFeatures(
            IN IMS_SINT32 nSIPMethod, IN IMS_BOOL bRequest, OUT AString& strContactFeatures);
    void UpdateRegCaps(IN CallerCapability* pRegCaps);

private:
    CallerCapability* GetExcludedFeaturesForRegCaps(IN IMS_BOOL bCreate = IMS_FALSE);
    CallerCapability* GetFeaturesForAllMessage(IN IMS_BOOL bCreate = IMS_FALSE);
    CallerCapability* GetFeaturesForRequest(
            IN IMS_SINT32 nSIPMethod, IN IMS_BOOL bCreate = IMS_FALSE);
    CallerCapability* GetFeaturesForResponse(
            IN IMS_SINT32 nSIPMethod, IN IMS_BOOL bCreate = IMS_FALSE);
    IMS_BOOL HasAdditionalFeatures(IN IMS_SINT32 nSIPMethod, IN IMS_BOOL bRequest);

private:
    CallerCapability* pExcludedFeaturesForRegCaps;
    CallerCapability* pFeaturesForAllMessage;
    IMSMap<IMS_SINT32, CallerCapability*>* pFeaturesForRequest;
    IMSMap<IMS_SINT32, CallerCapability*>* pFeaturesForResponse;

    CallerCapability* pRegCaps;
};

#endif  // _FEATURE_CAPS_H_
