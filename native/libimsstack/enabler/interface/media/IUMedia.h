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

#ifndef _IUMEDIA_H_
#define _IUMEDIA_H_

#include "IMSTypeDef.h"
#include "ImsMessageDef.h"
#include "IUEventParam.h"
#include "IUIMS.h"

//----------------------------------------------------------------------
// Messages for VT Media
//----------------------------------------------------------------------
#define IMS_MEDIA_FILENAME_LEN      128

#define IMS_MEDIA_MSG_REASON        300
#define IMS_MEDIA_MSG_NOTIFY        400

class IUMedia
{
    // Google_IMS_IF :: VIDEO_CALL_PROVIDER {
public:
    class ParamValue
    {
    public:
        // SETSURFACE_CMD
        enum
        {
            SURFACE_FAR = 1,
            SURFACE_NEAR = 2,
        };
    };
    // Google_IMS_IF :: VIDEO_CALL_PROVIDER }
};

class IUMediaReason
{
public:
    static const IMS_SINT32 REASON_NOERROR               = (IMS_MEDIA_MSG_REASON+1);
    static const IMS_SINT32 REASON_EVENT_FAIL            = (IMS_MEDIA_MSG_REASON+2);
    static const IMS_SINT32 UNDEFINED                    = (IMS_MEDIA_MSG_REASON+99);
};

