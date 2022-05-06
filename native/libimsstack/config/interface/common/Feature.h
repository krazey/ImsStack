/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#ifndef _FEATURE_H_
#define _FEATURE_H_

#include "RCObject.h"
#include "AStringArray.h"
#include "ServiceIdentifier.h"

class Feature : public RCObject
{
public:
    explicit Feature(IN const AString& strFeature_);
    Feature(IN const AString& strTag_, IN const AString& strValue_);
    Feature(IN const Feature& objRHS);
    virtual ~Feature();

public:
    Feature& operator=(IN const Feature& objRHS);

public:
    IMS_BOOL Equals(IN const Feature* pOther) const;
    const AString& GetTag() const;
    const AString& GetValue() const;
    IMS_BOOL IsSameTag(IN const Feature* pOther) const;
    IMS_BOOL IsTagOnly() const;
    AString ToString() const;
    AString ToStringValueOnly() const;

    static const IMS_CHAR* GetFeatureTag(IN IMS_SINT32 nBaseTag);
    static IMS_BOOL IsFeatureTag(IN const AString& strName);

private:
    void ExtractProperties(IN const AString& strFeature);
    static AString DoPercentDecoding(IN const AString& strValue);
    static AString DoPercentEncoding(IN const AString& strValue);
    static IMS_BOOL IsBaseTag(IN const AString& strName);
    static AString StripPrefixInSIPTree(IN const AString& strName);

public:
    // Base Tags
    enum
    {
        BASE_AUDIO = 0,
        BASE_APPLICATION,
        BASE_DATA,
        BASE_CONTROL,
        BASE_VIDEO,
        BASE_TEXT,
        BASE_AUTOMATA,
        BASE_CLASS,
        BASE_DUPLEX,
        BASE_MOBILITY,
        BASE_DESCRIPTION,
        BASE_EVENTS,
        BASE_PRIORITY,
        BASE_METHODS,
        BASE_EXTENSIONS,
        BASE_SCHEMES,
        BASE_ACTOR,
        BASE_ISFOCUS,
        BASE_LANGUAGE,
        BASE_TYPE,
        BASE_MAX
    };

    static const IMS_CHAR* BASE_TAG[BASE_MAX];

    // Other Tags
    static const IMS_CHAR OTHER_APP_SUBTYPE[];
    static const IMS_CHAR OTHER_MESSAGE[];
    static const IMS_CHAR OTHER_G_3GPP_IARI_REF[];
    static const IMS_CHAR OTHER_G_3GPP_ICSI_REF[];

    // Parameters
    static const IMS_CHAR FLAG_EXPLICIT[];
    static const IMS_CHAR FLAG_REQUIRE[];

    // Prefix for IARI & ICSI
    static const IMS_CHAR PREFIX_3GPP_IARI[];
    static const IMS_CHAR PREFIX_3GPP_ICSI[];

private:
    enum
    {
        NONE = 0x00,
        // Tag-Value
        BOOLEAN = 0x01,
        NUMERIC = 0x02,
        TOKEN_NOBANG = 0x04,
        // String-Value: starts with LAQUOT & ends with RAQUOT
        STRING_VALUE = 0x08,
        // QUOTED_STRING = 0x10,
        //  IARI
        IARI = 0x20,
        // ICSI
        ICSI = 0x40
    };

    IMS_SINT32 nType;
    IMS_BOOL bCaseSensitivity;
    AString strTag;
    AString strValue;
};

class FeatureSet
{
public:
    explicit FeatureSet(IN const AString& strTag_);
    FeatureSet(IN const AString& strTag_, IN const AString& strValues_);
    ~FeatureSet();

private:
    FeatureSet(IN const FeatureSet& objRHS);
    FeatureSet& operator=(IN const FeatureSet& objRHS);

public:
    IMS_SINT32 Add(IN const AString& strTag);
    IMS_SINT32 Add(IN const AString& strTag, IN const AString& strValue);
    IMS_BOOL Contains(IN const Feature* pFeature) const;
    IMS_BOOL Contains(IN const AString& strValue) const;
    const AString& GetTag() const;
    const IMSList<Feature*>& GetFeatures() const;
    IMS_BOOL IsEmpty() const;
    Feature* Lookup(IN const Feature* pFeature, IN IMS_BOOL bDetach = IMS_FALSE);
    IMS_SINT32 Remove(IN const AString& strTag);
    IMS_SINT32 Remove(IN const AString& strTag, IN const AString& strValue);
    AString ToString() const;

    static FeatureSet* FromServiceIdentifier(IN const ServiceIdentifier& objServiceId);

private:
    IMS_SINT32 Add(IN Feature* pFeature);
    void AddFeatures(IN const AString& strValues);
    IMS_SINT32 Remove(IN const Feature* pFeature);

public:
    enum
    {
        OP_FAIL = (-1),
        OP_ADD,
        OP_ADD_REF,
        OP_REMOVE,
        OP_REMOVE_REF
    };

private:
    AString strTag;
    IMSList<Feature*> objFeatures;
};

#endif  // _FEATURE_H_
