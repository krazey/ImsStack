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
 *  @brief      Impl of MMPFBoardConfigInfo
 */
#include <mmpf/MMPFBoardConfigInfo.h>

mmpf_int32 MMPFBoardConfigInfo::GetChipset()
{
    return MMPF_BC_CHIPSET_QCT;
}

mmpf_uint32 MMPFBoardConfigInfo::GetAudioCodecSupport()
{
    mmpf_uint32 eAudioCodecSupport = 0x00;

    // Default Codec
    eAudioCodecSupport |= MMPF_BC_SUPPORT_AUDIO_CODEC_AMR;
    eAudioCodecSupport |= MMPF_BC_SUPPORT_AUDIO_CODEC_AMRWB;
    eAudioCodecSupport |= MMPF_BC_SUPPORT_AUDIO_CODEC_G711;

    return eAudioCodecSupport;
}

mmpf_bool MMPFBoardConfigInfo::IsAMRWBCodecSupport()
{
    return MMPF_TRUE;
}

mmpf_bool MMPFBoardConfigInfo::IsEVSCodecSupport()
{
    return MMPF_FALSE;
}

mmpf_bool MMPFBoardConfigInfo::IsMTKChipset()
{
    return MMPF_FALSE;
}

mmpf_bool MMPFBoardConfigInfo::IsMTKVoNRSocketSupport()
{
    return MMPF_FALSE;
}

mmpf_bool MMPFBoardConfigInfo::IsEVSSWBSupport()
{
    return MMPF_FALSE;
}

mmpf_bool MMPFBoardConfigInfo::IsHEVCSupport()
{
    return MMPF_FALSE;
}

mmpf_bool MMPFBoardConfigInfo::IsRTCPXRSupport()
{
    return MMPF_FALSE;
}

mmpf_bool MMPFBoardConfigInfo::IsCHPSupport()
{
    return MMPF_FALSE;
}

mmpf_bool MMPFBoardConfigInfo::IsTargetOperator(const mmpf_str* /*target*/)
{
    return MMPF_FALSE;
}

// H.264
void MMPFBoardConfigInfo::GetSpropParameterSet(mmpf_uint32 /*nWidth*/, mmpf_uint32 /*nHeight*/,
        mmpf_uint32 /*nProfile*/, mmpf_uint32 /*nLevel*/, mmpf_str* /*szBuffSpropparam*/)
{
}

// HEVC - VPS,SPS,PPS
void MMPFBoardConfigInfo::GetHEVCConfigFrameSet(mmpf_uint32 /*nWidth*/, mmpf_uint32 /*nHeight*/,
        mmpf_uint32 /*nProfile*/, mmpf_uint32 /*nLevel*/, mmpf_str* /*szBuffSpropparam*/)
{
}
