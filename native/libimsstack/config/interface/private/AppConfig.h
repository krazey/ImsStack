/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

#include "ImsRegistry.h"
#include "IAppConfig.h"

class CoreServiceConfig;
class AppConfigPrivate;

class AppConfig : public IAppConfig
{
public:
    explicit AppConfig(IN const AString& strAppId_);
    AppConfig(IN const AppConfig& objRHS);
    virtual ~AppConfig();

public:
    AppConfig& operator=(IN const AppConfig& objRHS);

public:
    // IAppConfig interface
    virtual const AString& GetAppId() const;
    virtual const ICoreServiceConfig* GetCoreServiceConfig(IN const AString& strServiceId) const;
    virtual ImsRegistry* ToRegistry() const;

    IMS_BOOL Create(IN const ImsRegistry& objRegistry, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IMS_BOOL Equals(IN const AString& strAppId) const;
    IMS_BOOL IsStreamMediaSupported() const;
    IMS_BOOL IsStreamMediaAudioSupported() const;
    IMS_BOOL IsStreamMediaVideoSupported() const;
    IMS_BOOL IsFramedMediaSupported() const;
    const AStringArray& GetFramedMediaMimeTypes() const;
    IMS_BOOL IsFramedMediaMaxSizePresent() const;
    IMS_UINT32 GetFramedMediaMaxSize() const;
    IMS_BOOL IsBasicMediaSupported() const;
    const AStringArray& GetBasicMediaMimeTypes() const;
    IMS_BOOL IsEventPackageSupported(IN const AString& strEvent) const;
    const AStringArray& GetSupportedEventPackages() const;
    IMS_BOOL IsHeaderReadable(IN const AString& strHeader) const;
    IMS_BOOL IsHeaderWritable(IN const AString& strHeader) const;
    AStringArray GetCapabilitySDPs(IN IMS_SINT32 nSector, IN IMS_SINT32 nMessageType) const;
    const CoreServiceConfig* GetCoreServiceConfigEx(IN const AString& strServiceId) const;
    const IMSList<CoreServiceConfig*>& GetCoreServiceConfigs() const;

private:
    AppConfigPrivate* pConfigP;
};

#endif  // _APP_CONFIG_H_
