/**
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

#ifndef _MEDIA_SESSION_CONFIG_H_
#define _MEDIA_SESSION_CONFIG_H_

#include "MediaDef.h"
#include "ImsList.h"
#include "AString.h"
#include "ICarrierConfigListener.h"

class ICarrierConfig;
class AudioConfiguration;
class VideoConfiguration;
class TextConfiguration;

class MediaSessionConfig : public ICarrierConfigListener
{
public:
    MediaSessionConfig(
            IN IMS_SINT32 nSlotId = 0, IN MEDIA_SERVICE_TYPE eServiceType = MEDIA_SERVICE_DEFAULT);
    virtual ~MediaSessionConfig();

    IMS_BOOL Create(IN IMS_SINT32 nSlotId);
    void SetServiceType(IN MEDIA_SERVICE_TYPE eServiceType);
    void ToDebugString() const;

    AudioConfiguration* GetAudioConfiguration() const;
    VideoConfiguration* GetVideoConfiguration() const;
    TextConfiguration* GetTextConfiguration() const;

    MEDIA_SERVICE_TYPE GetServiceType() const;
    IMS_BOOL IsSessionLevelBandwidth() const;
    IMS_BOOL IsAnbrSupported() const;
    IMS_BOOL IsSupportMultiConfigInEarlySession() const;
    IMS_BOOL IsSdpReofferFullCapa() const;

    static const IMS_BOOL DEFAULT_SESSION_LEVEL_BW = IMS_FALSE;
    static const IMS_BOOL DEFAULT_ANBR_CAPABILITY = IMS_FALSE;
    static const IMS_BOOL DEFAULT_SUPPORT_MULTICONFIG = IMS_TRUE;

private:
    void Clear();
    IMS_BOOL Update(IN ICarrierConfig* piCc);
    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

    IMS_BOOL CreateAudioConfiguration(IN ICarrierConfig* piCc);
    IMS_BOOL CreateVideoConfiguration(IN ICarrierConfig* piCc);
    IMS_BOOL CreateTextConfiguration(IN ICarrierConfig* piCc);

    IMS_BOOL UpdateAudioConfiguration(IN ICarrierConfig* piCc);
    IMS_BOOL UpdateVideoConfiguration(IN ICarrierConfig* piCc);
    IMS_BOOL UpdateTextConfiguration(IN ICarrierConfig* piCc);

    void MakeAnalyzerList(IN CONST AString& strList);
    void MakeAnalyzerOptionList(IN CONST AString& strList);

private:
    AudioConfiguration* m_pAudioConfig;
    VideoConfiguration* m_pVideoConfig;
    TextConfiguration* m_pTextConfig;

    MEDIA_SERVICE_TYPE m_nServiceType;
    IMS_BOOL m_bIsSessLevelBW;
    IMS_BOOL m_bAnbrSupported;
    IMS_BOOL m_bSupportMultiConfigInEarlySession;
    IMS_BOOL m_bSdpReofferFullCapa;
};
#endif  // _MEDIA_SESSION_CONFIG_H_
