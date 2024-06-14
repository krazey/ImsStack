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

#ifndef AUDIO_NEGO_H_
#define AUDIO_NEGO_H_

#include "media/IMedia.h"
#include "BaseNego.h"
#include "ISession.h"
#include "MediaDef.h"
#include "audio/AudioDef.h"
#include "audio/AudioProfileUtil.h"
#include "config/AudioConfiguration.h"

/**
 * @brief The class to negotiate and form the SDP attribute belong to the m=audio line
 *
 */
class AudioNego : public BaseNego
{
public:
    explicit AudioNego(IMS_SINT32 nSlotId = IMS_SLOT_0);
    AudioNego(IN const AudioNego& objAudioNego);
    AudioNego& operator=(IN const AudioNego& obj);
    virtual ~AudioNego();

    /**
     * @brief Form the SDP with the current profile based on the state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to form the session level SDP
     * @param pDescriptor The SDP descriptor instance to form the media level SDP
     * @param eDir The media direction of the SDP
     * @param bEnforceReofferMode To indicate the SDP should be set using full codec capability
     * @return IMS_BOOL Returns IMS_TRUE when there is no error during forming SDP, IMS_FALSE when
     * it is failed to form
     */
    virtual IMS_BOOL FormSdp(IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir,
            IN IMS_BOOL bEnforceReofferMode);

    /**
     * @brief Check if audio codec from SDP is supported
     *
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the media level SDP
     * @return IMS_BOOL Returns IMS_TRUE when audio codec from SDP is supported and the remote audio
     * port is not 0, otherwise returns IMS_FALSE
     */
    virtual IMS_BOOL IsMediaCodecFromSdpSupported(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);

    /**
     * @brief Negotiate the SDP and make the negotiate profile based on the nego state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the media level SDP
     * @param eDir The media direction of the SDP
     */
    virtual void NegotiateSdp(IN const NEGO_STATE eNegoState,
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT IMS_SINT32& eDir);

    /**
     * @brief Get the negotiated remote ip address
     *
     * @return const IpAddress& The ip address
     */
    virtual const IpAddress& GetNegotiatedRemoteAddress();

    /**
     * @brief Get the negotiated remote port number
     *
     * @return IMS_UINT32 The port number
     */
    virtual IMS_SINT32 GetRemotePort();

    /**
     * @brief Get the negotiated local profile object
     */
    virtual AudioProfile* GetNegotiatedLocalProfile();

    /**
     * @brief Get the negotiated negotiated profile object
     */
    virtual AudioProfile* GetNegotiatedNegoProfile();

    /**
     * @brief Get the negotiated peer profile object
     */
    virtual AudioProfile* GetNegotiatedPeerProfile();

    /**
     * @brief Get the negotiated audio direction
     */
    virtual MEDIA_DIRECTION GetNegotiatedDirection(void);

    /**
     * @brief Get the negotiated audio codec
     */
    virtual AUDIO_CODEC GetNegotiatedCodec(void);

    /**
     * @brief Get the negotiated audio codec mode rate(bitrate)
     */
    virtual AUDIO_CODEC_BITRATE GetNegotiatedAudioCodecRate(void);

    /**
     * @brief Get if the telephony-event is negotiated
     */
    virtual IMS_BOOL HasNegotiatedDtmf(void);

    /**
     * @brief Get the port number from the negotiated profile
     */
    virtual IMS_SINT32 GetNegotiatedRtpPort(void);

    /**
     * @brief Get the negotiated audio bandwidth
     */
    virtual IMS_SINT32 GetMediaBandwidth(void);

protected:
    AudioConfiguration* ConfigCasting(IN MediaConfiguration* pConfig);
    AudioProfile* ProfileCasting(IN MediaBaseProfile* pProfile);
    AudioProfile* GetLocalProfile(IN OaModel* pOaModel) override;
    AudioProfile* GetPeerProfile(IN OaModel* pOaModel) override;
    AudioProfile* GetNegotiatedProfile(IN OaModel* pOaModel) override;

private:
    void Copy(IN const AudioNego* pAudioNego);
    IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MEDIA_DIRECTION eDir);
    IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir);
    IMS_BOOL FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir,
            IN IMS_BOOL bEnforceReofferMode);
    MEDIA_DIRECTION NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    MEDIA_DIRECTION NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    IMS_BOOL MakeSdpFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    IMS_BOOL MakeProfileFromSdp(IN ISessionDescriptor* pSessionDescriptor,
            IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    IMS_BOOL MakeNegotiatedProfile(IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT AudioProfile* pNegotiatedProfile);
    IMS_BOOL GetFmtpFromString(IN const AString& strFmtp, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL FindEvsInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bIsOfferReceived, OUT IMS_UINT32* pBandwidthNegoList,
            OUT IMS_UINT32* pBitrateNegoList, OUT IMS_UINT32* pModeSetNegoList);
    IMS_BOOL GetFmtpFromString(IN const AString& strFmtp, OUT AudioProfile::AmrFmtp* pFmtp);
    IMS_BOOL FindAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bIsOfferReceived, OUT IMS_UINT32* pnNegoModeSetList,
            OUT IMS_UINT32* pnNegoDefaultRtpModeSet);
    IMS_BOOL FindMatchedAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bIsOfferReceived, IN IMS_BOOL bReturnMode,
            OUT IMS_UINT32* pnNegoModeSetList, OUT IMS_UINT32* pnNegoDefaultRtpModeSet);
    IMS_BOOL FindPcmInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload);
    IMS_SINT32 CompareModeSet(IN AudioProfile::AmrFmtp* pSrcFmtp,
            IN AudioProfile::AmrFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
            IN IMS_BOOL bReturnMode, OUT IMS_UINT32* nNegoModeSet,
            OUT IMS_UINT32* nNegoDefaultRtpModeSet);
    IMS_BOOL CompareEvsBwBrMode(IN AudioProfile::EvsFmtp* pSrcFmtp,
            IN AudioProfile::EvsFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
            OUT IMS_UINT32* nNegoBwList, OUT IMS_UINT32* nNegoBrList,
            OUT IMS_UINT32* nNegoModeList);
    IMS_BOOL CompareEvsBwBrModeLegacy(IN AudioProfile::EvsFmtp* pSrcFmtp,
            IN AudioProfile::EvsFmtp* pDestFmtp, OUT IMS_UINT32* nNegoBwList,
            OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList);
    IMS_SINT32 FindPayloadIndexFromProfile(IN const AString& strCodecName,
            IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL isOfferReceivedCase);
    IMS_SINT32 FindMatchedPayloadIndexFromProfile(IN const AString& strCodecName,
            IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL isOfferReceivedCase, IN IMS_BOOL bReturnMode);
    MEDIA_DIRECTION UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDirection,
            IN MEDIA_DIRECTION eLocalDirection, IN IMS_BOOL bIsMtCase);
    IMS_BOOL MakeCapaNegoProfileFromSdp(
            IN IMediaDescriptor* pDescriptor, OUT AudioProfile::CapaNego* pObjCapaNego);
    OaModel* GetNegotiatedOaModel(IMS_BOOL bCheckConfirmed = IMS_FALSE);

    void SetSdpSessionIpAddress(
            OUT ISessionDescriptor* pSessionDescriptor, IN AudioProfile* pProfile);
    void SetSdpMediaDescription(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void SetSdpMediaBandwidth(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
};

#endif
