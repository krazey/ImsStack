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
#ifndef REMOTE_CAPABILITIES_
#define REMOTE_CAPABILITIES_

#include "AString.h"

class AppConfig;
class CoreServiceConfig;
class FeatureSet;

class RemoteCapabilities
{
public:
    RemoteCapabilities();
    virtual ~RemoteCapabilities();

    RemoteCapabilities(IN const RemoteCapabilities&) = delete;
    RemoteCapabilities& operator=(IN const RemoteCapabilities&) = delete;

public:
    IMS_BOOL Create(IN const IMSList<AString>& objCapabilities);
    IMS_BOOL IsCompatible(IN const AppConfig* pAppConfig, IN const AString& strServiceId) const;

private:
    /**
     * @brief Checks if the remote device supports streaming audio or audio content-type.
     */
    inline IMS_BOOL IsAudioSupported() const { return m_bAudioSupported; }
    /**
     * @brief Checks if the remote device supports streaming video or video content-type.
     */
    inline IMS_BOOL IsVideoSupported() const { return m_bVideoSupported; }
    /**
     * @brief Checks if the remote device supports Framed Media, MSRP.
     */
    inline IMS_BOOL IsFramedMediaSupported() const { return m_bFramedMediaSupported; }
    IMS_BOOL IsAppSubTypeSupported(IN const AString& strAppSubType) const;
    IMS_BOOL IsEventSupported(IN const AString& strEvent) const;
    IMS_BOOL IsIariSupported(IN const AString& strIari) const;
    IMS_BOOL IsIcsiSupported(IN const AString& strIcsi) const;
    IMS_BOOL IsFeatureTagSupported(IN const AString& strFeatureTag) const;

    IMS_BOOL IsBasicMediaCompatible(IN const AppConfig* pAppConfig) const;
    IMS_BOOL IsEventCompatible(IN const AppConfig* pAppConfig) const;
    IMS_BOOL IsCoreServiceCompatible(
            IN const AppConfig* pAppConfig, IN const AString& strServiceId) const;
    IMS_BOOL IsCoreServiceCompatible(IN const CoreServiceConfig* pServiceConfig) const;

    static void RemoveAllFeatureSets(IN_OUT IMSList<FeatureSet*>& objFeatureSets);

private:
    IMS_BOOL m_bAudioSupported;
    IMS_BOOL m_bVideoSupported;
    IMS_BOOL m_bFramedMediaSupported;
    IMS_BOOL m_bApplicationSupported;

    IMSList<FeatureSet*> m_objAppSubTypes;
    IMSList<FeatureSet*> m_objEvents;
    IMSList<FeatureSet*> m_objIcsis;
    IMSList<FeatureSet*> m_objIaris;
    IMSList<FeatureSet*> m_objFeatureTags;
};

#endif
