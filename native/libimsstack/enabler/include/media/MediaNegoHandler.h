/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MEDIA_NEGO_HANDLER_H_
#define MEDIA_NEGO_HANDLER_H_

#include "ISession.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MediaNego.h"

class IMediaNegoFactory;
class MediaEnvironment;

/**
 * @brief Handles the creation, management, and SDP negotiation logic associated with MediaNego
 * instances for a MediaSession.
 */
class MediaNegoHandler
{
public:
    /**
     * @brief Constructor
     * @param nSlotId The UICC slot id.
     * @param pEnvironment Shared pointer to the media environment containing network
     *                     and service information.
     * @param pFactory The factory to inject the dependency
     */
    MediaNegoHandler(IMS_UINT32 nSlotId, std::shared_ptr<MediaEnvironment> pEnvironment,
            std::shared_ptr<IMediaNegoFactory> pFactory);

    /**
     * @brief Destructor. Cleans up all managed MediaNego instances.
     */
    virtual ~MediaNegoHandler();

    // --- MediaNego Instance Management ---

    /**
     * @brief Creates a new MediaNego instance or forks an existing one.
     *
     * If nExistingNegoId is 0, a new MediaNego is created and its profile initialized.
     * If nExistingNegoId is non-zero, a new MediaNego is created by forking the
     * state from the MediaNego identified by nExistingNegoId.
     *
     * @param nExistingNegoId The ID (pointer) of an existing MediaNego to fork from,
     *                        or 0 to create a new one.
     * @return Return a new MediaNego id.
     */
    virtual IMS_UINTP CreateMediaNego(IMS_UINTP nExistingNegoId = 0);

    /**
     * @brief Finds a MediaNego instance by its ID.
     * @param nNegoId The ID (pointer) of the MediaNego instance to find.
     * @return std::shared_ptr<MediaNego> Pointer to the found MediaNego instance, or nullptr if not
     * found. The caller does NOT own this pointer.
     */
    virtual std::shared_ptr<MediaNego> FindMediaNego(IMS_UINTP nNegoId);

    /**
     * @brief Deletes a specific MediaNego instance identified by its ID.
     * @param nNegoId The ID (pointer) of the MediaNego instance to delete.
     * @return IMS_BOOL IMS_TRUE if deletion was successful or the ID was not found,
     *                  IMS_FALSE if nNegoId is invalid (e.g., UNDEFINED_NEGO_ID).
     */
    virtual IMS_BOOL DeleteMediaNego(IMS_UINTP nNegoId);

    /**
     * @brief Deletes all managed MediaNego instances.
     */
    virtual void ClearAllMediaNego();

    // --- SDP Negotiation Operations (Delegated to MediaNego) ---

    /**
     * @brief Delegates SDP forming (offer/answer/re-offer) to the specified MediaNego instance.
     * @see MediaNego::FormSdp
     * @param nNegoId The ID of the MediaNego instance.
     * @param pSession ISession instance for SDP manipulation.
     * @param eType The media types involved.
     * @param nAudioDirection Desired audio direction.
     * @param nVideoDirection Desired video direction.
     * @param nTextDirection Desired text direction.
     * @param bEnforceReofferMode Flag for re-offer mode.
     * @return IMS_BOOL IMS_TRUE on success, IMS_FALSE on failure (e.g., NegoId not found).
     */
    virtual IMS_BOOL FormSdp(IMS_UINTP nNegoId, OUT ISession* pSession, MEDIA_CONTENT_TYPE eType,
            IMS_SINT32 nAudioDirection, IMS_SINT32 nVideoDirection, IMS_SINT32 nTextDirection = -1,
            IMS_BOOL bEnforceReofferMode = IMS_FALSE);

    /**
     * @brief Delegates getting supported media types from SDP to the specified MediaNego instance.
     * @see MediaNego::GetSupportedMediaTypesFromSdp
     * @param nNegoId The ID of the MediaNego instance.
     * @param pSession ISession instance containing the remote SDP.
     * @return MEDIA_CONTENT_TYPE The supported media types, or MEDIA_TYPE_INVALID on failure.
     */
    virtual MEDIA_CONTENT_TYPE GetSupportedMediaTypesFromSdp(
            IMS_UINTP nNegoId, IN ISession* pSession);

    /**
     * @brief Delegates SDP negotiation (offer/answer) to the specified MediaNego instance.
     * @see MediaNego::NegotiateSdp
     * @param nNegoId The ID of the MediaNego instance.
     * @param pSession ISession instance containing the SDP to negotiate.
     * @param nAudioDirection Output parameter for negotiated audio direction.
     * @param nVideoDirection Output parameter for negotiated video direction.
     * @param nTextDirection Output parameter for negotiated text direction.
     * @param errorReason Output parameter for the reason if negotiation fails.
     * @return IMS_BOOL IMS_TRUE on successful negotiation, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL NegotiateSdp(IMS_UINTP nNegoId, IN ISession* pSession,
            OUT IMS_SINT32* nAudioDirection, OUT IMS_SINT32* nVideoDirection,
            OUT IMS_SINT32* nTextDirection, OUT MediaNego::MediaNegoResult& errorReason);
    /**
     * @brief Delegates the cleanup of deleted media descriptors to the specified MediaNego
     * instance.
     * @see MediaNego::FinalizeSdp
     * @param nNegoId The ID of the MediaNego instance.
     * @param pSession ISession instance associated with the SDP.
     */
    virtual void FinalizeSdp(IMS_UINTP nNegoId, IN ISession* pSession);

