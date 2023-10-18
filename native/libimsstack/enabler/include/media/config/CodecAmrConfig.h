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

#ifndef CODEC_AMR_CONFIG_H_
#define CODEC_AMR_CONFIG_H_

#include "config/CodecConfig.h"

class CodecAmrConfig : public CodecConfig
{
public:
    /**
     * @brief Construct a new codec amr config
     *
     * @param nType audio codec type (AUDIO_AMR, AUDIO_AMR_WB)
     * @param nPayloadTypeNum payload type number
     */
    CodecAmrConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum);
    /**
     * @brief Destroy the codec amr config object
     *
     */
    virtual ~CodecAmrConfig();
    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration
     * @param nCodecIdx codec index within each codec type
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc, IN IMS_SINT32 nCodecIdx) override;
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
    IMS_SINT32 GetModeSet() const;
    /**
     * @brief Get the mode-set list
     *
     * @return IMS_UINT32 Return the audio codec mode-set list
     */
    IMS_UINT32 GetModeSetList() const;
    /**
     * @brief Get the default mode-set list
     *
     * @return IMS_UINT32 Return the audio codec default mode-set list
     */
    IMS_UINT32 GetDefaultModeSetList() const;
    /**
     * @brief Get the information whether to include AMR codec modeset in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include modeset attribute to sdp
     * IMS_FALSE Not include modeset attribute to sdp
     */
    IMS_BOOL GetShowModeSet() const;
    /**
     * @brief Get the payload format
     *
     * @return IMS_SINT32 Return bandwidth-efficient or octet-align
     */
    IMS_SINT32 GetOctetAlign() const;
    /**
     * @brief Get the sampling rate
     *
     * @return IMS_SINT32 Return the audio codec sampling rate
     */
    IMS_SINT32 GetSamplingRate() const;
    /**
     * @brief Get the dtx
     *
     * @return IMS_BOOL Return true if dtx is supported
     * Return false if drx is not supported
     */
    IMS_BOOL GetDtx() const;
    /**
     * @brief Get mode-change-capability
     *
     * @return IMS_SINT32 Return mode-change-capability
     */
    IMS_SINT32 GetModeChangeCapability() const;
    /**
     * @brief Get mode-change-period
     *
     * @return IMS_SINT32 Return mode-change-period
     */
    IMS_SINT32 GetModeChangePeriod() const;
    /**
     * @brief Get the mode-change-neighbor
     *
     * @return IMS_SINT32 Return mode-change-neighbor
     */
    IMS_SINT32 GetModeChangeNeighbor() const;

public:
    enum
    {
        /** Full payload is octet aligned */
        BANDWIDTH_EFFICIENT = 0,
        /** All the fields are individually aligned to octet boundaries */
        OCTET_ALIGN = 1
    };

    static const IMS_SINT32 DEFAULT_CHANNEL = 1;
    static const IMS_SINT32 DEFAULT_OCTET_ALIGN = BANDWIDTH_EFFICIENT;
    static const IMS_SINT32 DEFAULT_MODESET_AMR = 7;
    static const IMS_SINT32 DEFAULT_MODESET_AMR_WB = 8;
    static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMR = 8000;
    static const IMS_SINT32 DEFAULT_SAMPLING_RATE_AMRWB = 16000;
    static const IMS_BOOL DEFAULT_AMR_DTX = IMS_TRUE;
    static const IMS_SINT32 DEFAULT_MODECHANGE_CAPABILITY = 1;
    static const IMS_SINT32 DEFAULT_MODECHANGE_PERIOD = 1;
    static const IMS_SINT32 DEFAULT_MODECHANGE_NEIGHBOR = 0;

private:
    /**
     *   Payload Num
     *      RTP Payload Number
     *      -1 means default value should be used.
     *      otherwise the value will be used for the rtp payload number
     *
     *   Mode Set
     *      If there are no value in objModeSets, it means 'mode-set' should be hided.
     *      Otherwise 'mode-set' will be presented in SDP
     *      and the values in objModeSets will be listed as 'mode-set' values.
     *
     *       AMR
     *          4.75 (0)
     *          5.15 (1)
     *          5.9 (2)
     *          6.7 (3)
     *          7.4 (4)
     *          7.95 (5)
     *          10.2 (6)
     *          12.2 (7)
     *       AMR-WB
     *          6.6 (0)
     *          8.85 (1)
     *          12.65 (2)
     *          14.25 (3)
     *          15.85 (4)
     *          18.25 (5)
     *          19.85 (6)
     *          23.05 (7)
     *          23.85 (8)
     *
     *   Other items
     *      -1 means that this item shold NOT be presented in SDP.
     *      If each item has other value, the item will be presented in SDP with the value.
     */

    IMS_SINT32 m_nChannel;
    IMS_BOOL m_bShowModeSet;
    IMS_UINT32 m_nModeSetList;  // 0 means support all mode set
    IMS_UINT32 m_nDefaultModeSetList;  // 0 means support all mode set
    IMS_SINT32 m_nOctetAlign;
    IMS_SINT32 m_nSamplingRate;
    IMS_BOOL m_bDtx;
    IMS_SINT32 m_nModeChangeCapability;
    IMS_SINT32 m_nModeChangePeriod;
    IMS_SINT32 m_nModeChangeNeighbor;
};

#endif
