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

#include "ServiceTrace.h"
#include "video/VideoDef.h"
#include "video/VideoProfileUtil.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC GLOBAL void VideoProfileUtil::GetWidthHeightFromResolution(
        VIDEO_RESOLUTION eResolution, IMS_UINT32* nWidth, IMS_UINT32* nHeight)
{
    switch (eResolution)
    {
        case VIDEO_RESOLUTION_QCIF_PR:
            *nWidth = 144;
            *nHeight = 176;
            break;
        case VIDEO_RESOLUTION_QVGA_LS:
            *nWidth = 320;
            *nHeight = 240;
            break;
        case VIDEO_RESOLUTION_QVGA_PR:
            *nWidth = 240;
            *nHeight = 320;
            break;
        case VIDEO_RESOLUTION_VGA_LS:
            *nWidth = 640;
            *nHeight = 480;
            break;
        case VIDEO_RESOLUTION_VGA_PR:
            *nWidth = 480;
            *nHeight = 640;
            break;
        case VIDEO_RESOLUTION_QCIF_LS:
            *nWidth = 176;
            *nHeight = 144;
            break;
        case VIDEO_RESOLUTION_CIF_LS:
            *nWidth = 352;
            *nHeight = 288;
            break;
        case VIDEO_RESOLUTION_CIF_PR:
            *nWidth = 288;
            *nHeight = 352;
            break;
        case VIDEO_RESOLUTION_SQCIF_LS:
            *nWidth = 128;
            *nHeight = 96;
            break;
        case VIDEO_RESOLUTION_SQCIF_PR:
            *nWidth = 96;
            *nHeight = 128;
            break;
        case VIDEO_RESOLUTION_SIF_LS:
            *nWidth = 352;
            *nHeight = 240;
            break;
        case VIDEO_RESOLUTION_SIF_PR:
            *nWidth = 240;
            *nHeight = 352;
            break;
        case VIDEO_RESOLUTION_HD_LS:
            *nWidth = 1280;
            *nHeight = 720;
            break;
        case VIDEO_RESOLUTION_HD_PR:
            *nWidth = 720;
            *nHeight = 1280;
            break;
        case VIDEO_RESOLUTION_FHD_LS:
            *nWidth = 1920;
            *nHeight = 1080;
            break;
        case VIDEO_RESOLUTION_FHD_PR:
            *nWidth = 1080;
            *nHeight = 1920;
            break;
        default:
            *nWidth = 0;
            *nHeight = 0;
            break;
    }

    IMS_TRACE_D("GetWidthHeightFromResolution - resolutionID[%d], nWidth[%d], nHeight[%d]",
            eResolution, *nWidth, *nHeight);
}

PUBLIC GLOBAL VIDEO_RESOLUTION VideoProfileUtil::GetResolutionFromWidthHeight(
        IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight)
{
    if (nWidth == 176 && nHeight == 144)
    {
        return VIDEO_RESOLUTION_QCIF_LS;
    }
    else if (nWidth == 144 && nHeight == 176)
    {
        return VIDEO_RESOLUTION_QCIF_PR;
    }
    else if (nWidth == 320 && nHeight == 240)
    {
        return VIDEO_RESOLUTION_QVGA_LS;
    }
    else if (nWidth == 240 && nHeight == 320)
    {
        return VIDEO_RESOLUTION_QVGA_PR;
    }
    else if (nWidth == 640 && nHeight == 480)
    {
        return VIDEO_RESOLUTION_VGA_LS;
    }
    else if (nWidth == 480 && nHeight == 640)
    {
        return VIDEO_RESOLUTION_VGA_PR;
    }
    else if (nWidth == 352 && nHeight == 288)
    {
        return VIDEO_RESOLUTION_CIF_LS;
    }
    else if (nWidth == 288 && nHeight == 352)
    {
        return VIDEO_RESOLUTION_CIF_PR;
    }
    else if (nWidth == 128 && nHeight == 96)
    {
        return VIDEO_RESOLUTION_SQCIF_LS;
    }
    else if (nWidth == 96 && nHeight == 128)
    {
        return VIDEO_RESOLUTION_SQCIF_PR;
    }
    else if (nWidth == 352 && nHeight == 240)
    {
        return VIDEO_RESOLUTION_SIF_LS;
    }
    else if (nWidth == 240 && nHeight == 352)
    {
        return VIDEO_RESOLUTION_SIF_PR;
    }
    else if (nWidth == 1280 && nHeight == 720)
    {
        return VIDEO_RESOLUTION_HD_LS;
    }
    else if (nWidth == 720 && nHeight == 1280)
    {
        return VIDEO_RESOLUTION_HD_PR;
    }
    else if (nWidth == 1920 && nHeight == 1080)
    {
        return VIDEO_RESOLUTION_FHD_LS;
    }
    else if (nWidth == 1080 && nHeight == 1920)
    {
        return VIDEO_RESOLUTION_FHD_PR;
    }
    else
    {
        return VIDEO_RESOLUTION_INVALID;
    }
}

