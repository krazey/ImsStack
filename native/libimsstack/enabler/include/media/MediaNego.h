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

#include "ImsSlot.h"
#include "MediaDef.h"
#include "audio/AudioDef.h"
#include "text/TextDef.h"
#include "video/VideoDef.h"

class AudioNego;
class IMedia;
class IMediaDescriptor;
class ISession;
class MediaEnvironment;
class SdpMedia;
class TextNego;
class VideoNego;

class MediaNego : public ImsSlot
{
public:
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
     * @brief Forms an SDP body for an outgoing offer, answer, or re-offer.
     *
     * This method populates the provided `ISession` object with the necessary SDP media
     * lines (`m=`) and attributes. It orchestrates the SDP generation for audio, video,
     * and text by delegating to the respective sub-negotiators (AudioNego, VideoNego,
     * TextNego). The behavior changes based on the current negotiation state (e.g.,
     * creating an initial offer, answering a received offer, or generating a re-offer).
     *
     * @param pSession [out] The `ISession` object to be populated with the generated SDP.
     * @param eMediaType A bitmask of `MEDIA_CONTENT_TYPE` specifying which media types to include.
     * @param eAudioDirection The desired `MEDIA_DIRECTION` for the audio stream.
     * @param eVideoDirection The desired `MEDIA_DIRECTION` for the video stream.
     * @param eTextDirection The desired `MEDIA_DIRECTION` for the text stream.
     * @param bEnforceReofferMode If `IMS_TRUE`, forces the SDP to be generated with the full set of
     *        local capabilities, which is typically used for re-offers.
     * @return IMS_BOOL Returns `IMS_TRUE` on success, or `IMS_FALSE` on failure (e.g., invalid
     *         state or arguments).
     */
    virtual IMS_BOOL FormSdp(OUT ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType,
            IN MEDIA_DIRECTION eAudioDirection, IN MEDIA_DIRECTION eVideoDirection,
            IN MEDIA_DIRECTION eTextDirection, IN IMS_BOOL bEnforceReofferMode);

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
     * @param pSession The ISession instance containing the SDP to negotiate.
     * @return SdpNegotiationResult A struct containing the negotiated media types, directions,
     * and any error that occurred.
     */
    virtual SdpNegotiationResult NegotiateSdp(IN ISession* pSession);

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
     * @brief Finalizes the session negotiation by cleaning up any stale negotiation models and
     * setting the final negotiation state based on the outcome.
     */
    virtual void FinalizeNegotiation();

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
     * @brief Get the negotiated codec bitrate (Kbps)
     *
     * @param eMediaType The media type to form, it can be audio/video/text defined in
     * MEDIA_CONTENT_TYPE
     * @return IMS_FLOAT Returns the bitrate of the negotiated codec (Kbps)
     */
    virtual IMS_FLOAT GetNegotiatedCodecBitrateKbps(IN MEDIA_CONTENT_TYPE eMediaType);

    /**
     * @brief Get the negotiated codec bandwidth (Khz)
     *
     * @param eMediaType The media type to form, it can be audio/video/text defined in
     * MEDIA_CONTENT_TYPE
     * @return IMS_FLOAT Returns the bandwidth of the negotiated codec (Khz)
     */
    virtual IMS_FLOAT GetNegotiatedCodecBandwidthKhz(IN MEDIA_CONTENT_TYPE eMediaType);

    /**
     * @brief Get the negotiated codec bitrate range
     *
     * @param eMediaType The media type to form, it can be audio/video/text defined in
     * MEDIA_CONTENT_TYPE
     * @param nBitrateStart The start of the bitrate range
     * @param nBitrateEnd The end of the bitrate range
     */
    virtual void GetNegotiatedCodecBitrateRange(IN MEDIA_CONTENT_TYPE eMediaType,
            OUT IMS_FLOAT& nBitrateStart, OUT IMS_FLOAT& nBitrateEnd);

    /**
     * @brief Get the negotiated codec bandwidth range
     *
     * @param eMediaType The media type to form, it can be audio/video/text defined in
     * MEDIA_CONTENT_TYPE
     * @param nBandwidthStart The start of the bandwidth range
     * @param nBandwidthEnd The end of the bandwidth range
     */
    virtual void GetNegotiatedCodecBandwidthRange(IN MEDIA_CONTENT_TYPE eMediaType,
            OUT IMS_FLOAT& nBandwidthStart, OUT IMS_FLOAT& nBandwidthEnd);

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
            IN MEDIA_DIRECTION eAudioDirection, IN MEDIA_DIRECTION eVideoDirection,
            IN MEDIA_DIRECTION eTextDirection, IN IMS_BOOL bEnforceReofferMode,
            OUT IMS_SINT32& nTotalAs, OUT IMediaDescriptor*& pDescriptorForAudio,
            OUT IMediaDescriptor*& pDescriptorForVideo, OUT IMediaDescriptor*& pDescriptorForText);
    void UpdateMediaDescriptor(IN ISession* pSession, IN IMediaDescriptor* pDescriptor,
            IN const SdpMedia* pSDPMedia, OUT IMediaDescriptor*& pNegotiatedAudioDescriptor,
            OUT IMediaDescriptor*& pNegotiatedVideoDescriptor,
            OUT IMediaDescriptor*& pNegotiatedTextDescriptor, OUT MEDIA_DIRECTION& eAudioDirection,
            OUT MEDIA_DIRECTION& eVideoDirection, OUT MEDIA_DIRECTION& eTextDirection);

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
