#ifndef MTC_CONFIGURATION_PROXY_H_
#define MTC_CONFIGURATION_PROXY_H_

#include "IMSTypeDef.h"
#include "AString.h"
#include "configuration/MtcConfigurationManager.h"
#include "configuration/ConfigDef.h"

class ConfigCache;

class MtcConfigurationProxy
{
public:
    MtcConfigurationProxy();
    ~MtcConfigurationProxy();
    MtcConfigurationProxy(IN const MtcConfigurationProxy&) = delete;
    MtcConfigurationProxy& operator=(IN const MtcConfigurationProxy&) = delete;

    void Init();

    IMS_BOOL Is(IN Feature eFeature) const;
    IMS_BOOL Is(IN Feature eFeature, IN const AString& strAdditionalInfo) const;
    IMS_BOOL Is(IN Feature eFeature, IN IMS_SINT32 nAdditionalInfo) const;
    IMS_SINT32 GetInt(IN Feature eFeature) const;
    IMS_SINT32 GetInt(IN Feature eFeature, IN IMS_BOOL bWParam, IN IMS_BOOL bLParam) const;
    const AString GetStr(IN Feature eFeature, IN IMS_SINT32 nAdditionalInfo) const;

    void PutConfigCache(IN Feature eFeature, IN IMS_SINT32 nValue);
    void PutConfigCache(IN Feature eFeature, IN IMS_BOOL bValue);
    void PutConfigCache(IN Feature eFeature, IN const AString& strValue);

    void OnRegistrationRefreshed();  // called by MtcService.

private:
    MtcConfigurationManager m_objManager;
    ConfigCache* m_pCache;
};

#endif
