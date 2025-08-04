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

#ifndef MTC_DEF_H_
#define MTC_DEF_H_

#include "AString.h"
#include "ImsTypeDef.h"

enum class UpdateType
{
    NONE,
    NORMAL,
    HOLD,
    RESUME,
    SESSION,
    CONF,
    REFRESH,
    SRVCC_RECOVERED_CANCEL,
    SRVCC_RECOVERED_FAILURE,
    LOCATION,
};

enum class PemType
{
    NONE = 0,
    SENDRECV = 1,
    SENDONLY = 2,
    RECVONLY = 3,
    INACTIVE = 4,
};

enum
{
    MEDIATYPE_NONE = 0,
    MEDIATYPE_AUDIO = (0x00000001),
    MEDIATYPE_VIDEO = (0x00000002),
    MEDIATYPE_TEXT = (0x00000004),
};

enum
{
    DIRECTION_INVALID = -1,
    DIRECTION_INACTIVE = 0,
    DIRECTION_RECEIVE = 1,
    DIRECTION_SEND = 2,
    DIRECTION_SEND_RECEIVE = 3,
};

enum
{
    MEDIA_QUALITY_NONE = 0,
    MEDIA_QUALITY_NOTUSED = 99,
};

enum
{
    AUDIO_QUALITY_NONE = MEDIA_QUALITY_NONE,

    AUDIO_QUALITY_AMR_NB = 1,
    AUDIO_QUALITY_AMR_WB = 2,
    AUDIO_QUALITY_EVS = 3,
    AUDIO_QUALITY_G711_PCMU = 4,
    AUDIO_QUALITY_G711_PCMA = 5,
    AUDIO_QUALITY_EVS_NB = 6,
    AUDIO_QUALITY_EVS_WB = 7,
    AUDIO_QUALITY_EVS_SWB = 8,
    AUDIO_QUALITY_EVS_FB = 9,
    AUDIO_QUALITY_MAX = 10,

    AUDIO_QUALITY_NOTUSED = MEDIA_QUALITY_NOTUSED,
};

enum
{
    VIDEO_QUALITY_NONE = MEDIA_QUALITY_NONE,

    VIDEO_QUALITY_QCIF = 1,
    VIDEO_QUALITY_QVGA_LS = 2,
    VIDEO_QUALITY_QVGA_PR = 3,
    VIDEO_QUALITY_VGA_LS = 4,
    VIDEO_QUALITY_VGA_PR = 5,
    VIDEO_QUALITY_CIF_LS = 6,
    VIDEO_QUALITY_CIF_PR = 7,
    VIDEO_QUALITY_QCIF_PR = 8,
    VIDEO_QUALITY_SQCIF_LS = 9,
    VIDEO_QUALITY_SQCIF_PR = 10,
    VIDEO_QUALITY_SIF_LS = 11,
    VIDEO_QUALITY_SIF_PR = 12,
    VIDEO_QUALITY_HD_LS = 13,
    VIDEO_QUALITY_HD_PR = 14,

    VIDEO_QUALITY_NOTUSED = MEDIA_QUALITY_NOTUSED,
};

enum
{
    TEXT_QUALITY_NONE = MEDIA_QUALITY_NONE,
    TEXT_QUALITY_T140 = 1,
    TEXT_QUALITY_T140_RED = 2,
    TEXT_QUALITY_NOTUSED = MEDIA_QUALITY_NOTUSED,
};

enum
{
    GTT_MODE_INVALID = -1,
    GTT_MODE_INACTIVE = 0,
    GTT_MODE_FULL = 1,
    GTT_MODE_HCO = 2,
    GTT_MODE_VCO = 3,
};

enum class SuppType
{
    NONE = -1,
    CALLER_ID = 0,
    CNAP = 1,
    CDIV_CAUSE = 2,
    CDIV_HISTORY = 3,
    CW = 4,
    ENFORCE_LT = 5,
    TARGET_URI = 6,
    CALLING_NUM_VERIFICATION = 7,
    TIP = 8,
    GEOLOCATION = 9,
    CALL_PULL = 10,
    CALL_COMPOSER_PRIORITY = 11,
    CALL_COMPOSER_SUBJECT = 12,
    CALL_COMPOSER_LOCATION_LAT = 13,
    CALL_COMPOSER_LOCATION_LONG = 14,
    CALL_COMPOSER_PICTURE_URL = 15,
    CALL_COMPOSER_IS_BUSINESS = 16,
    SESSION_ID = 17,
};

enum class OipType
{
    INVALID = -1,
    NONE = 0,
    IDENTITY = 1,
    RESTRICTED = 2,
    UNKNOWN = 3,
    PAYPHONE = 4,
    UNAVAILABLE = 5,
};

enum class CdivCause
{
    NONE = 0,
    UNCONDITION = 1,
    BUSY = 2,
    REJECT = 3,
    NOANSWER = 4,
    NO_REPLY = 5,
    DEFLECTION = 6,
    NOT_LOGGED_IN = 7,
    DEFLECTION_ALERTING = 8,
    NOT_REACHABLE = 9,
};

enum
{
    CW_TYPE_NONE = 0,
    CW_TYPE_TERMINAL = 1, /* CW NOTIFICATION */
    CW_TYPE_NETWORK = 2,  /* CW INDICATION */
};

