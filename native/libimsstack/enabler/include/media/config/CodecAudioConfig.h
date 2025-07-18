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
    virtual ~CodecAudioConfig() override;
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
    IMS_SINT32 GetChannel() const { return m_nChannel; }
    /**
     * @brief Get the mode-set
     *
     * @return IMS_SINT32 Return the audio codec mode-set
     */
    IMS_SINT32 GetModeSet() const;
    /**
     * @brief Get the mode-set list
     *
     * @return IMS_UINT32 Return the audio codec mode-set list
     */
    IMS_UINT32 GetModeSetList() const { return m_nModeSetList; }
    /**
     * @brief Get the default mode-set list
     *
     * @return IMS_UINT32 Return the audio codec default mode-set list
     */
    IMS_UINT32 GetDefaultModeSetList() const { return m_nDefaultModeSetList; }
    /**
     * @brief Get the mode-change-capability
     *
     * @return IMS_SINT32 Return the mode-change-capability
     */
    IMS_SINT32 GetModeChangeCapability() const { return m_nModeChangeCapability; }
    /**
     * @brief Get the mode-change-period
     *
     * @return IMS_SINT32 Return the mode-change-period
     */
    IMS_SINT32 GetModeChangePeriod() const { return m_nModeChangePeriod; }
    /**
     * @brief Get the mode-change-neighbor
     *
     * @return IMS_SINT32 Return the mode-change-neighbor
     */
    IMS_SINT32 GetModeChangeNeighbor() const { return m_nModeChangeNeighbor; }
    /**
     * @brief Get the information whether to include AMR codec modeset in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include modeset attribute to sdp
     * IMS_FALSE Not include modeset attribute to sdp
     */
    IMS_BOOL GetVisibleModeSet() const { return m_bVisibleModeSet; }
    /**
     * @brief Get the information whether to include the mode-change-capability in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include mode-change-capability attribute to sdp
     * IMS_FALSE Not include mode-change-capability attribute to sdp
     */
    IMS_BOOL GetVisibleModeChangeCapability() const { return m_bVisibleModeChangeCapability; }
    /**
     * @brief Get the information whether to include the mode-change-period in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include mode-change-period attribute to sdp
     * IMS_FALSE Not include mode-change-period attribute to sdp
     */
    IMS_BOOL GetVisibleModeChangePeriod() const { return m_bVisibleModeChangePeriod; }
    /**
     * @brief Get the information whether to include the mode-change-neighbor in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include mode-change-neighbor attribute to sdp
     * IMS_FALSE Not include mode-change-neighbor attribute to sdp
     */
    IMS_BOOL GetVisibleModeChangeNeighbor() const { return m_bVisibleModeChangeNeighbor; }
    /**
     * @brief Get the dtx (scr) enabled
     *
     * @return IMS_SINT32 Return the Audio codec (AMR/EVS) dtx enabled
     */
    IMS_BOOL GetDtx() const { return m_bDtx; }
    /**
     * @brief Convert the mode-set array to mode-set list value
     *
     * @param objCodecModeset The mode-set value array
     */
    IMS_SINT32 ConvertModeSetList(ImsVector<IMS_SINT32> objCodecModeset);
    /**
     * @brief Set whether to include the mode-change-capability in sdp
     *
     * @param bVisibleModeChangeCapability Flag for enable/disable this rule
     */
    void SetVisibleModeChangeCapability(IMS_BOOL bVisibleModeChangeCapability);

    /**
     * @brief Set whether to include the mode-change-period in sdp
     *
     * @param bVisibleModeChangePeriod Flag for enable/disable this rule
     */
    void SetVisibleModeChangePeriod(IMS_BOOL bVisibleModeChangePeriod);

    /**
     * @brief Set whether to include the mode-change-neighbor in sdp
     *
     * @param bVisibleModeChangeNeighbor Flag for enable/disable this rule
     */
    void SetVisibleModeChangeNeighbor(IMS_BOOL bVisibleModeChangeNeighbor);

    /**
     * @brief Set whether to include the amr mode-set in sdp
     *
     * @param bVisibleModeSet Flag for enable/disable this rule
     */
    void SetVisibleModeSet(IMS_BOOL bVisibleModeSet);

    /**
     * @brief Set the amr mode-set list
     *
     * @param nModeSetList The amr mode-set List
     */
    void SetModeSetList(IMS_UINT32 nModeSetList);

    /**
     * @brief Set the mode-change-capability value
     *
     * @param nModeChangeCapability The mode-change-capability value
     */
    void SetModeChangeCapability(IMS_SINT32 nModeChangeCapability);

    /**
     * @brief Set the mode-change-period value
     *
     * @param nModeChangeCapability The mode-change-period value
     */
    void SetModeChangePeriod(IMS_SINT32 nModeChangePeriod);

    /**
     * @brief Set the mode-change-neighbor value
     *
     * @param nModeChangeCapability The mode-change-neighbor value
     */
    void SetModeChangeNeighbor(IMS_SINT32 nModeChangeNeighbor);

    /**
     * @brief Set the default amr mode-set list
     *
     * @param nModeChangeCapability The default amr mode-set list
     */
    void SetDefaultModeSetList(IMS_UINT32 nDefaultModeSetList);

    /**
     * @brief Set the channel value
     *
     * @param nChannel The channel value
     */
    void SetChannel(IMS_SINT32 nChannel);

    /**
     * @brief Set the dtx (scr) enabled
     *
     * @param bDtx The Dtx on/off value
     */
    void SetDtx(IMS_BOOL bDtx);

public:
    static const IMS_SINT32 DEFAULT_CHANNEL;
    static const IMS_SINT32 DEFAULT_MODESET_AMRNB;
    static const IMS_SINT32 DEFAULT_MODESET_AMRWB;
    static const IMS_SINT32 FULL_MODESET_AMRNB;
    static const IMS_SINT32 FULL_MODESET_AMRWB;
    static const IMS_SINT32 DEFAULT_MODECHANGE_CAPABILITY;
    static const IMS_SINT32 DEFAULT_MODECHANGE_PERIOD;
    static const IMS_SINT32 DEFAULT_MODECHANGE_NEIGHBOR;
    static const IMS_SINT32 DEFAULT_MAXRED;
    static const IMS_BOOL DEFAULT_DTX;

private:
    IMS_BOOL m_bDtx;
    IMS_BOOL m_bVisibleModeSet;
    IMS_BOOL m_bVisibleModeChangeCapability;
    IMS_BOOL m_bVisibleModeChangePeriod;
    IMS_BOOL m_bVisibleModeChangeNeighbor;
    IMS_SINT32 m_nChannel;
    IMS_UINT32 m_nModeSetList;
    IMS_UINT32 m_nDefaultModeSetList;
    IMS_SINT32 m_nModeChangeCapability;
    IMS_SINT32 m_nModeChangePeriod;
    IMS_SINT32 m_nModeChangeNeighbor;
};

#endif
