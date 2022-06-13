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

#ifndef _IMS_AUDIO_NEGO_H_
#define _IMS_AUDIO_NEGO_H_

// == INCLUDES =========================================================
#include "ImsSlot.h"

#include "media/IMedia.h"
#include "ISession.h"

#include "MediaDef.h"
#include "audio/AudioDef.h"
#include "MediaEnvironment.h"

#include "audio/AudioProfile.h"
#include "config/AudioConfiguration.h"
#include "audio/AudioProfileConfigurer.h"

class AudioNego : public ImsSlot
{
    // == INNER CLASS ================================================================
public:
    class OaModel
    {
    public:
        AudioProfile* pSrcProfile;
        AudioProfile* pDestProfile;
        AudioProfile* pNegotiatedProfile;
        IMS_SINTP nSessionDescriptorKey;
        IMS_BOOL bConfirmedSession;

    public:
        OaModel() :
                pSrcProfile(IMS_NULL),
                pDestProfile(IMS_NULL),
                pNegotiatedProfile(IMS_NULL),
                nSessionDescriptorKey(0),
                bConfirmedSession(IMS_FALSE){};

        ~OaModel()
        {
            if (pSrcProfile != IMS_NULL)
            {
                delete pSrcProfile;
            }
            if (pDestProfile != IMS_NULL)
            {
                delete pDestProfile;
            }
            if (pNegotiatedProfile != IMS_NULL)
            {
                delete pNegotiatedProfile;
            }
        };

    private:
        OaModel(IN const OaModel& obj);
        OaModel& operator=(IN const OaModel& obj);

    public:
        IMS_BOOL IsAllProfileExist()
        {
            if (pSrcProfile != IMS_NULL && pDestProfile != IMS_NULL &&
                    pNegotiatedProfile != IMS_NULL)
            {
                return IMS_TRUE;
            }
            else
            {
                return IMS_FALSE;
            }
        };
    };

    // == Constructor, Destructor, Operator Overloading ========================================
protected:
    AudioNego(IMS_SINT32 nSlotId = IMS_SLOT_0);

public:
    static AudioNego* Create(IN IMS_SINT32 nSlotId = IMS_SLOT_0,
            IN MEDIA_SERVICE_TYPE eServiceType = MEDIA_SERVICE_DEFAULT);
    virtual ~AudioNego();

private:
    AudioNego(IN const AudioNego& obj);
    AudioNego& operator=(IN const AudioNego& obj);

    // == PUBLIC METHOD ==============================================================
public:
    virtual void CreateProfiles(IN MediaEnvironment* pEnvironment);
    virtual void DestroyProfiles();
    virtual void SetMediaEnvironment(IN MediaEnvironment* pEnvironment);
    virtual void SetSessionType(IN MEDIA_CONTENT_TYPE eSessionType);
    AudioConfiguration* GetConfig();
    void Copy(IN AudioNego* pAudioNego);

    // -- Negotiation APIs -------------------------------------------------------------------------
    virtual IMS_BOOL FormSDP(IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType,
            IN MEDIA_DIRECTION eDir);
    virtual IMS_BOOL NegotiateSDP(IN NEGO_STATE eNegoState, IN IMS_BOOL bForking,
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT MEDIA_DIRECTION* eDir);
    virtual void FinalizeSDP(IN IMS_SINTP nSessionDescriptorKey, NEGO_STATE eNegoState);

    // -- Additional function APIs
    // -------------------------------------------------------------------------
    IMS_BOOL SetPort(IN IMS_UINT32 nPort);
    IPAddress GetLocalAddr() { return m_objBaseProfile.objIpAddr; };
    IMS_UINT32 GetLocalPort() { return m_objBaseProfile.nDataPort; };
    IPAddress GetNegotiatedRemoteAddr();
    IMS_UINT32 GetNegotiatedRemotePort();
    // -- Condition checking APIs
    // -------------------------------------------------------------------------
    OaModel* GetNegotiatedOaModel(IMS_BOOL bCheckConfirmed = IMS_FALSE);
    IMS_BOOL GetNegotiatedProfileSet(OUT AudioProfile*& pSrcProfile,
            OUT AudioProfile*& pDestProfile, OUT AudioProfile*& pNegotiatedProfile);
    AudioProfile* GetNegotiatedDestProfile();

