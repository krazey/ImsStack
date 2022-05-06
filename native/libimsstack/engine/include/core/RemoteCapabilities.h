/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091201  toastops@                 Created
    </table>

    Description

*/

#ifndef _REMOTE_CAPABILITIES_
#define _REMOTE_CAPABILITIES_

#include "AString.h"

class AppConfig;
class CoreServiceConfig;
class FeatureSet;

class RemoteCapabilities
{
public:
    RemoteCapabilities();
    virtual ~RemoteCapabilities();

public:
    IMS_BOOL Create(IN CONST IMSList<AString>& objCapabilities);
    IMS_BOOL IsCompatible(IN CONST AppConfig* pAppConfig, IN CONST AString& strServiceId) const;

private:
    IMS_BOOL IsAudioSupported() const;
    IMS_BOOL IsVideoSupported() const;
    IMS_BOOL IsFramedMediaSupported() const;
    IMS_BOOL IsAppSubTypeSupported(IN CONST AString& strAppSubType) const;
    IMS_BOOL IsEventSupported(IN CONST AString& strEvent) const;
    IMS_BOOL IsIARISupported(IN CONST AString& strIARI) const;
    IMS_BOOL IsICSISupported(IN CONST AString& strICSI) const;
    IMS_BOOL IsFeatureTagSupported(IN CONST AString& strFeatureTag) const;

    IMS_BOOL IsBasicMediaCompatible(IN CONST AppConfig* pAppConfig) const;
    IMS_BOOL IsEventCompatible(IN CONST AppConfig* pAppConfig) const;
    IMS_BOOL IsCoreServiceCompatible(
            IN CONST AppConfig* pAppConfig, IN CONST AString& strServiceId) const;
    IMS_BOOL IsCoreServiceCompatible(IN CONST CoreServiceConfig* pServiceConfig) const;

    static void RemoveAllFeatureSets(IN_OUT IMSList<FeatureSet*>& objFeatureSets);

private:
    IMS_BOOL bAudioSupported;
    IMS_BOOL bVideoSupported;
    IMS_BOOL bFramedMediaSupported;
    IMS_BOOL bApplicationSupported;

    IMSList<FeatureSet*> objAppSubTypes;
    IMSList<FeatureSet*> objEvents;
    IMSList<FeatureSet*> objICSIs;
    IMSList<FeatureSet*> objIARIs;
    IMSList<FeatureSet*> objFeatureTags;
};

#endif  // _REMOTE_CAPABILITIES_
