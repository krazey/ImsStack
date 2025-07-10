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

#ifndef CODEC_EVS_CONFIG_H_
#define CODEC_EVS_CONFIG_H_

#include "AString.h"
#include "config/CodecAudioConfig.h"

class CodecEvsConfig : public CodecAudioConfig
{
public:
    /** Specifies the range of source codec bit-rate for EVS Primary mode in the session. */
    enum
    {
        /** EVS bitrate 5.9 kbps */
        EVS_PRIMARY_MODE_BITRATE_5_9_KBPS = 0,
        /** EVS bitrate 7.2 kbps */
        EVS_PRIMARY_MODE_BITRATE_7_2_KBPS = 1,
        /** EVS bitrate 8.0 kbps */
        EVS_PRIMARY_MODE_BITRATE_8_0_KBPS = 2,
        /** EVS bitrate 9.6 kbps */
        EVS_PRIMARY_MODE_BITRATE_9_6_KBPS = 3,
        /** EVS bitrate 13.2 kbps */
        EVS_PRIMARY_MODE_BITRATE_13_2_KBPS = 4,
        /** EVS bitrate 16.4 kbps */
        EVS_PRIMARY_MODE_BITRATE_16_4_KBPS = 5,
        /** EVS bitrate 24.4 kbps */
        EVS_PRIMARY_MODE_BITRATE_24_4_KBPS = 6,
        /** EVS bitrate 32 kbps */
        EVS_PRIMARY_MODE_BITRATE_32_0_KBPS = 7,
        /** EVS bitrate 48 kbps */
        EVS_PRIMARY_MODE_BITRATE_48_0_KBPS = 8,
        /** EVS bitrate 64 kbps */
        EVS_PRIMARY_MODE_BITRATE_64_0_KBPS = 9,
        /** EVS bitrate 96 kbps */
        EVS_PRIMARY_MODE_BITRATE_96_0_KBPS = 10,
        /** EVS bitrate 128 kbps */
        EVS_PRIMARY_MODE_BITRATE_128_0_KBPS = 11,
        /** EVS bitrate MAX */
        EVS_PRIMARY_MODE_BITRATE_MAX = EVS_PRIMARY_MODE_BITRATE_128_0_KBPS
    };

    /** Specifies the EVS codec encoding bandwidth options */
    enum
    {
        /** EVS narrow band only */
        EVS_ENCODED_BW_TYPE_NB = 0,
        /** EVS wide band only */
        EVS_ENCODED_BW_TYPE_WB = 1,
        /** EVS super wide band only */
        EVS_ENCODED_BW_TYPE_SWB = 2,
        /** EVS full band only */
        EVS_ENCODED_BW_TYPE_FB = 3,
        /** EVS nb and wb */
        EVS_ENCODED_BW_TYPE_NB_WB = 4,
        /** EVS nb, wb and swb */
        EVS_ENCODED_BW_TYPE_NB_WB_SWB = 5,
        /** EVS nb, wb, swb and fb */
        EVS_ENCODED_BW_TYPE_NB_WB_SWB_FB = 6
    };

    /** Enum EVS Bandwidth Type*/
    enum
    {
        /** EVS narrow band */
        EVS_BANDWIDTH_NB = 0,
        /** EVS wide band */
        EVS_BANDWIDTH_WB = 1,
        /** EVS super wide band */
        EVS_BANDWIDTH_SWB = 2,
        /** EVS full band */
        EVS_BANDWIDTH_FB = 3,
        EVS_BANDWIDTH_MAX = EVS_BANDWIDTH_FB
    };

