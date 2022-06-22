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
#ifndef FEATURE_H_
#define FEATURE_H_

#include "AStringArray.h"
#include "RCObject.h"

#include "ServiceIdentifier.h"

class Feature : public RCObject
{
public:
    explicit Feature(IN const AString& strFeature);
    Feature(IN const AString& strTag, IN const AString& strValue);
    Feature(IN const Feature& other);
    virtual ~Feature();

public:
    Feature& operator=(IN const Feature& other);

public:
    IMS_BOOL Equals(IN const Feature* pOther) const;
    inline const AString& GetTag() const { return m_strTag; }
    inline const AString& GetValue() const { return m_strValue; }
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
    static AString StripPrefixInSipTree(IN const AString& strName);

public:
    /// Base Tags
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
        /// Tag-Value
        BOOLEAN = 0x01,
        NUMERIC = 0x02,
        TOKEN_NOBANG = 0x04,
        /// String-Value: starts with LAQUOT & ends with RAQUOT
        STRING_VALUE = 0x08,
        // QUOTED_STRING = 0x10,
        ///  IARI
        IARI = 0x20,
        /// ICSI
        ICSI = 0x40
    };

    IMS_SINT32 m_nType;
    IMS_BOOL m_bCaseSensitivity;
    AString m_strTag;
    AString m_strValue;
};

class FeatureSet
{
public:
    explicit FeatureSet(IN const AString& strTag);
    FeatureSet(IN const AString& strTag, IN const AString& strValues);
    ~FeatureSet();

    FeatureSet(IN const FeatureSet&) = delete;
    FeatureSet& operator=(IN const FeatureSet&) = delete;

public:
    IMS_SINT32 Add(IN const AString& strTag);
    IMS_SINT32 Add(IN const AString& strTag, IN const AString& strValue);
    IMS_BOOL Contains(IN const Feature* pFeature) const;
    IMS_BOOL Contains(IN const AString& strValue) const;
    inline const AString& GetTag() const { return m_strTag; }
    inline const IMSList<Feature*>& GetFeatures() const { return m_objFeatures; }
    inline IMS_BOOL IsEmpty() const { return (m_objFeatures.GetSize() == 0); }
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
    AString m_strTag;
    IMSList<Feature*> m_objFeatures;
};

#endif
