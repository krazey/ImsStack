/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090608  toastops@                 Created
    </table>

    Description

*/

#ifndef _CONFIGURATION_MANAGER_H_
#define _CONFIGURATION_MANAGER_H_

#include "AStringArray.h"

class AppConfig;
class ConfigurationManagerPrivate;

class SubscriberConfig;
class EngineConfig;
class SipConfig;
class MediaConfig;

class ConfigurationManager
{
private:
    ConfigurationManager();
    ConfigurationManager(IN const ConfigurationManager& objRHS);
    ConfigurationManager& operator=(IN const ConfigurationManager& objRHS);

public:
    ~ConfigurationManager();

public:
    // IMS registry for J281 requirements - application configuration
    const AppConfig* GetAppConfig(
            IN const AString& strAppId, IN IMS_SINT32 nSlotId = IMS_SLOT_0) const;
    AStringArray GetAppIds(IN IMS_SINT32 nSlotId = IMS_SLOT_0) const;
    IMS_BOOL IsAppConfigured(IN const AString& strAppId, IN IMS_SINT32 nSlotId = IMS_SLOT_0) const;
    void RemoveAppConfig(IN const AString& strAppId, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IMS_RESULT StoreAppConfig(IN AppConfig* pAppConfig, IN const AString& strAppId,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    // Returns the configuration mode (file / xml / db)
    IMS_SINT32 GetConfigMode() const;
    // Returns the locator of the configuration
    const AString& GetConfigLocator() const;

    // Subscriber configuration - impl. defined
    //    This config. includes IMS-related information in the ISIM.
    const SubscriberConfig* GetSubscriberConfig(
            IN const AString& strId, IN IMS_SINT32 nSlotId = IMS_SLOT_0) const;
    const IMSList<SubscriberConfig*>& GetSubscriberConfigs(
            IN IMS_SINT32 nSlotId = IMS_SLOT_0) const;
    // Engine configuration - impl. defined
    //    This config. includes the information for an optional/additional operation
    //    in J281 engine implementation.
    const EngineConfig* GetEngineConfig(IN IMS_SINT32 nSlotId = IMS_SLOT_0) const;
    // SIP configuration - impl. defined
    //    This config. includes the SIP-specific information for a default UA behavior.
    const SipConfig* GetSipConfig(IN IMS_SINT32 nSlotId = IMS_SLOT_0) const;
    // Media configuration - impl. defined
    //    This config. includes the media-specific information (SDP for session & capabilities).
    const MediaConfig* GetMediaConfig(IN IMS_SINT32 nSlotId = IMS_SLOT_0) const;

    static ConfigurationManager* GetInstance();

    //// Load a default basic configuration for IMS client platform
    IMS_BOOL Initialize(IN const AString& strLocator, IN IMS_SINT32 nMode);

    // Invoked by enabler threads
    void InitConfigs(IN IMS_SINT32 nSlotId);
    void RefreshConfigs(IN IMS_SINT32 nSlotId);

public:
    enum
    {
        // .conf file
        MODE_FILE = 0,
        // .xml file
        MODE_XML,
        // database
        MODE_DB,
        // hsyun - 110701 Add Hard coding mode
        MODE_CODE
        // hsyun - end
    };

private:
    ConfigurationManagerPrivate* pConfigMngrP;
};

#endif  // _CONFIGURATION_MANAGER_H_
