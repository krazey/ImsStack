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
#ifndef CORE_SERVICE_CONFIG_H_
#define CORE_SERVICE_CONFIG_H_

#include "AStringArray.h"

#include "ICoreServiceConfig.h"

class CoreServiceConfigPrivate;
class FeatureSet;
class ImsRegistry;
class QosProperty;

class CoreServiceConfig : public ICoreServiceConfig
{
public:
    explicit CoreServiceConfig(IN const AString& strServiceId);
    virtual ~CoreServiceConfig();

    CoreServiceConfig(IN const CoreServiceConfig&) = delete;
    CoreServiceConfig& operator=(IN const CoreServiceConfig&) = delete;

public:
    // ICoreServiceConfig interface
    const AString& GetServiceId() const override;
    IMS_BOOL IsIariSupported() const override;
    const ServiceIdentifier& GetIari() const override;
    const IMSList<ServiceIdentifier>& GetIcsis() const override;
    const IMSList<ServiceIdentifier>& GetFeatureTags() const override;
    const AString& GetMediaProfile() const override;

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
    CoreServiceConfigPrivate* m_pConfigPrivate;
};

#endif
