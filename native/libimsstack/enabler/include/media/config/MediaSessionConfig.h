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

#ifndef MEDIA_SESSION_CONFIG_H_
#define MEDIA_SESSION_CONFIG_H_

#include "MediaDef.h"
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
    explicit MediaSessionConfig(
            IN IMS_SINT32 nSlotId = 0, IN MEDIA_SERVICE_TYPE eServiceType = MEDIA_SERVICE_DEFAULT);
    virtual ~MediaSessionConfig() override;

    /**
     * @brief Create codec using the configuration.
     *
     * @param nSlotId SIM slot id
     * @return IMS_BOOL Return true if the create function is executed without error, return false
     * if the create function is failed.
     */
    virtual IMS_BOOL Create(IN IMS_SINT32 nSlotId);

    /**
     * @brief Set the service type.
     *
     * @param eServiceType service type (ex: default, emergency).
     */
    virtual void SetServiceType(IN MEDIA_SERVICE_TYPE eServiceType);

    /**
     * @brief Print debug strings.
     */
    virtual void ToDebugString() const;

    /**
     * @brief Get the audio configuration.
     *
     * @return AudioConfiguration* Return audio configuration.
     */
    virtual AudioConfiguration* GetAudioConfiguration() const;

    /**
     * @brief Get the video configuration.
     *
     * @return VideoConfiguration* Return video configuration.
     */
    virtual VideoConfiguration* GetVideoConfiguration() const;

    /**
     * @brief Get the text configuration.
     *
     * @return TextConfiguration* Return text configuration.
     */
    virtual TextConfiguration* GetTextConfiguration() const;

    /**
     * @brief Get the service type.
     *
     * @return MEDIA_SERVICE_TYPE Return the service type value.
     */
    virtual MEDIA_SERVICE_TYPE GetServiceType() const;

    /**
     * @brief Get whether bandwidth is added to sessionlevel.
     *
     * @return IMS_BOOL Return true if the session level bandwidth is enabled, return false if the
     * session level bandwidth is disabled.
     */
    virtual IMS_BOOL IsSessionLevelBandwidth() const;

    /**
     * @brief Get whether ANBR is supported in modem side.
     *
     * @return IMS_BOOL Return true if ANBR is supported, return false if ANBR is not supported.
     */
    virtual IMS_BOOL IsAnbrSupported() const;

    /**
     * @brief Get whether multiple configurations are supported in an early session.
     *
     * @return IMS_BOOL Return true if multiple configurations are supported in an early session,
     * otherwise false.
     */
    virtual IMS_BOOL IsSupportMultiConfigInEarlySession() const;

    /**
     * @brief Get whether to use full codec capability when sending a SDP re-offer.
     *
     * @return IMS_BOOL Return true if full codec capability is used for SDP re-offers,
     * otherwise false.
     */
    virtual IMS_BOOL IsSdpReofferFullCapability() const;

    static const IMS_BOOL DEFAULT_SESSION_LEVEL_BW = IMS_FALSE;
    static const IMS_BOOL DEFAULT_ANBR_CAPABILITY = IMS_FALSE;
    static const IMS_BOOL DEFAULT_SUPPORT_MULTICONFIG = IMS_TRUE;

protected:
    IMS_BOOL CreateAudioConfiguration(IN ICarrierConfig* piCc);
    IMS_BOOL CreateVideoConfiguration(IN ICarrierConfig* piCc);
    IMS_BOOL CreateTextConfiguration(IN ICarrierConfig* piCc);

private:
    void Clear();
    void ResetMediaConfigurations(IN IMS_SINT32 nSlotId);
    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

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

#endif
