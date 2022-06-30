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
    /**
     * @brief Construct a new media session config
     *
     * @param nSlotId SIM slot id
     * @param eServiceType service type (ex: default, emergency)
     */
    MediaSessionConfig(
            IN IMS_SINT32 nSlotId = 0, IN MEDIA_SERVICE_TYPE eServiceType = MEDIA_SERVICE_DEFAULT);
    /**
     * @brief Destroy the media session config
     *
     */
    virtual ~MediaSessionConfig();
    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    IMS_BOOL Create(IN IMS_SINT32 nSlotId);
    /**
     * @brief Set the service type
     *
     * @param eServiceType service type (ex: default, emergency)
     */
    void SetServiceType(IN MEDIA_SERVICE_TYPE eServiceType);
    /**
     * @brief Print debug strings
     *
     */
    void ToDebugString() const;
    /**
     * @brief Get the audio configuration
     *
     * @return AudioConfiguration* Return audio configuration
     */
    AudioConfiguration* GetAudioConfiguration() const;
    /**
     * @brief Get the video configuration
     *
     * @return VideoConfiguration* Return video configuration
     */
    VideoConfiguration* GetVideoConfiguration() const;
    /**
     * @brief Get the text configuration
     *
     * @return TextConfiguration* Return text configuration
     */
    TextConfiguration* GetTextConfiguration() const;
    /**
     * @brief Get the service type
     *
     * @return MEDIA_SERVICE_TYPE Return the service type value
     */
    MEDIA_SERVICE_TYPE GetServiceType() const;
    /**
     * @brief Get whether bandwidth is added to sessionlevel
     *
     * @return IMS_BOOL Return true if the session level bandwidth is enabled
     * Return false if the session level bandwidth is disabled
     */
    IMS_BOOL IsSessionLevelBandwidth() const;
    /**
     * @brief Get whether anbr is supported in modem side
     *
     * @return IMS_BOOL Return true if anbr is supported
     * Return false if anbr is not supported
     */
    IMS_BOOL IsAnbrSupported() const;
    /**
     * @brief Get whether multiconfiginearlysession is supported
     *
     * @return IMS_BOOL Return true if multiconfiginearlysession is supported
     * Return false if multiconfiginearlysession is not supported
     */
    IMS_BOOL IsSupportMultiConfigInEarlySession() const;
    /**
     * @brief Get whether to use full capability when sdp reoffer
     *
     * @return IMS_BOOL Return true if SdpReofferFullCapability is enabled
     * Return false if SdpReofferFullCapability is disabled
     */
    IMS_BOOL IsSdpReofferFullCapability() const;

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
    IMS_BOOL m_bSdpReofferFullCapability;
};
#endif  // _MEDIA_SESSION_CONFIG_H_