enum
{
    CALLERID_NONE = 0,
    CALLERID_NETWORK = 1,
    CALLERID_RESTRICTED = 2,
    CALLERID_IDENTITY = 3,
};

enum
{
    TIP_TYPE_NONE = 0,
    TIP_TYPE_IDENTITY = 1,
    TIP_TYPE_RESTRICTED = 2,
};

enum
{
    TIP_MODE_NONE = 0,
    TIP_MODE_TEMPORARY = 1,
    TIP_MODE_PERMANENT = 2,
};

enum
{
    CALLING_NUM_VERSTAT_NONE = 0,
    CALLING_NUM_VERSTAT_VERIFIED = 1,
    CALLING_NUM_VERSTAT_NOT_VERIFIED = 2,
};

enum
{
    CONF_CREATE_NONE = 0,

    CONF_CREATE_START = 1,
    CONF_CREATE_STARTED = 2,
    CONF_CREATE_MERGE = 3,
    CONF_CREATE_EXPAND = 4,
    CONF_CREATE_EXPANDED_BY = 5,
};

enum
{
    CALL_COMPOSER_PRIORITY_NONE = 0,
    CALL_COMPOSER_PRIORITY_URGENT = 1,
};

struct MediaInfo
{
public:
    inline MediaInfo() :
            eAudioDirection(DIRECTION_INVALID),
            eVideoDirection(DIRECTION_INVALID),
            eTextDirection(DIRECTION_INVALID),
            eAudioQuality(AUDIO_QUALITY_NONE),
            eVideoQuality(VIDEO_QUALITY_NONE),
            eGttMode(GTT_MODE_INVALID)
    {
    }
    inline MediaInfo(IN const MediaInfo& objRhs) :
            eAudioDirection(objRhs.eAudioDirection),
            eVideoDirection(objRhs.eVideoDirection),
            eTextDirection(objRhs.eTextDirection),
            eAudioQuality(objRhs.eAudioQuality),
            eVideoQuality(objRhs.eVideoQuality),
            eGttMode(objRhs.eGttMode)
    {
    }
    inline MediaInfo(IN IMS_SINT32 eInitAudioDirection, IN IMS_SINT32 eInitVideoDirection,
            IN IMS_SINT32 eInitTextDirection, IN IMS_UINT32 eInitAudioQuality,
            IN IMS_UINT32 eInitVideoQuality, IN IMS_SINT32 eInitGttMode) :
            eAudioDirection(eInitAudioDirection),
            eVideoDirection(eInitVideoDirection),
            eTextDirection(eInitTextDirection),
            eAudioQuality(eInitAudioQuality),
            eVideoQuality(eInitVideoQuality),
            eGttMode(eInitGttMode)
    {
    }
    inline ~MediaInfo() {}

public:
    inline MediaInfo& operator=(IN const MediaInfo& objRhs)
    {
        if (this != &objRhs)
        {
            eAudioDirection = objRhs.eAudioDirection;
            eVideoDirection = objRhs.eVideoDirection;
            eTextDirection = objRhs.eTextDirection;
            eAudioQuality = objRhs.eAudioQuality;
            eVideoQuality = objRhs.eVideoQuality;
            eGttMode = objRhs.eGttMode;
        }

        return (*this);
    }

    IMS_BOOL operator==(const MediaInfo& objRhs) const
    {
        if (this == &objRhs)
        {
            return IMS_TRUE;
        }

        return eAudioDirection == objRhs.eAudioDirection &&
                eVideoDirection == objRhs.eVideoDirection &&
                eTextDirection == objRhs.eTextDirection && eAudioQuality == objRhs.eAudioQuality &&
                eVideoQuality == objRhs.eVideoQuality && eGttMode == objRhs.eGttMode;
    }

    IMS_BOOL operator!=(const MediaInfo& objRhs) const { return !(*this == objRhs); }

public:
    IMS_SINT32 eAudioDirection;
    IMS_SINT32 eVideoDirection;
    IMS_SINT32 eTextDirection;
    IMS_UINT32 eAudioQuality;
    IMS_UINT32 eVideoQuality;
    IMS_SINT32 eGttMode;
};

class SuppService
{
public:
    inline SuppService() :
            strValue(AString::ConstNull()),
            nValue(0),
            bValue(IMS_FALSE)
    {
    }
    inline SuppService(IN const SuppService& objRHS) :
            strValue(objRHS.strValue),
            nValue(objRHS.nValue),
            bValue(objRHS.bValue)
    {
    }
    inline ~SuppService() {}

public:
    inline SuppService& operator=(IN const SuppService& objRhs)
    {
        if (this != &objRhs)
        {
            strValue = objRhs.strValue;
            nValue = objRhs.nValue;
            bValue = objRhs.bValue;
        }

        return (*this);
    }

    IMS_BOOL operator==(const SuppService& objRhs) const
    {
        if (this == &objRhs)
        {
            return IMS_TRUE;
        }

        return strValue == objRhs.strValue && nValue == objRhs.nValue && bValue == objRhs.bValue;
    }

    IMS_BOOL operator!=(const SuppService& objRhs) const { return !(*this == objRhs); }

public:
    AString strValue;
    IMS_SINT32 nValue;
    IMS_BOOL bValue;
};

#endif