PUBLIC GLOBAL VIDEO_PROFILE_AVC VideoProfileUtil::GetAvcProfileFromProfileLevelId(
        const AString& strProfileLevelId)
{
    // Table 5.  Combinations of profile_idc and profile-iop (RFC 6184)
    //      Profile   profile_idc         profile-iop
    //                (hexadecimal)       (binary)
    //
    //        CB              42 (B)       x1xx0000
    //               same as: 4D (M)       1xxx0000
    //               same as: 58 (E)       11xx0000
    //        B               42 (B)       x0xx0000
    //               same as: 58 (E)       10xx0000
    //        M               4D (M)       0x0x0000
    //        E               58           00xx0000
    //        H               64           00000000

    if (strProfileLevelId.GetLength() < 3)
    {
        return AVC_PROFILE_NOT_USED;
    }

    IMS_UINT32 nProfileIop = 0;
    VIDEO_PROFILE_AVC nReProfile = AVC_PROFILE_NOT_USED;

    if ((IMS_UINT32)strProfileLevelId[2] >= (IMS_UINT32)'0' &&
            (IMS_UINT32)strProfileLevelId[2] <= (IMS_UINT32)'9')
    {
        nProfileIop = (IMS_UINT32)(strProfileLevelId[2] - '0');
    }
    else if ((IMS_UINT32)strProfileLevelId[2] >= (IMS_UINT32)'A' &&
            (IMS_UINT32)strProfileLevelId[2] <= (IMS_UINT32)'F')
    {
        nProfileIop = (IMS_UINT32)(strProfileLevelId[2] - 'A' + 10);
    }
    else if ((IMS_UINT32)strProfileLevelId[2] >= (IMS_UINT32)'a' &&
            (IMS_UINT32)strProfileLevelId[2] <= (IMS_UINT32)'f')
    {
        nProfileIop = (IMS_UINT32)(strProfileLevelId[2] - 'a' + 10);
    }
    else
    {
        IMS_TRACE_E(0, "GetAvcProfileFromProfileLevelId - INVALID Profil-iop[%c]",
                (IMS_CHAR)strProfileLevelId[2], 0, 0);
    }

    IMS_TRACE_D("GetAvcProfileFromProfileLevelId - nProfileIop[%d]", nProfileIop, 0, 0);

    // baseline profile
    if ((IMS_UINT32)strProfileLevelId[0] == (IMS_UINT32)'4' &&
            (IMS_UINT32)strProfileLevelId[1] == (IMS_UINT32)'2')
    {
        nReProfile = ((nProfileIop & 0x04) != 0) ? AVC_PROFILE_CB : AVC_PROFILE_B;
    }
    // main profile
    else if ((IMS_UINT32)strProfileLevelId[0] == (IMS_UINT32)'4' &&
            (IMS_UINT32)strProfileLevelId[1] == (IMS_UINT32)'D')
    {
        nReProfile = (nProfileIop & 0x08) ? AVC_PROFILE_CB : AVC_PROFILE_M;
    }
    // extended profile
    else if ((IMS_UINT32)strProfileLevelId[0] == (IMS_UINT32)'5' &&
            (IMS_UINT32)strProfileLevelId[1] == (IMS_UINT32)'8')
    {
        if (nProfileIop & 0x0C)
        {
            nReProfile = AVC_PROFILE_CB;
        }
        else if ((nProfileIop & 0x08) != 0 && (nProfileIop & 0x04) == 0)
        {
            nReProfile = AVC_PROFILE_M;
        }
        else
        {
            nReProfile = AVC_PROFILE_E;
        }
    }
    // high profile
    else if ((IMS_UINT32)strProfileLevelId[0] == (IMS_UINT32)'6' &&
            (IMS_UINT32)strProfileLevelId[1] == (IMS_UINT32)'4')
    {
        nReProfile = AVC_PROFILE_H;
    }

    IMS_TRACE_D("GetAvcProfileFromProfileLevelId - nProfile[%d]", nReProfile, 0, 0);

    return nReProfile;
}