    static const IMS_BOOL DEFAULT_DTX_RECV;
    static const IMS_SINT32 DEFAULT_HF_ONLY;
    static const IMS_SINT32 DEFAULT_EVS_MODESWITCH;
    static const IMS_SINT32 DEFAULT_BR;
    static const IMS_SINT32 DEFAULT_BR_LIST;
    static const IMS_SINT32 DEFAULT_BW_LIST;
    static const IMS_SINT32 DEFAULT_CMR;
    static const IMS_SINT32 DEFAULT_CH_AW_RECV;
    static const IMS_SINT32 NOT_DEFINED;
    static const IMS_SINT32 CMR_NOT_PRESENT;

public:
    /**
     * @brief Construct a new codec evs config
     *
     * @param nType audio codec type (ex: evs)
     * @param nPayloadTypeNum payload type number
     */
    CodecEvsConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum);
    /**
     * @brief Destroy the codec evs config
     *
     */
    virtual ~CodecEvsConfig() override;
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
     * @brief Get the information whether to include dtx in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include dtx to sdp
     * IMS_FALSE Not include dtx to sdp
     */
    IMS_BOOL GetVisibleDtx() const;
    /**
     * @brief Get the dtx recv
     *
     * @return IMS_BOOL Return true if dtx-recv is supported
     * Return false if dtx-recv is not supported
     */
    IMS_BOOL GetDtxRecv() const;
    /**
     * @brief Get the information whether to include Header-full only in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include HfOnly to sdp
     * IMS_FALSE Not include HfOnly to sdp
     */
    IMS_BOOL GetVisibleHfOnly() const;
    /**
     * @brief Get the header-full only mode
     *
     * @return IMS_SINT32 Return the HFonly mode
     */
    IMS_SINT32 GetHfOnly() const;
    /**
     * @brief Get the information whether to include Evs mode switch in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include evs mode switch to sdp
     * IMS_FALSE Not include evs mode switc to sdp
     */
    IMS_BOOL GetVisibleEvsModeSwitch() const;
    /**
     * @brief Get the evs mode switch
     *
     * @return IMS_SINT32 Return the evs-mode-switch
     */
    IMS_SINT32 GetEvsModeSwitch() const;
    /**
     * @brief Get the bitrate list
     *
     * @return IMS_UINT32 Return the bitrate list
     */
    IMS_UINT32 GetBrList() const;
    /**
     * @brief Get the bitrate
     *
     * @return IMS_SINT32 Return the bitrate
     */
    IMS_SINT32 GetBr() const;
    /**
     * @brief Get the bandwidth list
     *
     * @return IMS_UINT32 Return the bandwidth list
     */
    IMS_UINT32 GetBwList() const;
    /**
     * @brief Get the information whether to include cmr in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include cmr to sdp
     * IMS_FALSE Not include cmr to sdp
     */
    IMS_BOOL GetVisibleCmr() const;
    /**
     * @brief Get the cmr
     *
     * @return IMS_SINT32 Return cmr (code-mode_request) value
     */
    IMS_SINT32 GetCmr() const;
    /**
     * @brief Get the information whether to include channel-aware recv in sdp
     *
     * @return IMS_BOOL Return IMS_TRUE Include channel-aware recv to sdp
     * IMS_FALSE Not include channel-aware recv to sdp
     */
    IMS_BOOL GetVisibleChAwareRecv() const;
    /**
     * @brief Get the channel-aware recv
     *
     * @return IMS_SINT32 Return the channel-aware recv value
     */
    IMS_SINT32 GetChAwareRecv() const;
    /**
     * @brief Create the default Evs codec
     *
     */
    VIRTUAL void CreateDefaultEvsCodec();

private:
    static IMS_SINT32 ConvertEvsBitrateToList(IN IMS_SINT32 nBrStart, IN IMS_SINT32 nBrEnd);
    static IMS_SINT32 ConvertEvsBandwidthToList(IN AString strBandwidth);
    static IMS_SINT32 GetEvsBitrateFromList(IN IMS_SINT32 nBitrateList);
    static IMS_SINT32 CheckEvsBandwidthWithBitrate(IN IMS_SINT32 nBwList, IN IMS_SINT32 nBrList);

private:
    IMS_BOOL m_bVisibleDtx;     // Indicate whether dtx attribute to display in SDP
    IMS_BOOL m_bVisibleHfOnly;  // Indicate whether hf-only attribute to display in SDP
    IMS_BOOL m_bVisibleCmr;
    IMS_BOOL m_bVisibleEvsModeSwitch;  // Indicate whether EvsModeSwitch attribute to display in SDP
    IMS_BOOL m_bVisibleChannelAwMode;  // Indicate whether channel-aware mode to display in SDP
    IMS_BOOL m_bVisibleAmrIOModeSet;
    IMS_BOOL m_bVisibleChAwRecv;

    IMS_BOOL m_bDtxRecv;          // 1(default) is turn on DTX
    IMS_SINT32 m_nHfOnly;         // 0(default) is both used, other is only hf format used
    IMS_SINT32 m_nEvsModeSwitch;  // 0(default) is "primary mode start"
    IMS_SINT32 m_nBrList;         // EVS primary mode bitrate range (kbps)
    IMS_SINT32 m_nBwList;  // bw has a value from the set: nb, wb, swb, fb, nb-wb, nb-swb, and nb-fb
    IMS_SINT32 m_nCmr;
    IMS_UINT32 m_nAmrIOModeSetList;
    IMS_UINT32 m_nDefaultAmrIOModeSetList;
    IMS_SINT32 m_nChAwRecv;  // -1: disabled / 0(default)
};

#endif