    /**
     * @brief Confirms the session negotiation and cleans up any stale negotiation models.
     * This should be called when a session is fully established (e.g., after 2xx/ACK).
     * @param nNegoId The ID of the MediaNego instance.
     */
    virtual void ConfirmSession(IMS_UINTP nNegoId);

    // --- Getters for Negotiated Information (Delegated to MediaNego) ---

    /**
     * @brief Gets the negotiation state of the specified MediaNego instance.
     * @see MediaNego::GetNegoState
     * @param nNegoId The ID of the MediaNego instance.
     * @return NEGO_STATE The current negotiation state, or STATE_NOTUSED if not found.
     */
    virtual NEGO_STATE GetNegoState(IMS_UINTP nNegoId);

    /**
     * @brief Determines the successfully negotiated media types for a MediaNego instance.
     * @param nNegoId The ID of the MediaNego instance.
     * @return MEDIA_CONTENT_TYPE A bitmask of the negotiated media types,
     *                            or MEDIA_TYPE_INVALID if not found or none negotiated.
     */
    virtual MEDIA_CONTENT_TYPE GetNegotiatedMediaType(IMS_UINTP nNegoId);

    /**
     * @brief Gets the negotiated quality (codec/resolution) for a specific media type.
     * @see MediaNego::GetNegotiatedAudioQuality, MediaNego::GetNegotiatedVideoQuality,
     * MediaNego::GetNegotiatedTextQuality
     * @param nNegoId The ID of the MediaNego instance.
     * @param eType The media type (AUDIO, VIDEO, or TEXT).
     * @return IMS_SINT32 The negotiated quality enum value, or a default/error value if not found
     * or not applicable.
     */
    virtual IMS_SINT32 GetNegotiatedQuality(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType);

    /**
     * @brief Gets the negotiated codec bitrate for a specific media type.
     *        Note: The implementation logic might need review based on actual requirements.
     * @param nNegoId The ID of the MediaNego instance.
     * @param eType The media type (AUDIO, VIDEO, or TEXT).
     * @return IMS_SINT32 The negotiated bitrate, or a default/error value.
     */
    virtual IMS_SINT32 GetNegotiatedCodecBitrate(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType);

    /**
     * @brief Gets the remote port for a specific media type.
     * @see AudioNego::GetRemotePort, VideoNego::GetRemotePort, TextNego::GetRemotePort
     * @param nNegoId The ID of the MediaNego instance.
     * @param eType The media type (AUDIO, VIDEO, or TEXT).
     * @return IMS_SINT32 The remote port number, or MEDIA_PORT_INVALID if not found or not
     * applicable.
     */
    virtual IMS_SINT32 GetRemotePort(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType);

    /**
     * @brief Gets the negotiated direction for a specific media type.
     * @see MediaNego::GetNegotiatedAudioDirection, MediaNego::GetNegotiatedVideoDirection,
     * MediaNego::GetNegotiatedTextDirection
     * @param nNegoId The ID of the MediaNego instance.
     * @param eType The media type (AUDIO, VIDEO, or TEXT).
     * @return MEDIA_DIRECTION The negotiated direction, or MEDIA_DIRECTION_INVALID if not found or
     * not applicable.
     */
    virtual MEDIA_DIRECTION GetNegotiatedDirection(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType);

    /**
     * @brief Gets the negotiated remote IP address for a specific media type.
     * @see AudioNego::GetNegotiatedRemoteAddress, VideoNego::GetNegotiatedRemoteAddress,
     * TextNego::GetNegotiatedRemoteAddress
     * @param nNegoId The ID of the MediaNego instance.
     * @param eType The media type (AUDIO, VIDEO, or TEXT).
     * @return IpAddress The negotiated remote IP address, or an invalid/empty IpAddress if not
     * found or not applicable.
     */
    virtual const IpAddress& GetNegotiatedRemoteAddress(
            IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType);

    // --- Options ---

    /**
     * @brief Sets the local RTP port for a specific media type within a MediaNego instance.
     * @see AudioNego::SetLocalPort, VideoNego::SetLocalPort, TextNego::SetLocalPort
     * @param nNegoId The ID of the MediaNego instance.
     * @param eType The media type (AUDIO, VIDEO, or TEXT).
     * @param nPort The port number to set.
     * @return IMS_BOOL IMS_TRUE on success, IMS_FALSE on failure (e.g., NegoId not found, invalid
     * media type).
     */
    virtual IMS_BOOL SetRtpPort(IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType, IMS_UINT32 nPort);

private:
    MediaNegoHandler(const MediaNegoHandler&) = delete;
    MediaNegoHandler& operator=(const MediaNegoHandler&) = delete;

    /**
     * @brief Generate the new identification of the MediaNego
     */
    IMS_UINTP GenerateNewNegoId();

    IMS_UINT32 m_nSlotId;
    std::shared_ptr<MediaEnvironment> m_pEnvironment;  // Needed for MediaNego::CreateProfile
    ImsMap<IMS_UINTP, std::shared_ptr<MediaNego>> m_objMapMediaNego;
    std::shared_ptr<IMediaNegoFactory> m_pMediaNegoFactory;
};

#endif  // MEDIA_NEGO_HANDLER_H_
