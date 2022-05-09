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

/*!
 *    @brief        new interface of the MMPF Board Config
 */
#ifndef MMPF_BOARDCONFIGINFO_H_INCLUDED
#define MMPF_BOARDCONFIGINFO_H_INCLUDED

#include <mmpf/MMPFConfigure.h>
#include <mmpf/MMPFDefinition.h>

/**
 *    @class        MMPFBoardConfigInfo
 *
 *    @brief        Get Model Board Config Info
 *
 */
class MMPFBoardConfigInfo
{
public:
    enum MMPF_BC_CHIPSET
    {
        MMPF_BC_CHIPSET_NONE = -1,
        MMPF_BC_CHIPSET_QCT = 0,
        MMPF_BC_CHIPSET_MTK,
        MMPF_BC_CHIPSET_XXX,
        MMPF_BC_CHIPSET_MAX,
    };

    enum MMPF_BC_SUPPORT_AUDIO_CODEC
    {
        MMPF_BC_SUPPORT_AUDIO_CODEC_AMR = 0x01,
        MMPF_BC_SUPPORT_AUDIO_CODEC_AMRWB = 0x02,
        MMPF_BC_SUPPORT_AUDIO_CODEC_EVS = 0x04,
        MMPF_BC_SUPPORT_AUDIO_CODEC_G711 = 0x08,
    };

public:
    static mmpf_int32 GetChipset();
    static mmpf_uint32 GetAudioCodecSupport();
    static mmpf_bool IsAMRWBCodecSupport();
    static mmpf_bool IsEVSCodecSupport();
    static mmpf_bool IsMTKChipset();
    static mmpf_bool IsMTKVoNRSocketSupport();
    static mmpf_bool IsEVSSWBSupport();
    static mmpf_bool IsHEVCSupport();
    static mmpf_bool IsRTCPXRSupport();
    static mmpf_bool IsCHPSupport();
    static mmpf_bool IsTargetOperator(const mmpf_str* target);
    static void GetSpropParameterSet(mmpf_uint32 nWidth, mmpf_uint32 nHeight, mmpf_uint32 nProfile,
            mmpf_uint32 nLevel, mmpf_str* szBuffSpropparam);
    static void GetHEVCConfigFrameSet(mmpf_uint32 nWidth, mmpf_uint32 nHeight, mmpf_uint32 nProfile,
            mmpf_uint32 nLevel, mmpf_str* szBuffSpropparam);
};

#endif  // MMPF_BOARDCONFIGINFO_H_INCLUDED
