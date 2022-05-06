/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090609  toastops@                 Created
    </table>

    Description

*/

#ifndef _CALLER_CAPABILITY_H_
#define _CALLER_CAPABILITY_H_

#include "AString.h"

class AppConfig;
class CoreServiceConfig;
class Feature;
class FeatureSet;
class ISipConfigV;

class CallerCapability
{
public:
    explicit CallerCapability(IN IMS_UINT32 nID_);
    ~CallerCapability();

private:
    CallerCapability(IN CONST CallerCapability& objRHS);
    CallerCapability& operator=(IN CONST CallerCapability& objRHS);

public:
    IMS_BOOL AddFeature(IN CONST Feature* pFeature);
    IMS_BOOL AddFeature(IN CONST FeatureSet* pFeatureSet);
    IMS_BOOL AddFeatures(IN CONST CallerCapability* pCC);
    void Clear();
    IMS_BOOL Create(IN CONST AppConfig* pAppConfig, IN CONST CoreServiceConfig* pServiceConfig,
            IN CONST ISipConfigV* piSipConfigV);
    IMS_BOOL Equals(IN CONST CallerCapability* pCC) const;
    const IMSList<FeatureSet*>& GetFeatures() const;
    IMS_BOOL HasFeature(IN CONST Feature* pFeature) const;
    IMS_BOOL IsEmpty() const;
    IMS_BOOL RemoveFeature(IN CONST Feature* pFeature);
    IMS_BOOL RemoveFeature(IN CONST FeatureSet* pFeatureSet);
    IMS_BOOL RemoveFeatures(IN CONST CallerCapability* pCC, IN IMS_BOOL bRemoveRef = IMS_TRUE);
    AString ToString() const;

private:
    IMS_BOOL Attach(IN CONST AString& strTag, IN CONST AString& strValue = AString::ConstNull());
    void Detach(IN CONST AString& strTag);
    FeatureSet* Lookup(IN CONST AString& strTag) const;

    IMS_SINT32 AddFeature(IN CONST AString& strTag);
    IMS_SINT32 AddFeature(IN CONST AString& strTag, IN CONST AString& strValue);
    IMS_SINT32 RemoveFeature(IN CONST AString& strTag);
    IMS_SINT32 RemoveFeature(IN CONST AString& strTag, IN CONST AString& strValue);

private:
    enum
    {
        FEATURE_UNCHANGED = 0x01,
        FEATURE_CHANGED = 0x02
    };

    IMS_UINT32 nID;
    IMSList<FeatureSet*> objContactFeatures;
};

#endif  // _CALLER_CAPABILITY_H_
