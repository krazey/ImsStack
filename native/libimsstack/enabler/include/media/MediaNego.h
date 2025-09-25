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

#ifndef MEDIA_NEGO_H_
#define MEDIA_NEGO_H_

#include <memory>

#include "ImsSlot.h"
#include "MediaDef.h"
#include "audio/AudioDef.h"
#include "text/TextDef.h"
#include "video/VideoDef.h"
#include "media/IMedia.h"

class AudioNego;
class TextNego;
class VideoNego;
class MediaEnvironment;
class ISession;
class IMediaDescriptor;

class MediaNego : public ImsSlot
{
public:
    enum MediaNegoResult
    {
        /** No error */
        NO_ERROR = 0,
        /** error when the descriptor is invalid*/
        ERROR_INVALID_DESCRIPTOR,
        /** error when there is no negotiated codec */
        ERROR_NO_CODEC_MATCHED,
        /** error when the ip version is not matched between offer and answer */
        ERROR_IP_MISMATCH,
        /** error when there is no audio m line in SDP*/
        ERROR_NO_AUDIO,
        /** error when there is no video m line in SDP*/
        ERROR_NO_VIDEO,
        /** error when there is no text m line in SDP*/
        ERROR_NO_TEXT,
    };

    explicit MediaNego(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    ~MediaNego() override;

    /**
     * @brief Creates MediaNego instance with given parameter
     *
     * @param pMediaEnvironment The instance of network connection information
     * @return IMS_BOOL Return IMS_TRUE when it successes, IMS_FALSE when the arguments is
     * invalid
     */
    virtual IMS_BOOL CreateProfile(IN std::shared_ptr<MediaEnvironment> pMediaEnvironment);

    /**
     * @brief Creates MediaNego object copied from the existing MediaNego instance
     *
     * @param pMediaNego the instance of existing MediaNego
     * @return IMS_BOOL Return IMS_TRUE when the forking success, IMS_FALSE when the arguments is
     * invalid
     */
    virtual IMS_BOOL Forking(IN MediaNego* pMediaNego);

    /**
     * @brief Forms SDP with the profile created and send the offer or answer with the given
     * arguments
     *
     * @param pSession ISession instance to get the SDP descriptor
     * @param eMediaType The media type to form, it can be audio/video/text defined in
     * MEDIA_CONTENT_TYPE
     * @param nAudioDirection The audio direction to set in the SDP
     * @param nVideoDirection The video direction to set in the SDP
     * @param nTextDirection The text direction to set in the SDP
     * @param bEnforceReofferMode To indicate the SDP should be set using full codec capability
     * @return IMS_BOOL Returns IMS_TRUE when the form SDP is done successfully and IMS_FALSE when
     * failed with invalid arguments
     */
    virtual IMS_BOOL FormSdp(OUT ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType,
            IN IMS_SINT32 nAudioDirection, IN IMS_SINT32 nVideoDirection,
            IN IMS_SINT32 nTextDirection, IN IMS_BOOL bEnforceReofferMode);

    /**
     * @brief Get Supported Media Types from SDP
     *
     * @param pSession ISession instance to get the SDP descriptor
     * @return MEDIA_CONTENT_TYPE The Supported media types
     */
    virtual MEDIA_CONTENT_TYPE GetSupportedMediaTypesFromSdp(IN ISession* pSession);

    /**
     * @brief Create negotiate profile from the SDP received from the network by the given arguments
     *
     * @param pSession ISession instance to get the SDP descriptor
     * @param nAudioDirection The audio direction negotiated
     * @param nVideoDirection The video direction negotiated
     * @param nTextDirection The text direction negotiated
     * @param errorReason The error reason when the negotiation is failed
     * @return IMS_BOOL Returns IMS_TRUE when the negotiation succeed and IMS_FALSE when failed.
     * MediaNegoResult will be set when negotiation failed
     */
    virtual IMS_BOOL NegotiateSdp(IN ISession* pSession, OUT IMS_SINT32& nAudioDirection,
            OUT IMS_SINT32& nVideoDirection, OUT IMS_SINT32& nTextDirection,
            OUT MediaNegoResult& errorReason);

    /**
     * @brief Finalizes the media session state by removing deleted media lines.
     *
     * This method iterates through the media lines (m-lines) in the provided `ISession`
     * and removes any that have been marked with `IMedia::STATE_DELETED`. This is a
     * crucial cleanup step after a session modification (e.g., a re-INVITE) is
     * confirmed, ensuring that the session object accurately reflects the active media.
     *
     * @param pSession The ISession instance whose media lines need to be finalized.
     */
    virtual void FinalizeSdp(IN ISession* pSession);

    /**
     * @brief Confirms the session negotiation, cleans up unconfirmed OA models,
     * and sets the final negotiation state.
     */
    virtual void ConfirmSession();

    /**
     * @brief Set the negotiation state
     *
     * @param eNegoState The state defined in NEGO_STATE
     */
    virtual void SetNegoState(NEGO_STATE eNegoState) { m_eNegoState = eNegoState; }

    /**
     * @brief Get the negotiation state
     *
     * @return NEGO_STATE
     */
    virtual NEGO_STATE GetNegoState() { return m_eNegoState; }

    /**
     * @brief Set the AudioNego instance
     */
    void SetAudioNego(std::shared_ptr<AudioNego> pAudioNego) { m_pAudioNego = pAudioNego; }

    /**
     * @brief Get the AudioNego instance
     */
    virtual std::shared_ptr<AudioNego> GetAudioNego() { return m_pAudioNego; }

    /**
     * @brief Set the VideoNego instance
     */
    void SetVideoNego(std::shared_ptr<VideoNego> pVideoNego) { m_pVideoNego = pVideoNego; }

    /**
     * @brief Get the VideoNego instance
     */
    virtual std::shared_ptr<VideoNego> GetVideoNego() { return m_pVideoNego; }

    /**
     * @brief Set the TextNego instance
     */
    void SetTextNego(std::shared_ptr<TextNego> pTextNego) { m_pTextNego = pTextNego; }

    /**
     * @brief Get the TextNego instance
     */
    virtual std::shared_ptr<TextNego> GetTextNego() { return m_pTextNego; }

    /**
     * @brief Get the media session type
     *
     * @return MEDIA_CONTENT_TYPE
     */
    virtual MEDIA_CONTENT_TYPE GetSessionType() { return m_eSessionType; }

    /**
     * @brief Get the negotiated audio direction
     *
     * @return MEDIA_DIRECTION
     */
    virtual MEDIA_DIRECTION GetNegotiatedAudioDirection(void);

    /**
     * @brief Get the negotiated video direction
     *
     * @return MEDIA_DIRECTION
     */
    virtual MEDIA_DIRECTION GetNegotiatedVideoDirection(void);

    /**
     * @brief Get the negotiated text direction
     *
     * @return MEDIA_DIRECTION
     */
    virtual MEDIA_DIRECTION GetNegotiatedTextDirection(void);

    /**
     * @brief Get the negotiated audio codec
     *
     * @return AUDIO_CODEC
     */
    virtual AUDIO_CODEC GetNegotiatedAudioQuality(void);

    /**
     * @brief Get the negotiated video resolution
     *
     * @return VIDEO_RESOLUTION
     */
    virtual VIDEO_RESOLUTION GetNegotiatedVideoQuality(void);

    /**
     * @brief Get the negotiated text resolution
     *
     * @return TEXT_CODEC
     */
    virtual TEXT_CODEC GetNegotiatedTextQuality(void);

    /**
     * @brief Get the media descriptor instance
     *
     * @param pIMedia
     * @return IMediaDescriptor*
     */
    virtual IMediaDescriptor* GetMediaDescriptor(IN IMedia* pIMedia);

    /**
     * @brief Check this instance is crated from forking
     *
     * @return IMS_BOOL Returns IMS_TRUE when this instance is created from forking
     */
    virtual IMS_BOOL IsForking();

    /**
     * @brief Check if the SDP negotiation is done in preview mode.
     */
    virtual IMS_BOOL IsPreviewMode();

    /**
     * @brief Set the mode is preview.
     */
    virtual void SetPreviewMode(IMS_BOOL bIsPreview);

private:
    ImsList<IMedia*> CreateIMediaListFromSession(
            IN ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType);
    void UpdateMediaTypeToNegotiate(IN ISession* pSession);
    void SetMediaDescriptorAsNotSupported(IN IMediaDescriptor* pDescriptor, IN SdpMedia* pSDPMedia);
    void UpdateNegoState(IMS_BOOL bFormSdp);
    void UpdateSessionLevelBandwidth(IN ISession* pSession, IMS_UINT32 nTotalAs);
    IMS_BOOL ProcessMediaLine(OUT ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType,
            IN IMS_SINT32 nAudioDirection, IN IMS_SINT32 nVideoDirection,
            IN IMS_SINT32 nTextDirection, IN IMS_BOOL bEnforceReofferMode, OUT IMS_SINT32& nTotalAs,
            OUT IMediaDescriptor*& pDescriptorForAudio, OUT IMediaDescriptor*& pDescriptorForVideo,
            OUT IMediaDescriptor*& pDescriptorForText);
    void UpdateMediaDescriptor(IN ISession* pSession, IN IMediaDescriptor* pDescriptor,
            IN const SdpMedia* pSDPMedia, OUT IMediaDescriptor*& pNegotiatedAudioDescriptor,
            OUT IMediaDescriptor*& pNegotiatedVideoDescriptor,
            OUT IMediaDescriptor*& pNegotiatedTextDescriptor, OUT IMS_SINT32& nAudioDirection,
            OUT IMS_SINT32& nVideoDirection, OUT IMS_SINT32& nTextDirection);

    NEGO_STATE m_eNegoState;
    std::shared_ptr<AudioNego> m_pAudioNego;
    std::shared_ptr<VideoNego> m_pVideoNego;
    std::shared_ptr<TextNego> m_pTextNego;
    std::shared_ptr<MediaEnvironment> m_pMediaEnvironment;
    MEDIA_CONTENT_TYPE m_eSessionType;
    IMS_BOOL m_bForking;
    IMS_BOOL m_bPreviewMode;
};

#endif
