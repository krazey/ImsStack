/**
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

#ifndef MEDIA_BASE_PROFILE_H_
#define MEDIA_BASE_PROFILE_H_

/**
 * The class is a base class of the Media (Audio/Video/Text) Profile.
 * Media Profile is used to keep the SDP negotiation information like
 * SDP offer, answer and the negotiated media information.
 */
class MediaBaseProfile
{
public:
    /**
     * Stands for "format parameter"
     * This class is a base class of each media fmtp.
     * Fmtp attributes are used within the SDP to carry parameters that provide
     * extra configuration details about a specific media codec used in the RTP stream.
     */
    class BaseFmtp
    {
    protected:
        BaseFmtp() {}

    public:
        virtual ~BaseFmtp() {}
    };

    /**
     * This class maps from a RTP payload type number (as used in an "m=" line)
     * to an encoding name denoting the payload format to be used.
     * It also provides information on the clock rate and encoding parameters.
     */
    class RtpMap
    {
    public:
        IMS_UINT32 nPayloadNum;    // Payload number
        AString strPayloadType;    // Payload type name
        IMS_UINT32 nSamplingRate;  // Sampling rate
        IMS_SINT32 nChannel;       // Number of channels

    public:
        RtpMap(IN const IMS_SINT32 channel = 0) :
                nPayloadNum(0),
                strPayloadType(AString::ConstNull()),
                nSamplingRate(0),
                nChannel(channel)
        {
        }

        RtpMap(IN const RtpMap& obj) :
                nPayloadNum(obj.nPayloadNum),
                strPayloadType(obj.strPayloadType),
                nSamplingRate(obj.nSamplingRate),
                nChannel(obj.nChannel)
        {
        }

        RtpMap& operator=(IN const RtpMap& obj)
        {
            if (this != &obj)
            {
                nPayloadNum = obj.nPayloadNum;
                strPayloadType = obj.strPayloadType;
                nSamplingRate = obj.nSamplingRate;
                nChannel = obj.nChannel;
            }
            return (*this);
        }
    };
};

#endif