class IUMediaNotify
{
public:
    static const IMS_SINT32 NOTIFY_ORIENTATION_PORTRAIT     = (IMS_MEDIA_MSG_NOTIFY+1);
    static const IMS_SINT32 NOTIFY_ORIENTATION_LANDSCAPE    = (IMS_MEDIA_MSG_NOTIFY+2);
    static const IMS_SINT32 UNDEFINED                       = (IMS_MEDIA_MSG_NOTIFY+99);
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------

// --------------- Media

class IUMediaSetSurfaceCmdParam :
        public IUEventParam
{
public:
    inline IUMediaSetSurfaceCmdParam(IN CONST IUMediaSetSurfaceCmdParam* pParam = NULL) :
            IUEventParam((IUEventParam*)pParam),
            nSessionID(0),
            nSurfaceType(0),
            nSurfaceTx(0),
            nSurfaceRx(0)
    {
        if (pParam == NULL) return;

        this->nSessionID = pParam->nSessionID;
        this->nSurfaceType = pParam->nSurfaceType;
        this->nSurfaceTx = pParam->nSurfaceTx;
        this->nSurfaceRx = pParam->nSurfaceRx;
    }

public:
    IMS_UINTP nSessionID;
    IMS_SINT32 nSurfaceType;
    IMS_SINTP nSurfaceTx;
    IMS_SINTP nSurfaceRx;
};

class IUMediaPreviewCameraCmdParam :
        public IUMediaSetSurfaceCmdParam
{
public:
    IMS_UINT32 nCamera;
public:
    IUMediaPreviewCameraCmdParam(IN CONST IUMediaPreviewCameraCmdParam* pParam = NULL) :
        IUMediaSetSurfaceCmdParam((IUMediaSetSurfaceCmdParam*)pParam),
    nCamera(0)
    {
        if (pParam == NULL)
        {
            return;
        }
        this->nCamera = pParam->nCamera;
    }
};

// ------ Video Quality Info On Screen
class IUMediaOnScreenDebugInfoParam :
        public IUEventParam
{
    public :
    inline IUMediaOnScreenDebugInfoParam() :
            nSessionID(0),
            nFrameRateRx(0),
            nBitRateRx(0),
            nFrameRateTx(0),
            nBitRateTx(0),
            nVQIndicator(0)
    {}

    inline virtual ~IUMediaOnScreenDebugInfoParam()
    {}

    public :
        IMS_UINTP nSessionID;
        IMS_UINT32 nFrameRateRx;
        IMS_UINT32 nBitRateRx;
        IMS_UINT32 nFrameRateTx;
        IMS_UINT32 nBitRateTx;
        IMS_UINT32 nVQIndicator;
};

// ------ Video Data Usage info
class IUMediaDataUsageInfoParam :
        public IUEventParam
{
    public :
    inline IUMediaDataUsageInfoParam() :
            nSessionID(0),
            nDataUsageRx(0),
            nDataUsageTx(0)
    {}

    inline virtual ~IUMediaDataUsageInfoParam()
    {}

    public :
        IMS_UINTP nSessionID;

        IMS_SLONG nDataUsageRx;
        IMS_SLONG nDataUsageTx;
};

class IUMediaResultIndParam :
        public IUEventParam
{
public :
    IMS_UINTP nSessionID;
    IMS_UINT32 nResult;
};

typedef IUMediaResultIndParam IUMediaFarFrameIndParam;
typedef IUMediaResultIndParam IUMediaPausedIndParam;
typedef IUMediaResultIndParam IUMediaResumedIndParam;
typedef IUMediaResultIndParam IUMediaCameraSelectedIndParam;
typedef IUMediaResultIndParam IUMediaCameraZoomChangedIndParam;
typedef IUMediaResultIndParam IUMediaCameraBrightnessChangedIndParam;
typedef IUMediaResultIndParam IUMediaCapturedIndParam;
typedef IUMediaResultIndParam IUMediaRecordingStartedParam;
typedef IUMediaResultIndParam IUMediaRecordingStoppedParam;
typedef IUMediaResultIndParam IUMediaAlternateImageStartedParam;
typedef IUMediaResultIndParam IUMediaAlternateImageStoppedParam;
typedef IUMediaResultIndParam IUMediaAudioStartedIndParam;
typedef IUMediaResultIndParam IUMediaDisplayUpdatedIndParam;
typedef IUMediaResultIndParam IUMediaViewSizeChangedIndParam;
typedef IUMediaResultIndParam IUMediaDisplaySwappedIndParam;
typedef IUMediaResultIndParam IUMediaAudioStoppedIndParam;

class IUMediaSessionKeyOnlyCmdParam :
        public IUEventParam
{
public :
    IMS_UINTP nSessionID;
};

typedef IUMediaSessionKeyOnlyCmdParam IUMediaResumeCmdParam;
typedef IUMediaSessionKeyOnlyCmdParam IUMediaStopRecordingCmdParam;
typedef IUMediaSessionKeyOnlyCmdParam IUMediaStopAlternateImageCmdParam;
typedef IUMediaSessionKeyOnlyCmdParam IUMediaAudioStopCmdParam;
typedef IUMediaSessionKeyOnlyCmdParam IUMediaSwapDisplayCmdParam;
typedef IUMediaSessionKeyOnlyCmdParam IUMediaMediaStartedIndParam;
typedef IUMediaSessionKeyOnlyCmdParam IUMediaAudioStartCmdParam;

class IUMediaValueAddedCmdParam :
        public IUEventParam
{
public :
    IMS_UINTP nSessionID;
    IMS_SINT32 nValue;
};

typedef IUMediaValueAddedCmdParam IUMediaPauseCmdParam;
typedef IUMediaValueAddedCmdParam IUMediaSelectCameraCmdParam;
typedef IUMediaValueAddedCmdParam IUMediaChangeCameraZoomCmdParam;
typedef IUMediaValueAddedCmdParam IUMediaChangeCameraBrightnessCmdParam;
typedef IUMediaValueAddedCmdParam IUMediaUpdateDisplayCmdParam;

class IUMediaViewResolutionCmdParam :
        public IUEventParam
{
public :
    IMS_UINTP nSessionID;
    IMS_SINT32 nWidth;
    IMS_SINT32 nHeight;
};

class IUMediaChangeOrientationCmdParam :
        public IUEventParam
{
public :
    IMS_UINTP nSessionID;
    IMS_SINT32 nView;
    IMS_SINT32 nOrientation;
};

class IUMediaFileRelatedCmdParam :
        public IUEventParam
{
public :
    IMS_UINTP   nSessionID;
    IMS_CHAR    szFile[IMS_MEDIA_FILENAME_LEN];
    IMS_UINT32    nValue;
};

class IUMediaVideoEffectDataParam :
        public IUEventParam
{
public :
    IMS_UINTP   nSessionID;
    IMS_UINT32  nKey;
    IMS_UINT32  nValue1;
    IMS_UINT32  nValue2;
};

typedef IUMediaFileRelatedCmdParam IUMediaCaptureCmdParam;
typedef IUMediaFileRelatedCmdParam IUMediaStartRecordingCmdParam;
typedef IUMediaFileRelatedCmdParam IUMediaStartAlternateImageCmdParam;

typedef enum _RTP_ANALYZER_MsgParam_TYPE_
{
    RTP_ANALYZER_TYPE_INVALID           = 0,
    RTP_ANALYZER_TYPE_DEBUGSCREEN,
    RTP_ANALYZER_TYPE_RTP,
    RTP_ANALYZER_TYPE_DRA,
    RTP_ANALYZER_TYPE_CIQ,
    RTP_ANALYZER_TYPE_RTT_DATA,
    RTP_ANALYZER_TYPE_CCT,
    RTP_ANALYZER_TYPE_DATAUSAGE,
    RTP_ANALYZER_TYPE_MAX
}RTP_ANALYZER_MsgParam_TYPE;

class IUMediaInternalMsgParam :
        public IUEventParam
{
public :
    IMS_UINT32    nWParam;
    IMS_UINT32    nLParam;
};

class IMediaAnalyzerBase
{
public:
    virtual ~IMediaAnalyzerBase()
    {}
};

class IMediaRTPInfoMsgParam :
        public IMediaAnalyzerBase
{
public :
    enum eRTPMediaType
    {
        RTP_MEDIA_TYPE_AUDIO            = 0,
        RTP_MEDIA_TYPE_VIDEO            = 1,
        RTP_MEDIA_TYPE_TEXT             = 2,
        RTP_MEDIA_TYPE_MAX
    };

    enum eRTPMediaDirection
    {
        RTP_MEDIA_DIRECTION_TX                = 0,
        RTP_MEDIA_DIRECTION_RX               = 1,
        RTP_MEDIA_DIRECTION_MAX               = 2
    };

public :
    inline IMediaRTPInfoMsgParam() :
            mediaType(RTP_MEDIA_TYPE_AUDIO),
            directionType(RTP_MEDIA_DIRECTION_TX),
            firstSSRC(0),
            durationTime(0),
            totalPacket(0),
            networkLostPacketCnt(0),
            droppedPacketCnt(0),
            latePacketCnt(0),
            meanJitter(0),
            maxJitter(0),
            maxDelta(0),
            peerDataPortNo(0),
            localDataPortNo(0),
            payloadTypeNo(0),
            meanDejitterbufferSize(0),
            meanDecoderDelay(0),
            updatedCMR(0),
            timeoutThreshold(0),
            bAudioPkt(IMS_TRUE)
    {}
    inline IMediaRTPInfoMsgParam(IN CONST IMediaRTPInfoMsgParam &objRHS) :
            mediaType(objRHS.mediaType),
            directionType(objRHS.directionType),
            firstSSRC(objRHS.firstSSRC),
            durationTime(objRHS.durationTime),
            totalPacket(objRHS.totalPacket),
            networkLostPacketCnt(objRHS.networkLostPacketCnt),
            droppedPacketCnt(objRHS.droppedPacketCnt),
            latePacketCnt(objRHS.latePacketCnt),
            meanJitter(objRHS.meanJitter),
            maxJitter(objRHS.maxJitter),
            maxDelta(objRHS.maxDelta),
            peerDataPortNo(objRHS.peerDataPortNo),
            localDataPortNo(objRHS.localDataPortNo),
            payloadTypeNo(objRHS.payloadTypeNo),
            meanDejitterbufferSize(objRHS.meanDejitterbufferSize),
            meanDecoderDelay(objRHS.meanDecoderDelay),
            updatedCMR(objRHS.updatedCMR),
            timeoutThreshold(objRHS.timeoutThreshold),
            bAudioPkt(objRHS.bAudioPkt)
    {}
    inline virtual ~IMediaRTPInfoMsgParam()
    {}

public :
    inline IMediaRTPInfoMsgParam& operator=(IN CONST IMediaRTPInfoMsgParam &objRHS)
    {
        if (this != &objRHS)
        {
            mediaType = objRHS.mediaType;
            directionType = objRHS.directionType;
            firstSSRC = objRHS.firstSSRC;
            durationTime = objRHS.durationTime;
            totalPacket = objRHS.totalPacket;
            networkLostPacketCnt = objRHS.networkLostPacketCnt;
            droppedPacketCnt = objRHS.droppedPacketCnt;
            latePacketCnt = objRHS.latePacketCnt;
            meanJitter = objRHS.meanJitter;
            maxJitter = objRHS.maxJitter;
            maxDelta = objRHS.maxDelta;
            peerDataPortNo = objRHS.peerDataPortNo;
            localDataPortNo = objRHS.localDataPortNo;
            payloadTypeNo = objRHS.payloadTypeNo;
            meanDejitterbufferSize = objRHS.meanDejitterbufferSize;
            meanDecoderDelay = objRHS.meanDecoderDelay;
            updatedCMR = objRHS.updatedCMR;
            timeoutThreshold = objRHS.timeoutThreshold;
            bAudioPkt = objRHS.bAudioPkt;
        }

        return (*this);
    }

public :

    eRTPMediaType mediaType;                // audio? video?
    eRTPMediaDirection directionType;        // RX? TX?
    IMS_UINT32 firstSSRC;                    // SSRC
    IMS_UINT32 durationTime;                // duration Time during a call
    IMS_UINT32 totalPacket;                // total packet count
    IMS_UINT32 networkLostPacketCnt;        // lost packet count by network
    IMS_UINT32 droppedPacketCnt;            // deleted packet count caused by playback keep time in jitter buffer
    IMS_UINT32 latePacketCnt;                // deleted packet count caused by lating in jitter buffer
    IMS_UINT32 meanJitter;                 // meanJitter , (use converting un-signed int value)
    IMS_UINT32 maxJitter;                    // J(i) = J(i-1) + (|D(i-1,i)| - J(i-1))/16 , (use converting un-signed int value)
    IMS_UINT32 maxDelta;                    // D(i,j) = (Rj - Ri) - (Sj - Si) = (Rj - Sj) - (Ri - Si) , (use converting un-signed int value)

    IMS_UINT32 peerDataPortNo;                // peer dataPort Number.
    IMS_UINT32 localDataPortNo;            // local dataPort Number.
    IMS_UINT32 payloadTypeNo;                // nego payloadTypeNumber
    IMS_UINT32 meanDejitterbufferSize;
    IMS_UINT32 meanDecoderDelay;
    IMS_UINT32 updatedCMR;
    IMS_UINT32 timeoutThreshold;
    IMS_BOOL bAudioPkt;
};

class IMediaDRAMsgParam :
        public IMediaAnalyzerBase
{
public:
    enum eDRAMediaType
    {
        DRA_MEDIA_TYPE_AUDIO            = 0,
        DRA_MEDIA_TYPE_VIDEO            = 1,
        DRA_MEDIA_TYPE_AUDIOVIDEO       = 2,
        DRA_MEDIA_TYPE_TEXT             = 3,
        DRA_MEDIA_TYPE_AUDIOTEXT        = 4,
        DRA_MEDIA_TYPE_VIDEOTEXT        = 5,
        DRA_MEDIA_TYPE_AUDIOVIDEOTEXT   = 6,
        DRA_MEDIA_TYPE_MAX
    };

    enum eDRAModeType
    {
        DRA_MEDIA_MODE_TX   = 0,
        DRA_MEDIA_MODE_RX   = 1,
        DRA_MEDIA_MODE_TRX  = 2
    };

    inline IMediaDRAMsgParam(IN CONST IMediaDRAMsgParam* pParam = NULL) :
            eMediaType(DRA_MEDIA_TYPE_AUDIO),
            eModeType(DRA_MEDIA_MODE_RX),
            nLossRate(0),
            nDelay(0),
            nJitter(0),
            nMeasuredPeriod(0),
            bAudioPkt(IMS_TRUE)
    {
        if (pParam == null)
        {
            return;
        }

        this->eMediaType = pParam->eMediaType;
        this->eModeType = pParam->eModeType;
        this->nLossRate = pParam->nLossRate;
        this->nDelay = pParam->nDelay;
        this->nJitter = pParam->nJitter;
        this->nMeasuredPeriod = pParam->nMeasuredPeriod;
        this->bAudioPkt = pParam->bAudioPkt;
    }

    inline IMediaDRAMsgParam& operator=(IN CONST IMediaDRAMsgParam &objParam)
    {
        if (this != &objParam)
        {
            this->eMediaType = objParam.eMediaType;
            this->eModeType = objParam.eModeType;
            this->nLossRate = objParam.nLossRate;
            this->nDelay = objParam.nDelay;
            this->nJitter = objParam.nJitter;
            this->nMeasuredPeriod = objParam.nMeasuredPeriod;
            this->bAudioPkt = objParam.bAudioPkt;
        }
        return (*this);
    }

    inline virtual ~IMediaDRAMsgParam()
    {}

public:
    eDRAMediaType   eMediaType;        // Audio: 0, Video: 1, text: 3,
    eDRAModeType    eModeType;        // Tx, Rx, TRx,
    IMS_SINT32      nLossRate;
    IMS_SINT32      nDelay;
    IMS_SINT32      nJitter;
    IMS_SINT32      nMeasuredPeriod;
    IMS_BOOL        bAudioPkt;
};

class IMediaAnalyzerStatMsgParam :
    public IMediaAnalyzerBase
{
public :
    enum eRTPMediaType
    {
        RTP_MEDIA_TYPE_INVALID          = -1,

        RTP_MEDIA_TYPE_AUDIO            = 0,
        RTP_MEDIA_TYPE_VIDEO            = 1,
        RTP_MEDIA_TYPE_TEXT             = 2,

        RTP_MEDIA_TYPE_MAX,
    };

public :
    inline IMediaAnalyzerStatMsgParam() :
            eMediaType(RTP_MEDIA_TYPE_AUDIO),
            nDurationTime(0),
            nMeanJitter(0),
            nMaxJitter(0),
            nNetworkLostPacketCnt(0)
    {}

    inline virtual ~IMediaAnalyzerStatMsgParam()
    {}

public :
    eRTPMediaType       eMediaType;              // Audio: 0, Video: 1, audioVideo: 2,
    IMS_UINT32          nDurationTime;           // duration Time during a call
    IMS_UINT32          nMeanJitter;             // meanJitter, (use converting un-signed int value)
    IMS_UINT32          nMaxJitter;              // J(i) = J(i-1) + (|D(i-1,i)| - J(i-1))/16 ,
                                                 //   (use converting un-signed int value)
    IMS_UINT32          nNetworkLostPacketCnt;   // lost packet count by network
};

class IMediaAnalyzerEventMsgParam :
        public IMediaAnalyzerBase
{
public :
    enum eRTPMediaType
    {
        RTP_MEDIA_TYPE_INVALID          = -1,

        RTP_MEDIA_TYPE_AUDIO            = 0,
        RTP_MEDIA_TYPE_VIDEO            = 1,
        RTP_MEDIA_TYPE_TEXT             = 2,

        RTP_MEDIA_TYPE_MAX,
    };

    enum eRTPMediaEventType
    {
        RTP_MEDIA_EVENT_TYPE_INVALID        = -1,

        RTP_MEDIA_EVENT_TYPE_PACKET_LOSS    = 0,
        RTP_MEDIA_EVENT_TYPE_JITTER_RESET   = 1,

        RTP_MEDIA_EVENT_TYPE_MAX,
    };

public :
    inline IMediaAnalyzerEventMsgParam() :
            eMediaType(RTP_MEDIA_TYPE_AUDIO),
            nEventType(RTP_MEDIA_EVENT_TYPE_INVALID),
            nTime(0),
            nReserved1(0),
            nReserved2(0),
            nReserved3(0)
    {}

    inline virtual ~IMediaAnalyzerEventMsgParam()
    {}

public :

    eRTPMediaType       eMediaType;              // Audio: 0, Video: 1, audioVideo: 2,
    IMS_UINT32          nEventType;              // Event Type
    IMS_UINT32          nTime;                   // Event Time
    IMS_UINT32          nReserved1;              // reserved value
    IMS_UINT32          nReserved2;              // reserved value
    IMS_UINT32          nReserved3;              // reserved value
};


class IMediaMOCAStatMsgParam :
        public IMediaAnalyzerBase
{
public :
    enum eRTPMediaType
    {
        RTP_MEDIA_TYPE_INVALID          = -1,

        RTP_MEDIA_TYPE_AUDIO            = 0,
        RTP_MEDIA_TYPE_VIDEO            = 1,
        RTP_MEDIA_TYPE_TEXT             = 2,

        RTP_MEDIA_TYPE_MAX,
    };

public :
    inline IMediaMOCAStatMsgParam() :
            eMediaType(RTP_MEDIA_TYPE_AUDIO),
            nDurationTime(0),
            nMeanJitter(0),
            nMaxJitter(0),
            nNetworkLostPacketCnt(0)
    {}

    inline virtual ~IMediaMOCAStatMsgParam()
    {}

    public :

    eRTPMediaType       eMediaType;              // Audio: 0, Video: 1, audioVideo: 2,
    IMS_UINT32          nDurationTime;           // duration Time during a call
    IMS_UINT32          nMeanJitter;             // meanJitter, (use converting un-signed int value)
    IMS_UINT32          nMaxJitter;              // J(i) = J(i-1) + (|D(i-1,i)| - J(i-1))/16 ,
                                                 //  (use converting un-signed int value)
    IMS_UINT32          nNetworkLostPacketCnt;   // lost packet count by network
};

class IMediaMOCAEventMsgParam :
        public IMediaAnalyzerBase
{
public :
    enum eRTPMediaType
    {
        RTP_MEDIA_TYPE_INVALID          = -1,

        RTP_MEDIA_TYPE_AUDIO            = 0,
        RTP_MEDIA_TYPE_VIDEO            = 1,
        RTP_MEDIA_TYPE_TEXT             = 2,

        RTP_MEDIA_TYPE_MAX,
    };

    enum eRTPMediaEventType
    {
        RTP_MEDIA_EVENT_TYPE_INVALID        = -1,

        RTP_MEDIA_EVENT_TYPE_PACKET_LOSS    = 0,
        RTP_MEDIA_EVENT_TYPE_JITTER_RESET   = 1,

        RTP_MEDIA_EVENT_TYPE_MAX,
    };

    public :
    inline IMediaMOCAEventMsgParam() :
            eMediaType(RTP_MEDIA_TYPE_AUDIO),
            nEventType(RTP_MEDIA_EVENT_TYPE_INVALID),
            nTime(0),
            nReserved1(0),
            nReserved2(0),
            nReserved3(0)
    {}

    inline virtual ~IMediaMOCAEventMsgParam()
    {}

    eRTPMediaType       eMediaType;              // Audio: 0, Video: 1, audioVideo: 2,
    IMS_UINT32          nEventType;              // Event Type
    IMS_UINT32          nTime;                   // Event Time
    IMS_UINT32          nReserved1;              // reserved value
    IMS_UINT32          nReserved2;              // reserved value
    IMS_UINT32          nReserved3;              // reserved value
};

#endif                                              // _IUMEDIA_H_
