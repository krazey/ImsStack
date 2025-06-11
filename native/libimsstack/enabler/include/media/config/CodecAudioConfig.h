/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef CODEC_AUDIO_CONFIG_H_
#define CODEC_AUDIO_CONFIG_H_

#include "config/CodecConfig.h"

class CodecAudioConfig : public CodecConfig
{
public:
    /**
     * @brief Construct a new codec audio(amr/evs) config
     *
     * @param nType audio codec type (AUDIO_AMR, AUDIO_AMR_WB, AUDIO_EVS)
     * @param nPayloadTypeNum payload type number
     */
    CodecAudioConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum);
    /**
     * @brief Destroy the codec audio config object
     */
    virtual ~CodecAudioConfig();
    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc) override;
    /**
     * @brief Print debug string
     *
     */
    virtual void ToDebugString() const override;
    /**
     * @brief Get the channel
     *
     * @return IMS_SINT32 Return the channel id - default : 1
     */
    IMS_SINT32 GetChannel() const;
    /**
     * @brief Get the mode-set
     *
     * @return IMS_SINT32 Return the audio codec mode-set
     */
    IMS_SINT32 GetAmrModeSet() const;
    /**
     * @brief Get the mode-set list
     *
     * @return IMS_UINT32 Return the audio codec mode-set list
     */
    IMS_UINT32 GetAmrModeSetList() const;
    /**
     * @brief Get the default mode-set list
     *
     * @return IMS_UINT32 Return the audio codec default mode-set list
     */
    IMS_UINT32 GetDefaultAmrModeSetList() const;
    /**
     * @brief Get the information whether to include AMR codec modeset in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include modeset attribute to sdp
     * IMS_FALSE Not include modeset attribute to sdp
     */
    IMS_BOOL GetShowAmrModeSet() const;

    /**
     * @brief Get mode-change-capability
     *
     * @return IMS_SINT32 Return the mode-change-capability
     */
    IMS_SINT32 GetModeChangeCapability() const;
    /**
     * @brief Get mode-change-period
     *
     * @return IMS_SINT32 Return the mode-change-period
     */
    IMS_SINT32 GetModeChangePeriod() const;
    /**
     * @brief Get the mode-change-neighbor
     *
     * @return IMS_SINT32 Return the mode-change-neighbor
     */
    IMS_SINT32 GetModeChangeNeighbor() const;
    /**
     * @brief Get the dtx (scr) enabled
     *
     * @return IMS_SINT32 Return the Audio codec (AMR/EVS) dtx enabled
     */
    IMS_BOOL GetDtx() const;

public:
    static const IMS_SINT32 DEFAULT_CHANNEL;
    static const IMS_SINT32 DEFAULT_MODESET_AMR;
    static const IMS_SINT32 DEFAULT_MODESET_AMR_WB;
    static const IMS_SINT32 DEFAULT_MODECHANGE_CAPABILITY;
    static const IMS_SINT32 DEFAULT_MODECHANGE_PERIOD;
    static const IMS_SINT32 DEFAULT_MODECHANGE_NEIGHBOR;
    static const IMS_SINT32 DEFAULT_MAXRED;
    static const IMS_BOOL DEFAULT_DTX;

protected:
    IMS_BOOL m_bDtx;
    IMS_BOOL m_bShowAmrModeSet;
    IMS_SINT32 m_nChannel;
    IMS_UINT32 m_nAmrModeSetList;
    IMS_UINT32 m_nDefaultAmrModeSetList;
    IMS_SINT32 m_nModeChangeCapability;
    IMS_SINT32 m_nModeChangePeriod;
    IMS_SINT32 m_nModeChangeNeighbor;
};

#endif
