#include "ConfigMedia.h"
#include "ConfigMtc.h"
#include "ConfigMts.h"
#include "ConfigSipDelegate.h"
#include "ConfigUce.h"
#include "StaticConfig.h"

struct ConfigMap
{
    const IMS_CHAR* pszKey;
    const IMS_CHAR* pszConfig;
};

LOCAL const ConfigMap s_objConfigMap[] = {
        // Media
        {ConfigMedia::MEDIA_NAME,              ConfigMedia::MEDIA_CONFIG             },
        {ConfigMedia::MEDIA_CAPABILITIES_NAME, ConfigMedia::MEDIA_CAPABILITIES_CONFIG},

        /// IMS services
        // Mtc
        {ConfigMtc::APP_NAME,                  ConfigMtc::APP_CONFIG                 },
        // Mts
        {ConfigMts::APP_NAME,                  ConfigMts::APP_CONFIG                 },
        // Uce
        {ConfigUce::APP_NAME,                  ConfigUce::APP_CONFIG                 },
        // SipDelegate
        {ConfigSipDelegate::APP_NAME,          ConfigSipDelegate::APP_CONFIG         }
};

PUBLIC GLOBAL const IMS_CHAR* StaticConfig::GetConfig(IN const AString& strName)
{
    IMS_UINT32 nCount = sizeof(s_objConfigMap) / sizeof(s_objConfigMap[0]);

    for (IMS_UINT32 i = 0; i < nCount; i++)
    {
        const ConfigMap* pConfigMap = &(s_objConfigMap[i]);

        if (strName.Equals(pConfigMap->pszKey))
        {
            return pConfigMap->pszConfig;
        }
    }

    return IMS_NULL;
}

PUBLIC GLOBAL const IMS_CHAR* StaticConfig::GetMediaConfig()
{
    const AString strName(ConfigMedia::MEDIA_NAME);
    return GetConfig(strName);
}