PUBLIC GLOBAL IMS_UINT32 VideoProfileUtil::GetAvcLevelFromProfileLevelId(
        const AString& strProfileLevelId)
{
    IMS_UINT32 nLevel = 12;

    if (strProfileLevelId.GetLength() < 6)
    {
        return nLevel;
    }

    // IMS_TRACE_D("GetAvcLevelFromProfileLevelId - strProfileLevelId[%s]",
    // strProfileLevelId.GetStr(),0,0);
    if ((IMS_UINT32)strProfileLevelId[4] >= (IMS_UINT32)'A' &&
            (IMS_UINT32)strProfileLevelId[4] <= (IMS_UINT32)'F')
    {
        nLevel = ((IMS_UINT32)(strProfileLevelId[4] - 'A') + 10) * 16;
    }
    else if ((IMS_UINT32)strProfileLevelId[4] >= (IMS_UINT32)'a' &&
            (IMS_UINT32)strProfileLevelId[4] <= (IMS_UINT32)'f')
    {
        nLevel = ((IMS_UINT32)(strProfileLevelId[4] - 'a') + 10) * 16;
    }
    else
    {
        nLevel = (IMS_UINT32)(strProfileLevelId[4] - '0') * 16;
    }

    if ((IMS_UINT32)strProfileLevelId[5] >= (IMS_UINT32)'A' &&
            (IMS_UINT32)strProfileLevelId[5] <= (IMS_UINT32)'F')
    {
        nLevel += ((IMS_UINT32)(strProfileLevelId[5] - 'A') + 10);
    }
    else if ((IMS_UINT32)strProfileLevelId[5] >= (IMS_UINT32)'a' &&
            (IMS_UINT32)strProfileLevelId[5] <= (IMS_UINT32)'f')
    {
        nLevel += ((IMS_UINT32)(strProfileLevelId[5] - 'a') + 10);
    }
    else
    {
        nLevel += (IMS_UINT32)(strProfileLevelId[5] - '0');
    }

    IMS_TRACE_D("GetAvcLevelFromProfileLevelId - nLevel[%d]", nLevel, 0, 0);

    return nLevel;
}

PRIVATE
VIDEO_RESOLUTION VideoProfileUtil::GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel)
{
    IMS_TRACE_D("GetAvcMaxResolutionFromLevel(): Level[%d]", nLevel, 0, 0);

    if (nLevel > 31)
    {
        nLevel = 31;
    }

    // default resolution is portrait
    switch (nLevel)
    {
        case 31:
            return VIDEO_RESOLUTION_HD_PR;
        case 30:
        case 22:
            return VIDEO_RESOLUTION_VGA_PR;
        case 21:
        case 20:
        case 14:
        case 13:
        case 12:
        case 11:
            return VIDEO_RESOLUTION_CIF_PR;
        case 10:
            return VIDEO_RESOLUTION_QCIF_PR;
        default:
            return VIDEO_RESOLUTION_NOT_USED;
    }
}

PUBLIC GLOBAL IMS_SINT32 VideoProfileUtil::CompareResolution(
        IN VIDEO_RESOLUTION eResolution1, IN VIDEO_RESOLUTION eResolution2)
{
    IMS_UINT32 nWidth1 = 0, nHeight1 = 0;
    GetWidthHeightFromResolution(eResolution1, &nWidth1, &nHeight1);
    IMS_UINT64 nPixels1 = (IMS_UINT64)nWidth1 * nHeight1;

    IMS_UINT32 nWidth2 = 0, nHeight2 = 0;
    GetWidthHeightFromResolution(eResolution2, &nWidth2, &nHeight2);
    IMS_UINT64 nPixels2 = (IMS_UINT64)nWidth2 * nHeight2;

    if (nPixels1 > nPixels2)
    {
        return 1;
    }
    else if (nPixels1 < nPixels2)
    {
        return -1;
    }

    return 0;
}

namespace
{
// Helper class for reading bits from a byte stream for SPS parsing.
class BitReader
{
public:
    BitReader(const IMS_BYTE* pData, IMS_UINT32 nSize) :
            mData(pData),
            mSize(nSize),
            mBitOffset(0)
    {
    }

    IMS_BOOL readBits(IMS_UINT32 nSize, IMS_UINT32* pOut)
    {
        if (nSize > 32)
            return IMS_FALSE;
        if (mBitOffset + nSize > mSize * 8)
            return IMS_FALSE;

        IMS_UINT32 result = 0;
        for (IMS_UINT32 i = 0; i < nSize; ++i)
        {
            IMS_UINT32 byteIndex = (mBitOffset) / 8;
            IMS_UINT32 bitIndex = (mBitOffset) % 8;
            result = (result << 1) | ((mData[byteIndex] >> (7 - bitIndex)) & 1);
            mBitOffset++;
        }

        if (pOut)
        {
            *pOut = result;
        }
        return IMS_TRUE;
    }

