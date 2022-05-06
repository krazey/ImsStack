/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#ifndef _PREFERENCE_HEADER_H_
#define _PREFERENCE_HEADER_H_

#include "AStringArray.h"

class FeatureSet;
class ISipHeader;

class PreferenceHeader
{
public:
    explicit PreferenceHeader(IN CONST AString& strHeader);
    explicit PreferenceHeader(IN CONST ISipHeader* piHeader);
    PreferenceHeader(IN IMS_BOOL bExplicit_, IN IMS_BOOL bRequire_);
    ~PreferenceHeader();

private:
    PreferenceHeader(IN CONST PreferenceHeader& objRHS);
    PreferenceHeader& operator=(IN CONST PreferenceHeader& objRHS);

public:
    IMS_BOOL AddFeature(IN CONST AString& strTag);
    IMS_BOOL AddFeature(IN CONST AString& strTag, IN CONST AString& strValue);
    IMS_BOOL Contains(IN CONST AString& strTag) const;
    IMS_BOOL Contains(IN CONST AString& strTag, IN CONST AString& strValue) const;
    const IMSList<FeatureSet*>& GetFeatureSets() const;
    IMS_BOOL IsExplicitPresent() const;
    IMS_BOOL IsRequirePresent() const;
    AString ToString() const;

private:
    IMS_BOOL Attach(IN CONST AString& strTag, IN CONST AString& strValue = AString::ConstNull());
    void Detach(IN CONST AString& strTag);
    FeatureSet* Lookup(IN CONST AString& strTag) const;

    void ExtractProperties(IN CONST AString& strFeatureSet);

private:
    IMSList<FeatureSet*> objPreferenceFeatures;
    IMS_BOOL bExplicit;
    IMS_BOOL bRequire;
};

#endif  // _PREFERENCE_HEADER_H_
