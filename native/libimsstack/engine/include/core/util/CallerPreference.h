/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090609  toastops@                 Created
    </table>

    Description

*/

#ifndef _CALLER_PREFERENCE_H_
#define _CALLER_PREFERENCE_H_

#include "AString.h"

class AppConfig;
class CoreServiceConfig;
class Feature;
class FeatureSet;
class ISipConfigV;
class PreferenceHeader;

class CallerPreference
{
public:
    static IMS_BOOL CreateAcceptContactHeaders(IN CONST AppConfig* pAppConfig,
            IN CONST CoreServiceConfig* pServiceConfig, IN CONST ISipConfigV* piSipConfigV,
            OUT IMSList<PreferenceHeader*>& objHeaders);
    static IMS_SINT32 GetCandidateScore(IN CONST AppConfig* pAppConfig,
            IN CONST CoreServiceConfig* pServiceConfig,
            IN CONST IMSList<PreferenceHeader*>& objHeaders,
            IN CONST IMSList<FeatureSet*>& objExtraFeatures);

private:
    static void AddFeature(
            IN CONST AString& strFeature, OUT IMSList<PreferenceHeader*>& objHeaders);
    static void AddFeature(IN CONST AString& strTag, IN CONST AString& strValue,
            OUT IMSList<PreferenceHeader*>& objHeaders);
    static void AddFeature(IN CONST AString& strTag, IN CONST AString& strValue,
            IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire,
            OUT IMSList<PreferenceHeader*>& objHeaders);
    static void AddFeature(IN IMS_BOOL bBooleanFeature, IN CONST AString& strTag,
            IN CONST AString& strValue, IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire,
            OUT IMSList<PreferenceHeader*>& objHeaders);

public:
    enum
    {
        SCORE_INVALID = -1
    };
};

#endif  // _CALLER_PREFERENCE_H_
