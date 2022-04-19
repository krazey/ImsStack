#ifndef _AOS_FEATURE_TAG_H_
#define _AOS_FEATURE_TAG_H_

#include "IMSTypeDef.h"
#include "IMSList.h"
#include "AString.h"

#include "ImsAosParameter.h"

class AosFeatureTag
{
public:
    AosFeatureTag(IN CONST AString& strName,
            IN CONST AString& strValue = AString::ConstNull(), IN IMS_UINT32 nType = 0,
            IN IMS_UINT32 nOption = OPTION_HEADER_PARAMETER);
    virtual ~AosFeatureTag();

    void SetFeatureTag(IN CONST AString& strName,
            IN CONST AString& strValue = AString::ConstNull());

    IMS_BOOL Equals(IN AosFeatureTag* pFeatureTag);
    IMS_BOOL Equals(IN CONST AString& strName, IN CONST AString& strValue = AString::ConstNull());

    AString& GetName();
    AString& GetValue();
    IMS_UINT32 GetType();
    IMS_UINT32 GetOption();

    enum
    {
        OPTION_HEADER_PARAMETER = 0,
        OPTION_EXTRA_CAPABILITY
    };

private:
    AString m_strName;
    AString m_strValue;
    IMS_UINT32 m_nType;
    IMS_UINT32 m_nOption;
};

class AosFeatureTagList
{
public:
    AosFeatureTagList();
    virtual ~AosFeatureTagList();

    IMS_BOOL AddFeatureTag(IN CONST AString& strName,
            IN CONST AString& strValue = AString::ConstNull(), IN IMS_UINT32 nType = 0,
            IN IMS_UINT32 nOption = AosFeatureTag::OPTION_HEADER_PARAMETER);
    IMS_BOOL RemoveFeatureTag(IN CONST AString& strName,
            IN CONST AString& strValue = AString::ConstNull());

    void AddFeature(IN IMS_UINT32 nFeature);
    void RemoveFeature(IN IMS_UINT32 nFeature);

    IMS_UINT32 GetFeatures();
    void ClearFeatures();
    void ClearFeatureTags();
    void Clear();

    IMS_BOOL Equals(IN AosFeatureTagList& objTargetList);
    void Copy(IN AosFeatureTagList& objSourceList);
    void CopyFeatures(IN IMS_UINT32 nFeatures);
    void CopyFeatureTags(IN IMSList<ImsAosFeatureTag*>& objFeatureTag);

    IMS_UINT32 GetSize();
    AosFeatureTag* GetAt(IN IMS_UINT32 nIndex);

    IMS_BOOL HasFeature(IN IMS_UINT32 nFeature);

    void PrintFeatureTagList();

private:
    IMSList<AosFeatureTag*> m_objFeatureTagList;
    IMS_UINT32 m_nFeatures;
};
#endif // _AOS_FEATURE_TAG_H_