    IMS_UINT32 GetLatestRemotePort();
    MEDIA_DIRECTION GetNegotiatedDirection(void);
    AUDIO_CODEC GetNegotiatedCodec(void);
    AUDIO_CODEC_BITRATE GetNegotiatedAudioCodecRate(void);
    IMS_BOOL HasNegotiatedDtmf(void);
    IMS_SINT32 GetNegotiatedRtpPort(void);
    IMS_SINT32 GetMediaBandwidth(void);

    // == PROTECTED METHOD ==========================================================
protected:
    virtual IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType,
            IN MEDIA_DIRECTION eDir);
    virtual IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType,
            IN MEDIA_DIRECTION eDir);
    virtual IMS_BOOL FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType,
            IN MEDIA_DIRECTION eDir);
    virtual MEDIA_DIRECTION NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    virtual MEDIA_DIRECTION NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    virtual MEDIA_DIRECTION NegotiateReanswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    IMS_BOOL MakeSdpFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    IMS_BOOL MakeProfileFromSdp(IN ISessionDescriptor* pSessionDescriptor,
            IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    IMS_BOOL MakeNegotiatedProfile(IN AudioProfile* pSrcProfile, IN AudioProfile* pDestProfile,
            IN IMS_BOOL bIsOfferReceived, OUT AudioProfile* pNegotiatedProfile);
    IMS_BOOL GetFmtpFromString(IN AString strFmtp, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL FindEvsInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bIsOfferReceived, OUT IMS_UINT32* pBandwidthNegoList,
            OUT IMS_UINT32* pBitrateNegoList, OUT IMS_UINT32* pModeSetNegoList);
    IMS_BOOL GetFmtpFromString(IN AString strFmtp, OUT AudioProfile::AmrFmtp* pFmtp);
    IMS_BOOL FindAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bIsOfferReceived, OUT IMS_UINT32* pnNegoModeSetList);
    IMS_BOOL FindPcmInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload);
    IMS_SINT32 CompareModeSet(IN AudioProfile::AmrFmtp* pSrcFmtp,
            IN AudioProfile::AmrFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
            OUT IMS_UINT32* nNegoModeSet);
    IMS_BOOL CompareEvsBwBrMode(IN AudioProfile::EvsFmtp* pSrcFmtp,
            IN AudioProfile::EvsFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
            OUT IMS_UINT32* nNegoBwList, OUT IMS_UINT32* nNegoBrList,
            OUT IMS_UINT32* nNegoModeList);
    IMS_BOOL CompareEvsBwBrModeLegacy(IN AudioProfile::EvsFmtp* pSrcFmtp,
            IN AudioProfile::EvsFmtp* pDestFmtp, OUT IMS_UINT32* nNegoBwList,
            OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList);
    IMS_BOOL FindTelephoneEventInProfile(
            IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload);
    IMS_SINT32 FindPayloadIndexFromProfile(IN AString strCodecName, IN AudioProfile* pProfile,
            IN AudioProfile::Payload* pPayload, IN IMS_BOOL isOfferReceivedCase);
    void RearrangeModeSetByAs(
            OUT AudioProfile::Payload* pPayload, IMS_BOOL bIpV6, IN IMS_SINT32 nAs);
    MEDIA_DIRECTION UpdateDirectionToMine(
            IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase);
    AString MakeCryptoAttributeFromSrtpProfile(IN AudioProfile* pProfile);
    IMS_BOOL MakeSrtpProfileFromCapaNego(IN_OUT AudioProfile* pProfile);
    IMS_BOOL MakeSrtpProfileFromCryptoAttr(OUT AudioProfile* pProfile, IN AString CryptoAttr);
    IMS_BOOL MakeCapaNegoProfileFromSdp(
            IN IMediaDescriptor* pDescriptor, OUT AudioProfile::CapaNego* pObjCapaNego);
    IMS_BOOL MakeNegotiatedCapaNegoProfile(IN AudioProfile::CapaNego* pSrcCapaNego,
            IN AudioProfile::CapaNego* pDestCapaNego,
            OUT AudioProfile::CapaNego* pNegotiatedCapaNego);

protected:
    IMSList<OaModel*> m_lstOaModel;
    AudioProfile m_objBaseProfile;
    MediaEnvironment* m_pMediaEnvironment;

private:
    MEDIA_CONTENT_TYPE m_eSessionType;
    static const AString EVS_BR[12];
    static const AString EVS_BW[4];
    static const AString EVS_BW_LIST[9];

public:
    static const AString AUDIO_CODEC_BANDWIDTH_STRING[4];
    static const AString AUDIO_CODEC_BITRATE_STRING[3][9];
};
#endif /* _IMS_AUDIO_NEGO_H_ */
