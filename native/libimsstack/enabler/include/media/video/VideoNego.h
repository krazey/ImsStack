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

#ifndef _IMS_VIDEO_NEGO_H_
#define _IMS_VIDEO_NEGO_H_

// == INCLUDES =========================================================
#include "ImsSlot.h"
#include "media/IMedia.h"
#include "ISession.h"

#include "MediaDef.h"
#include "video/VideoDef.h"
#include "MediaEnvironment.h"

#include "config/VideoConfiguration.h"
#include "video/VideoProfile.h"
#include "video/VideoProfileConfigurer.h"

class MediaSession;

class VideoNego : ImsSlot
{
    // == INNER CLASS ================================================================
public:
    class OaModel
    {
    public:
        VideoProfile* pSrcProfile;
        VideoProfile* pDestProfile;
        VideoProfile* pNegotiatedProfile;
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
public:
    static VideoNego* Create(IN IMS_SINT32 nSlotID = IMS_SLOT_0,
            IN MEDIA_SERVICE_TYPE eServiceType = MEDIA_SERVICE_DEFAULT);
    virtual ~VideoNego();
    void Copy(IN VideoNego* pVideoNego);

private:
    VideoNego(IN IMS_SINT32 nSlotID = IMS_SLOT_0);
    VideoNego(IN const VideoNego& obj);
    VideoNego& operator=(IN const VideoNego& obj);

    // == PUBLIC METHOD ==============================================================
public:
    virtual void CreateProfiles(IN MediaEnvironment* pEnvironment);
    virtual void DestroyProfiles();
    virtual void SetMediaEnvironment(IN MediaEnvironment* pEnvironment);
    virtual void SetSessionType(IN MEDIA_CONTENT_TYPE eSessionType);
    VideoConfiguration* GetConfig();
    // -- Negotiation APIs -------------------------------------------------------------------------
    virtual IMS_BOOL FormSDP(IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType,
            IN MEDIA_DIRECTION eDir);
    virtual IMS_BOOL NegotiateSDP(NEGO_STATE eNegoState, IN IMS_BOOL bForking,
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
    IMS_BOOL GetNegotiatedCvoResult();
    // -- Condition checking APIs
    // -------------------------------------------------------------------------
    OaModel* GetNegotiatedOaModel(IN IMS_BOOL bCheckConfirmed = IMS_FALSE);
    IMS_BOOL GetNegotiatedProfileSet(OUT VideoProfile*& pSrcProfile,
            OUT VideoProfile*& pDestProfile, OUT VideoProfile*& pNegotiatedProfile);
    VideoProfile* GetNegotiatedDestProfile();
    MEDIA_DIRECTION GetNegotiatedDirection();
    VIDEO_RESOLUTION GetNegotiatedResolution(IN IMS_BOOL bCheckConfirmed = IMS_FALSE);
    IMS_SINT32 GetNegotiatedRtpPort();
    IMS_SINT32 GetMediaBandwidth();
    IMS_BOOL GetWidthHeightFromResolutionId(
            IN VIDEO_RESOLUTION eResolutionId, OUT IMS_UINT32* pnWidth, OUT IMS_UINT32* pnHeight);
    VIDEO_RESOLUTION GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel);

    // == PROTECTED METHOD ==========================================================
private:
    virtual IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType,
            IN MEDIA_DIRECTION eDir);
    virtual IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType,
            IN MEDIA_DIRECTION eDir);
    virtual IMS_BOOL FormReOffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_CONTENT_TYPE eType,
            IN MEDIA_DIRECTION eDir);
    virtual MEDIA_DIRECTION NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    virtual MEDIA_DIRECTION NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    virtual MEDIA_DIRECTION NegotiateReanswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    IMS_BOOL MakeSdpFromProfile(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile);
    IMS_BOOL MakeProfileFromSdp(IN ISessionDescriptor* pSessionDescriptor,
            IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile);
    virtual IMS_BOOL MakeNegotiatedPayload(IN VideoProfile::Payload* pSrcPayload,
            IN VideoProfile::Payload* pDstPayload, IN VideoConfiguration* pConfig,
            OUT VideoProfile::Payload* pNegoPayload);
    virtual IMS_BOOL MakeNegotiatedProfile(IN VideoProfile* pSrcProfile,
            IN VideoProfile* pDestProfile, IN IMS_BOOL bIsOfferReceived,
            OUT VideoProfile* pNegotiatedProfile);
    IMS_BOOL GetFmtpFromString(IN AString strFmtp, OUT VideoProfile::AvcFmtp* pFmtp);
    IMS_BOOL GetFmtpFromString(IN AString strFmtp, OUT VideoProfile::HevcFmtp* pFmtp);
    virtual VideoProfile::Payload* FindPayloadInProfile(
            IN VideoProfile* pProfile, IN VideoProfile::Payload* pPayload);
    IMS_SINT32 FindPayloadIndexFromProfile(
            IN VideoProfile* pProfile, IN VideoProfile::Payload* pPayload);
    MEDIA_DIRECTION UpdateDirectionToMine(
            IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase);
    IMS_BOOL GetAvpfFromAttributes(IN SdpMediaFormat* pMediaFormat,
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);
    IMS_BOOL GetCorrectImageIndex(IN IMS_SINT32 nPayloadTypeNum, IN IMSList<AString> objAttributes,
            OUT IMS_UINT32* nIndex);
    VIDEO_RESOLUTION GetResolutionFromSdp(IN VIDEO_CODEC codecType, IN AString strImageAttrFromSdp,
            IN AString strFrameSizeFromSdp, IN AString strSpropParam, IN IMS_SINT32 nQcif = -1);
    IMS_BOOL GetWidthHeightFromSdp_ImageAttr(IN AString strImageAttrFromSdp,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
    IMS_BOOL GetWidthHeightFromSdp_SpropParam(IN VIDEO_CODEC codecType, IN IMS_CHAR* szSprop,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
    IMS_BOOL GetWidthHeightFromSdp_FrameSize(IN AString strFrameSizeFromSdp,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
    VIDEO_RESOLUTION GetResolutionFromWidthHeight(IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight);
    IMS_BOOL MakeImageAttributeLine(IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId,
            OUT AString& strImageAttr);
    IMS_BOOL MakeFrameSizeLine(IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId,
            OUT AString& strFrameSize);
    // -- CapaNego API ---------------------------------------------------------------
    IMS_BOOL MakeCapaNegoProfileFromSdp(
            IN IMediaDescriptor* pDescriptor, OUT VideoProfile::CapaNego* pObjCapaNego);
    virtual IMS_BOOL MakeNegotiatedCapaNegoProfile(IN VideoProfile::CapaNego* pSrcCapaNego,
            IN VideoProfile::CapaNego* pDestCapaNego,
            OUT VideoProfile::CapaNego* pNegotiatedCapaNego);
    IMS_BOOL CheckAvpfFromProfile(IN VideoProfile* pProfile);
    IMS_BOOL GetAvpfFromAttributes_EX(IN IMediaDescriptor* pMediaDescriptor,
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);

    // == PROTECTED VARIABLE ==========================================================
private:
    IMSList<OaModel*> m_lstOaModel;
    VideoProfile m_objBaseProfile;
    MediaEnvironment* m_pMediaEnvironment;
    MEDIA_CONTENT_TYPE m_eSessionType;
    IMS_BOOL m_bNegotiatedCvoResult;
};
#endif /* _IMS_VIDEO_NEGO_H_ */
