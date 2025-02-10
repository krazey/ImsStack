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

#ifndef TEXT_SESSION_H_
#define TEXT_SESSION_H_

#include <MediaQualityThreshold.h>
#include "BaseSession.h"
#include "text/TextDef.h"
#include "text/TextProfile.h"

class TextConfiguration;

class TextSession : public BaseSession
{
public:
    enum
    {
        /** The default state when the instance is just created*/
        STATE_NONE = 0,
        /** The state that the openSession is done */
        STATE_IDLE,
        /** The state that the rtp/rtcp stream is running */
        STATE_LIVE,
        /** The state that the rtp stream is stopped but the rtcp stream is running */
        STATE_PAUSED,
    };

    explicit TextSession(IN IMS_SINT32 nSlotId = 0);
    virtual ~TextSession();

    /**
     * @brief Set TextConfig for libpixelimsmedia from src/dest/negotiated profile
     * @param pLocalProfile : local profile of the SDP negotiation
     * @param pPeerProfile : peer profile of the SDP negotiation
     * @param pNegoProfile : negotiated profile of the SDP negotiation
     * return IMS_BOOL : false for error, true for successful
     */
    IMS_BOOL UpdateRtpConfig(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            IN TextProfile* pNegoProfile);

    /**
     * @brief Update AccessNetwork information in the RtpConfig
     *
     * @param nAccessNetwork : AccessNetwork information
     */
    void UpdateAccessNetwork(IMS_UINT32 nAccessNetwork);

    /**
     * @brief Update MediaQualityThreshold parameters and send it to the java
     *
     * @param bActiveSession Set IMS_TRUE if this session is active
     * @param bEnableRtcp Set IMS_TRUE to enable monitoring Rtcp inacitivity, IMS_FALSE to disable
     * rtcp monitoring
     * @return IMS_BOOL Returns IMS_TRUE when the sending MediaQualityThreshold is done
     * successfully, IMS_FALSE when it is failed with invalid arguments
     */
    IMS_BOOL UpdateMediaQualityThreshold(IN IMS_BOOL bActiveSession, IN IMS_BOOL bEnableRtcp);

    /**
     * @brief Handles the message from the telecom
     *
     * @param nMsg The message type
     * @param pParam The message parameter
     * @return IMS_BOOL Returns IMS_TRUE when the parameter is valid, IMS_FALSE when it is invalid
     */
    IMS_BOOL OnTextMessages(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);

    /*
     * request OPEN_SESSION with updated TextConfig
     */
    IMS_BOOL Open();

    /*
     * request MODIFY_SESSION with updated TextConfig
     */
    IMS_BOOL Modify();

    /*
     * request CLOSE_SESSION with updated TextConfig
     */
    IMS_BOOL Close();

    /*
     * request SET_MEDIA_QUALITY with Text Media qualityThreshold
     */
    IMS_BOOL SetMediaQuality();

    /**
     * @brief Get the local port number
     *
     * @return IMS_SINT32 The port number
     */
    IMS_SINT32 GetLocalPort();

    /**
     * @brief Get the remote port number
     *
     * @return IMS_SINT32 The port number
     */
    IMS_SINT32 GetRemotePort();

private:
    TextConfiguration* GetConfiguration();

    MediaQualityThreshold m_objMediaQualityThreshold;
};

#endif
