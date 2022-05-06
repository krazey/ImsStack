/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#ifndef _CORE_SERVICE_CONFIG_H_
#define _CORE_SERVICE_CONFIG_H_

#include "AStringArray.h"
#include "ICoreServiceConfig.h"

class FeatureSet;
class ImsRegistry;
class QosProperty;
class CoreServiceConfigPrivate;

class CoreServiceConfig : public ICoreServiceConfig
{
public:
    explicit CoreServiceConfig(IN const AString& strServiceId_);
    virtual ~CoreServiceConfig();

private:
    CoreServiceConfig(IN const CoreServiceConfig& objRHS);
    CoreServiceConfig& operator=(IN const CoreServiceConfig& objRHS);

public:
    // ICoreServiceConfig interface
    virtual const AString& GetServiceId() const;
    virtual IMS_BOOL IsIARISupported() const;
    virtual const ServiceIdentifier& GetIARI() const;
    virtual const IMSList<ServiceIdentifier>& GetICSIs() const;
    virtual const IMSList<ServiceIdentifier>& GetFeatureTags() const;
    virtual const AString& GetMediaProfile() const;

    IMS_BOOL Create(IN const AStringArray& objCoreServiceProperty);
    IMS_BOOL AddProperty(IN const AStringArray& objProperty);
    const IMSList<FeatureSet*>& GetFeatureSets() const;
    AStringArray GetQosContentTypes() const;
    const QosProperty* GetFlowSpecSend(IN const AString& strContentType) const;
    const QosProperty* GetFlowSpecReceive(IN const AString& strContentType) const;
    const AStringArray& GetRegistrationHeaders() const;
    IMS_BOOL IsConnectionModelSupported() const;

    void ToRegistry(IN_OUT ImsRegistry*& pRegistry) const;

private:
    CoreServiceConfigPrivate* pConfigP;
};

#endif  // _CORE_SERVICE_CONFIG_H_
