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
#include "config/ImsCodec.h"
#include "config/CodecConfigFactory.h"

__IMS_TRACE_TAG_USER_DECL__("MED.CONF");

PUBLIC GLOBAL CodecConfig* CodecConfigFactory::CreateAudioPayloadConfig(
        ICarrierConfig* piCc, IMS_SINT32 nCodec, IMS_SINT32 nPayloadTypeNum, IMS_SINT32 nCodecIdx)
{
    IMS_TRACE_D("CreateAudioPayloadConfig nCodec[%d], nPayloadTypeNum[%d] nCodecIdx[%d]", nCodec,
            nPayloadTypeNum, nCodecIdx);

    if (nCodec == ImsCodec::AUDIO_NONE)
    {
        return IMS_NULL;
    }

    CodecConfig* pCodecConfig = IMS_NULL;

    switch (nCodec)
    {
        case ImsCodec::AUDIO_AMR:
        case ImsCodec::AUDIO_AMR_WB:
        {
            CodecAmrConfig* pAMRConfig = IMS_NULL;
            pAMRConfig = new CodecAmrConfig(nCodec, nPayloadTypeNum);

            if (!pAMRConfig->Create(piCc, nCodecIdx))
            {
                IMS_TRACE_D("pAMRConfig Create failure", 0, 0, 0);

                delete pAMRConfig;
                return IMS_NULL;
            }

            pCodecConfig = pAMRConfig;
        }
        break;
        case ImsCodec::AUDIO_PCMA:
        case ImsCodec::AUDIO_PCMU:
        {
            CodecPcmConfig* pPCMConfig = IMS_NULL;
            pPCMConfig = new CodecPcmConfig(nCodec, nPayloadTypeNum);

            if (!pPCMConfig->Create(piCc, nCodecIdx))
            {
                IMS_TRACE_D("pPCMConfig Create failure", 0, 0, 0);

                delete pPCMConfig;
                return IMS_NULL;
            }

            pCodecConfig = pPCMConfig;
        }
        break;
        case ImsCodec::AUDIO_TELEPHONE_EVENT:
        case ImsCodec::AUDIO_TELEPHONE_EVENT_WB:
        {
            CodecTelephoneEventConfig* pTelephoneEventConfig = IMS_NULL;
            pTelephoneEventConfig = new CodecTelephoneEventConfig(nCodec, nPayloadTypeNum);

            if (!pTelephoneEventConfig->Create(piCc, nCodecIdx))
            {
                IMS_TRACE_D("pTelephoneEventConfig Create failure", 0, 0, 0);

                delete pTelephoneEventConfig;
                return IMS_NULL;
            }

            pCodecConfig = pTelephoneEventConfig;
        }
        break;
        case ImsCodec::AUDIO_EVS:
        {
            CodecEvsConfig* pEVSConfig = IMS_NULL;
            pEVSConfig = new CodecEvsConfig(nCodec, nPayloadTypeNum);

            if (!pEVSConfig->Create(piCc, nCodecIdx))
            {
                IMS_TRACE_D("pEVSConfig Create failure", 0, 0, 0);

                delete pEVSConfig;
                return IMS_NULL;
            }

            pCodecConfig = pEVSConfig;
        }
        break;
    }

    return pCodecConfig;
}

PUBLIC GLOBAL CodecConfig* CodecConfigFactory::CreateVideoPayloadConfig(
        ICarrierConfig* piCc, IMS_SINT32 nCodec, IMS_SINT32 nPayloadTypeNum, IMS_SINT32 nCodecIdx)
{
    if (nCodec == ImsCodec::VIDEO_NONE)
    {
        return IMS_NULL;
    }

    CodecConfig* pCodecConfig = IMS_NULL;

    switch (nCodec)
    {
        case ImsCodec::VIDEO_AVC:
        {
            CodecAvcConfig* pAvcConfig = IMS_NULL;
            pAvcConfig = new CodecAvcConfig(nCodec, nPayloadTypeNum);

            if (!pAvcConfig->Create(piCc, nCodecIdx))
            {
                IMS_TRACE_D("pAvcConfig Create failure", 0, 0, 0);

                delete pAvcConfig;
                return IMS_NULL;
            }

            pCodecConfig = pAvcConfig;
        }
        break;

        case ImsCodec::VIDEO_HEVC:
            // pCodecConfig = new CodecHevcConfig(nCodec, nPayloadTypeNum);   //Need to add later
            break;
    }
    return pCodecConfig;
}

PUBLIC GLOBAL CodecConfig* CodecConfigFactory::CreateTextPayloadConfig(
        ICarrierConfig* piCc, IMS_SINT32 nCodec, IMS_SINT32 nPayloadTypeNum, IMS_SINT32 nCodecIdx)
{
    if (nCodec == ImsCodec::TEXT_NONE)
    {
        return IMS_NULL;
    }

    CodecConfig* pCodecConfig = IMS_NULL;

    switch (nCodec)
    {
        case ImsCodec::TEXT_T140:
        {
            CodecT140Config* pT140Config = IMS_NULL;
            pT140Config = new CodecT140Config(nCodec, nPayloadTypeNum);

            if (!pT140Config->Create(piCc, nCodecIdx))
            {
                IMS_TRACE_D("pT140Config Create failure", 0, 0, 0);

                delete pT140Config;
                return IMS_NULL;
            }

            pCodecConfig = pT140Config;
        }
        break;
        case ImsCodec::TEXT_RED:
            // Need to add later
            break;
    }
    return pCodecConfig;
}
