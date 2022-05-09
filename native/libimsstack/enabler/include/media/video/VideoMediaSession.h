#ifndef _IMS_VIDEO_MEDIA_SESSION_H_
#define _IMS_VIDEO_MEDIA_SESSION_H_

// == INCLUDES =============================================================
#include <MediaQualityThreshold.h>
#include <VideoConfig.h>
#include "BaseSession.h"
#include "video/VideoDef.h"
#include "video/VideoProfile.h"
#include "config/VideoConfiguration.h"

using namespace android::telephony::imsmedia;

class VideoMediaSession : public BaseSession
{
private:
    VideoMediaSession(IN const VideoMediaSession& obj);
    VideoMediaSession& operator=(IN const VideoMediaSession& obj);

    // == PUBLIC METHOD ==============================================================
public:
    VideoMediaSession(IN IMS_SINT32 nSlodId = 0);
    virtual ~VideoMediaSession();
    void SetConfig(IN VideoConfiguration* pConfig);

    /*
     * Set VideoConfig for libimsmedia from src/dest/negotiated profile
     * @param pSrcProfile : local profile of the SDP negotiation
     * @param pDestProfile : peer profile of the SDP negotiation
     * @param pNegoProfile : negotiated profile of the SDP negotiation
     * return IMS_BOOL : false for error, true for successful
     */
    IMS_BOOL UpdateRtpConfig(IN VideoProfile* pSrcProfile, IN VideoProfile* pDestProfile,
            IN VideoProfile* pNegoProfile);
    IMS_BOOL IsDirectionHold();
    void HoldRtpConfig();
    IMS_BOOL UpdateMediaQualityThreshold(IN IMS_BOOL bIsHold);
    IMS_BOOL UpdateLocalEndPoint(IN VideoProfile* pNegoProfile);
    void UpdateLocalEndPoint(IN IPAddress objLocalAddr, IN IMS_UINT32 nPort);
    IMS_BOOL OnVideoMessages(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);

    /*
     * request OPEN_SESSION with updated VideoConfig
     */
    IMS_BOOL Open();

    /*
     * request MODIFY_SESSION with updated VideoConfig
     */
    IMS_BOOL Modify();

    /*
     * request ADD_CONFIG with updated VideoConfig
     */
    IMS_BOOL Add();

    /*
     * request DELETE_CONFIG with updated VideoConfig
     */
    IMS_BOOL Delete();

    /*
     * request CONFIRM_CONFIG with updated VideoConfig
     */
    IMS_BOOL Confirm();

    /*
     * request CLOSE_SESSION with updated VideoConfig
     */
    IMS_BOOL Close();

    /*
     * request SET_MEDIA_QUALITY with Video Media qualityThreshold
     */
    IMS_BOOL SetMediaQuality();
    IMS_BOOL SendDtmf(IN IMS_CHAR cDtmfCode, IN IMS_SINT32 nDuration);
    // notification - do it later
    //    virtual void SendNotifyToListener(IN IMS_SINT32 nNotify);
    //    virtual void SendNotifyInfoToListener(IMS_SINT32 nEvent, AString strNotifyInfo = IMS_NULL,
    //        IMS_SINT32 nNotifyInfo = -1, IMS_BOOL bNotifyInfo = IMS_FALSE);
    virtual void SendEventToUi(IN IMS_SINT32 nEvent, IN IMS_SINT32 nResult);
    IMS_BOOL OnSetSurfaceCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnFarframeInd(IN IMS_UINTP pParam);
    IMS_BOOL OnStartPreviewCameraCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnSelectCameraCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnChangeCameraZoomCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnSetPauseImageCmd(IN IMS_UINTP pParam);
    IMS_BOOL OnPeerDimensionChangedInd(IN IMS_UINTP pParam);
    IMS_BOOL OnVideoDataUsageCmd();
    IMS_BOOL OnVideoDataUsageInfoCmd(IN IMS_UINTP pParam);

protected:
    VideoConfiguration* m_pConfig;
    VideoConfig m_objVideoConfig;
    MediaQualityThreshold m_objMediaQualityThreshold;
    IPAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
    IMS_UINT32 m_nCameraId;
};

#endif /* End of _IMS_VIDEO_MEDIA_SESSION_H_*/