    IMS_BOOL readUe(IMS_UINT32* pOut = nullptr)
    {
        IMS_UINT32 nLeadingZeroBits = 0;
        while (true)
        {
            IMS_UINT32 bit;
            if (!readBits(1, &bit))
            {
                return IMS_FALSE;
            }
            if (bit == 1)
            {
                break;
            }

            nLeadingZeroBits++;
            if (nLeadingZeroBits >= 32)
            {
                return IMS_FALSE;
            }
        }

        IMS_UINT32 value = 0;
        if (nLeadingZeroBits > 0)
        {
            if (!readBits(nLeadingZeroBits, &value))
            {
                return IMS_FALSE;
            }
        }

        if (pOut)
        {
            *pOut = (1 << nLeadingZeroBits) - 1 + value;
        }

        return IMS_TRUE;
    }

    IMS_BOOL skipBits(IMS_UINT32 nSize)
    {
        if (mBitOffset + nSize > mSize * 8)
        {
            return IMS_FALSE;
        }

        mBitOffset += nSize;
        return IMS_TRUE;
    }

private:
    const IMS_BYTE* mData;
    const IMS_UINT32 mSize;
    IMS_UINT32 mBitOffset;
};

}  // namespace

PUBLIC GLOBAL IMS_BOOL VideoProfileUtil::GetWidthHeightFromSpropParam(IN VIDEO_CODEC eCodecType,
        IN const AString& strSpropParam, OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    if (strSpropParam.IsEmpty() || nImageWidth == IMS_NULL || nImageHeight == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<AString> lstParamSet = strSpropParam.Split(',');
    if (lstParamSet.IsEmpty())
    {
        return IMS_FALSE;
    }

    AString objSpsBase64;
    if (eCodecType == VIDEO_CODEC_AVC)
    {
        if (lstParamSet.GetSize() < 1)
            return IMS_FALSE;
        objSpsBase64 = lstParamSet.GetAt(0);
    }
    else if (eCodecType == VIDEO_CODEC_HEVC)
    {
        // TODO: Implement HEVC SPS parsing.
        IMS_TRACE_E(0, "GetWidthHeightFromSpropParam(): HEVC not supported yet", 0, 0, 0);
        return IMS_FALSE;
    }
    else
    {
        return IMS_FALSE;
    }

    AString objSps = AString::FromBase64(objSpsBase64);
    if (objSps.IsEmpty())
    {
        return IMS_FALSE;
    }

    // Skip NAL unit header (1 byte for AVC)
    if (objSps.GetLength() < 2)
        return IMS_FALSE;
    BitReader objBitReader(
            reinterpret_cast<const IMS_BYTE*>(objSps.GetStr() + 1), objSps.GetLength() - 1);

    IMS_UINT32 nProfileIdc;
    if (!objBitReader.readBits(8, &nProfileIdc))
        return IMS_FALSE;
    if (!objBitReader.skipBits(16))
        return IMS_FALSE;  // constraint_flags and level_idc
    if (!objBitReader.readUe())
        return IMS_FALSE;  // seq_parameter_set_id

    IMS_UINT32 nChromaFormatIdc = 1;  // Assume 4:2:0 for non-high profiles

    if (nProfileIdc == 100 || nProfileIdc == 110 || nProfileIdc == 122 || nProfileIdc == 244 ||
            nProfileIdc == 44 || nProfileIdc == 83 || nProfileIdc == 86 || nProfileIdc == 118 ||
            nProfileIdc == 128 || nProfileIdc == 138)
    {
        if (!objBitReader.readUe(&nChromaFormatIdc))
            return IMS_FALSE;
        if (nChromaFormatIdc == 3)
        {
            if (!objBitReader.skipBits(1))
                return IMS_FALSE;  // separate_colour_plane_flag
        }
        if (!objBitReader.readUe())
            return IMS_FALSE;  // bit_depth_luma_minus8
        if (!objBitReader.readUe())
            return IMS_FALSE;  // bit_depth_chroma_minus8
        if (!objBitReader.skipBits(1))
            return IMS_FALSE;  // qpprime_y_zero_transform_bypass_flag
        IMS_UINT32 nSeqScalingMatrixPresentFlag;
        if (!objBitReader.readBits(1, &nSeqScalingMatrixPresentFlag))
            return IMS_FALSE;
        if (nSeqScalingMatrixPresentFlag)
        {
            // Scaling matrix parsing is complex and not needed for resolution.
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSpropParam(): SPS with scaling matrix not fully supported "
                    "for parsing",
                    0, 0, 0);
            return IMS_FALSE;
        }
    }

    if (!objBitReader.readUe())
        return IMS_FALSE;  // log2_max_frame_num_minus4
    IMS_UINT32 nPicOrderCountType;
    if (!objBitReader.readUe(&nPicOrderCountType))
        return IMS_FALSE;
    if (nPicOrderCountType == 0)
    {
        if (!objBitReader.readUe())
            return IMS_FALSE;  // log2_max_pic_order_cnt_lsb_minus4
    }
    else if (nPicOrderCountType == 1)
    {
        if (!objBitReader.skipBits(1))
            return IMS_FALSE;  // delta_pic_order_always_zero_flag
        if (!objBitReader.readUe())
            return IMS_FALSE;  // offset_for_non_ref_pic
        if (!objBitReader.readUe())
            return IMS_FALSE;  // offset_for_top_to_bottom_field
        IMS_UINT32 nNumRefFramesInPicOrderCntCycle;
        if (!objBitReader.readUe(&nNumRefFramesInPicOrderCntCycle))
            return IMS_FALSE;
        for (IMS_UINT32 i = 0; i < nNumRefFramesInPicOrderCntCycle; i++)
        {
            if (!objBitReader.readUe())
                return IMS_FALSE;  // offset_for_ref_frame
        }
    }

    if (!objBitReader.readUe())
        return IMS_FALSE;  // max_num_ref_frames
    if (!objBitReader.skipBits(1))
        return IMS_FALSE;  // gaps_in_frame_num_value_allowed_flag

    IMS_UINT32 nPicWidthInMbsMinus1;
    if (!objBitReader.readUe(&nPicWidthInMbsMinus1))
        return IMS_FALSE;
    IMS_UINT32 nPicHeightInMapUnitsMinus1;
    if (!objBitReader.readUe(&nPicHeightInMapUnitsMinus1))
        return IMS_FALSE;
    IMS_UINT32 nFrameMbsOnlyFlag;
    if (!objBitReader.readBits(1, &nFrameMbsOnlyFlag))
        return IMS_FALSE;

    *nImageWidth = (nPicWidthInMbsMinus1 + 1) * 16;
    *nImageHeight = (2 - nFrameMbsOnlyFlag) * (nPicHeightInMapUnitsMinus1 + 1) * 16;

    if (!nFrameMbsOnlyFlag)
    {
        if (!objBitReader.skipBits(1))
            return IMS_FALSE;  // mb_adaptive_frame_field_flag
    }

    if (!objBitReader.skipBits(1))
        return IMS_FALSE;  // direct_8x8_inference_flag

    IMS_UINT32 nFrameCroppingFlag;
    if (!objBitReader.readBits(1, &nFrameCroppingFlag))
        return IMS_FALSE;
    if (nFrameCroppingFlag)
    {
        IMS_UINT32 nFrameCropLeftOffset, nFrameCropRightOffset, nFrameCropTopOffset,
                nFrameCropBottomOffset;
        if (!objBitReader.readUe(&nFrameCropLeftOffset))
            return IMS_FALSE;
        if (!objBitReader.readUe(&nFrameCropRightOffset))
            return IMS_FALSE;
        if (!objBitReader.readUe(&nFrameCropTopOffset))
            return IMS_FALSE;
        if (!objBitReader.readUe(&nFrameCropBottomOffset))
            return IMS_FALSE;

        IMS_UINT32 nCropUnitX = 1;
        IMS_UINT32 nCropUnitY = 1;

        if (nChromaFormatIdc == 1)  // 4:2:0
        {
            nCropUnitX = 2;
            nCropUnitY = 2;
        }
        else if (nChromaFormatIdc == 2)  // 4:2:2
        {
            nCropUnitX = 2;
        }

        *nImageWidth -= nCropUnitX * (nFrameCropLeftOffset + nFrameCropRightOffset);
        *nImageHeight -= nCropUnitY * (2 - nFrameMbsOnlyFlag) *
                (nFrameCropTopOffset + nFrameCropBottomOffset);
    }

    IMS_TRACE_D("GetWidthHeightFromSpropParam(): width[%d], height[%d]", *nImageWidth,
            *nImageHeight, 0);
    return IMS_TRUE;
}
