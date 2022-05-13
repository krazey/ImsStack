/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20150329    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.imsservice.mmtel.videocall.base;

import android.app.ActivityManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.pm.ServiceInfo;
import android.net.Uri;
import android.os.AsyncTask;
import android.telecom.InCallService;
import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsVideoCallProvider;
import android.text.TextUtils;
import android.view.Surface;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.enabler.mtc.IUMtcMedia;
import com.android.imsstack.enabler.mtc.MtcMediaSession;
import com.android.imsstack.enabler.mtc.OnScreenDebugInfoVideo;
import com.android.imsstack.imsservice.mmtel.call.IVideoCallSession;
import com.android.imsstack.imsservice.mmtel.util.VideoDimension;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;

import java.util.ArrayList;
import java.util.List;

public class ImsVideoCallProviderBase extends ImsVideoCallProvider
        implements IVideoCallSession.EventListener {
    protected static final int CAMERA_ID_NONE = 99;

    protected static final int CALL_STATE_IDLE = 0;
    // Originating call is started
    protected static final int CALL_STATE_INITIATING = 1;
    // Incoming call is notified to framework and user alert will be done soon.
    protected static final int CALL_STATE_ALERTING = 2;
    // Call establishment is completed
    protected static final int CALL_STATE_ESTABLISHED = 3;
    // Call is terminated
    protected static final int CALL_STATE_TERMINATED = 4;
    // Video call upgrade request is in progress
    protected static final int CALL_STATE_VIDEO_UPGRADE_REQUESTED = 5;

    /*
     * Display orientation for video call
     */
    protected final static int ORIENTATION_0 = 0;
    protected final static int ORIENTATION_90 = 1;
    protected final static int ORIENTATION_180 = 2;
    protected final static int ORIENTATION_270 = 3;

    /*
     * Display type for video call
     */
    protected final static int DISPLAY_NEAR = 0;
    protected final static int DISPLAY_FAR = 1;
    protected final static int DISPLAY_NEAR_N_FAR = 2;

    /*
     * Display size
     */
    protected final static int DISPLAY_SIZE_NORMAL = 0;
    protected final static int DISPLAY_SIZE_FULL = 1;

    // Package names for a dialer
    private static final String DEFAULT_DIALER = "com.android.contacts";
    private static final String SKT_T_PHONE = "com.skt.prod.dialer";

    private final IVideoCallSession mCallSession;
    private MtcMediaSession mMediaSession = null;
    private MtcMediaSessionListenerProxy mListenerProxy = new MtcMediaSessionListenerProxy();
    private VideoDimension mVideoDimension = null;
    private PauseImageTask mPauseImageTask = null;
    private boolean mPauseImageSet = false;
    private boolean mIs3rdPartyDialerSetAsDefaultDialer = false;
    private boolean mIsTPhone = false;
    private int mCallState = CALL_STATE_IDLE;

    public ImsVideoCallProviderBase(IVideoCallSession callSession,
            MtcMediaSession mediaSession) {
        mCallSession = callSession;
        mMediaSession = mediaSession;

        if (mediaSession != null) {
            mediaSession.setListener(mListenerProxy);
        }

        if (callSession != null) {
            callSession.setVideoCallProvider(this);
            callSession.setEventListener(this);
        }

        mIs3rdPartyDialerSetAsDefaultDialer = is3rdPartyDialerSetAsDefaultDialerForVideoCall();

        if (isVideoCallFor3rdPartyDialer()) {
            logi("Default dialer for video call is a 3rd party dialer");
            mIsTPhone = SKT_T_PHONE.equals(getDefaultDialerPackage());
        }

        ImsPrivateProperties.Ephemeral.setBoolean(
                ImsPrivateProperties.Ephemeral.KEY_THIRD_PARTY_DIALER_FOR_VIDEO_CALL,
                isVideoCallFor3rdPartyDialer(), 0);
    }

    /**
     * FIXME: It needs to be checked how to pass the camera id.
     * ImsCamera#CAMERA_REAR (0)
     * ImsCamera#CAMERA_FRONT (1)
     * ImsCamera#CAMERA_FRONT2 (2)
     *      It's used for a special case to select front camera for video call.
     */
    @Override
    public void onSetCamera(String cameraId) {
        if (mMediaSession == null) {
            // Exception handling
            handleMediaSessionSelectCameraCompleted(
                    IUMtcMedia.Reason.REASON_EVENT_FAIL);
            return;
        }

        if (cameraId == null) {
            mMediaSession.selectCamera(CAMERA_ID_NONE);
        } else {
            int camId = getCameraIdInt(cameraId);

            if (camId >= ImsCamera.CAMERA_REAR) {
                mMediaSession.selectCamera(camId);
            } else {
                // Exception handling
                handleMediaSessionSelectCameraCompleted(
                        IUMtcMedia.Reason.REASON_EVENT_FAIL);
            }
        }
    }

    @Override
    public void onSetPreviewSurface(Surface surface) {
        if (mMediaSession == null) {
            // Exception handling
            return;
        }

        mMediaSession.setPreviewSurface(surface);
    }

    @Override
    public void onSetDisplaySurface(Surface surface) {
        if (mMediaSession == null) {
            // Exception handling
            return;
        }

        mMediaSession.setDisplaySurface(surface);
    }

    @Override
    public void onSetDeviceOrientation(int rotation) {
        if (mMediaSession == null) {
            return;
        }

        logi("onSetDeviceOrientation :: rotation=" + rotation);

        mMediaSession.setDisplayOrientation(
                DISPLAY_NEAR_N_FAR,
                getOrientationForUC(convertDeviceOrientation(rotation)));
    }

    @Override
    public void onSetZoom(float value) {
        if (mMediaSession == null) {
            return;
        }

        // FIXME: Do we need to handle the float?
        Float f = Float.valueOf(value);

        mMediaSession.setCameraZoom(f.intValue());
    }

    @Override
    public void onSendSessionModifyRequest(VideoProfile fromProfile, VideoProfile toProfile) {
        if (mCallSession == null) {
            return;
        }

        mCallSession.sendSessionModifyRequest(fromProfile, toProfile);
    }

    @Override
    public void onSendSessionModifyResponse(VideoProfile responseProfile) {
        if (mCallSession == null) {
            return;
        }

        mCallSession.sendSessionModifyResponse(responseProfile);
    }

    @Override
    public void onRequestCameraCapabilities() {
        // no-op
    }

    @Override
    public void onRequestCallDataUsage() {
        log("onRequestCallDataUsage");

        if (mMediaSession == null) {
            return;
        }

        mMediaSession.requestCallDataUsage();
    }

    @Override
    public void onSetPauseImage(Uri uri) {
        if ((mPauseImageTask != null)
                && (mPauseImageTask.getStatus() != AsyncTask.Status.FINISHED)) {
            log("PauseImageTask :: cancelling...");
            mPauseImageTask.cancel(true);
        }

        if (uri == null) {
            setPauseImageInternal(null);
            return;
        }

        int videoQuality = 0;
        ImsStreamMediaProfile mediaProfile
                = (mCallSession != null) ? mCallSession.getStreamMediaProfile() : null;

        if (mediaProfile != null) {
            videoQuality = VideoCallUtils.getVideoQualityFromMediaProfileForMediaInfo(
                    mediaProfile.getVideoQuality());
        }

        mPauseImageTask = new PauseImageTask(videoQuality);
        mPauseImageTask.execute(uri);
    }

    // [IMS][IMS_VT], 20150327, IMS_VIDEO_CALL {
    // @Override
    // @PRIVATE_IMS_API#Deprecated
    public void onStopAudio() {
        if (mMediaSession == null) {
            return;
        }

        mMediaSession.stopAudio();
    }

    // @Override
    // @PRIVATE_IMS_API
    public void onCaptureVideo(String file, int display) {
        if (mMediaSession == null) {
            return;
        }

        mMediaSession.captureVideo(file, display);
    }

    // @Override
    // @PRIVATE_IMS_API
    public void onSetMultitaskingState(int state) {
        mCallSession.setMultitaskingState(state);
    }

    // @Override
    // @PRIVATE_IMS_API (Deprecated from R-OS)
    public void onStartBackground() {
        if (mMediaSession == null) {
            handleMediaSessionStartBackgroundCompleted(
                    IUMtcMedia.Reason.REASON_EVENT_FAIL);
            return;
        }

        mMediaSession.startBackground();
    }

    // @Override
    // @PRIVATE_IMS_API (Deprecated from R-OS)
    public void onStopBackground() {
        if (mMediaSession == null) {
            handleMediaSessionStopBackgroundCompleted(
                    IUMtcMedia.Reason.REASON_EVENT_FAIL);
            return;
        }

        mMediaSession.stopBackground();
    }

    // @Override
    // @PRIVATE_IMS_API#Deprecated
    public void onSetAlternativeImage(String file) {
        if (mMediaSession == null) {
            handleMediaSessionSetAlternativeImageCompleted(
                    IUMtcMedia.Reason.REASON_EVENT_FAIL);
            return;
        }

        mPauseImageSet = true;
        mMediaSession.setAlternativeImage(file);
    }

    // @Override
    // @PRIVATE_IMS_API#Deprecated
    public void onClearAlternativeImage() {
        if (mMediaSession == null) {
            handleMediaSessionClearAlternativeImageCompleted(
                    IUMtcMedia.Reason.REASON_EVENT_FAIL);
            return;
        }

        if (mPauseImageSet) {
            mPauseImageSet = false;
            mMediaSession.clearAlternativeImage();
        } else {
            log("Ignore clearAlternativeImage");
            handleMediaSessionClearAlternativeImageCompleted(
                    IUMtcMedia.Reason.REASON_NOERROR);
        }
    }

    // @Override
    // @PRIVATE_IMS_API#Deprecated
    public void onSetCameraBrightness(int brightness) {
        if (mMediaSession == null) {
            return;
        }

        mMediaSession.setCameraBrightness(brightness);
    }

    // @Override
    // @PRIVATE_IMS_API
    public void onSetCameraOnOff(int onOff) {
        mCallSession.setCameraSetting(onOff);
    }

    // @Override
    // @PRIVATE_IMS_API
    public void onSwapDisplay() {
        if (mMediaSession == null) {
            return;
        }

        mMediaSession.swapDisplay();
    }

    // @Override
    // @PRIVATE_IMS_API
    public void onUpdateDisplay(int display) {
        if (mMediaSession == null) {
            return;
        }
        mMediaSession.updateDisplay(display);
    }

    /**
     * #DISPLAY_SIZE_NORMAL (0)
     * #DISPLAY_SIZE_FULL (1)
     */
    // @Override
    // @PRIVATE_IMS_API
    public void onSetDisplaySize(int display, int size) {
        if (mMediaSession == null) {
            return;
        }

        mMediaSession.setDisplaySize(display, size);
    }

    /**
     * #ORIENTATION_0 (0)
     * #ORIENTATION_90 (1)
     * #ORIENTATION_180 (2)
     * #ORIENTATION_270 (3)
     */
    // @Override
    // @PRIVATE_IMS_API
    public void onSetDisplayOrientation(int display, int orientation) {
        if (mMediaSession == null) {
            return;
        }

        mMediaSession.setDisplayOrientation(display, orientation);
    }
    // [IMS][IMS_VT], 20150327, IMS_VIDEO_CALL }

    @Override
    public void onCallEvent(int event) {
        log("VideoCallProvider :: callEvent=" + event);
        handleCallEvent(event);
    }

    @Override
    public void onSessionModificationAbortedByCameraOff() {
        // no-op
    }

    public void updateMediaSession(MtcMediaSession mediaSession) {
        if (mMediaSession != null) {
            mMediaSession.setListener(null);
            mMediaSession = null;
        }

        mMediaSession = mediaSession;

        if (mMediaSession != null) {
            mMediaSession.setListener(mListenerProxy);
        }
    }

    public void close() {
        if (mCallSession != null) {
            mCallSession.setEventListener(null);
        }

        if (mMediaSession != null) {
            mMediaSession.setListener(null);
            mMediaSession = null;
        }
    }

    protected final MtcMediaSession getMediaSession() {
        return mMediaSession;
    }

    protected final IVideoCallSession getVideoCallSession() {
        return mCallSession;
    }

    protected boolean isTPhone() {
        return mIsTPhone;
    }

    protected boolean isVideoCallFor3rdPartyDialer() {
        return mIs3rdPartyDialerSetAsDefaultDialer;
    }

    protected int getCallState() {
        return mCallState;
    }

    protected void setCallState(int state) {
        if (mCallState != state) {
            logi("VideoCallProvider :: callState - " + mCallState + " >> " + state);
            mCallState = state;
        }
    }

    protected void handleCallEvent(int event) {
        switch (event) {
        case IVideoCallSession.EVENT_CALL_IDLE:
            setCallState(CALL_STATE_IDLE);
            break;
        case IVideoCallSession.EVENT_CALL_INITIATING:
            setCallState(CALL_STATE_INITIATING);
            break;
        case IVideoCallSession.EVENT_CALL_ALERTING:
            setCallState(CALL_STATE_ALERTING);
            break;
        case IVideoCallSession.EVENT_CALL_ESTABLISHED:
            setCallState(CALL_STATE_ESTABLISHED);
            break;
        case IVideoCallSession.EVENT_CALL_TERMINATED:
            setCallState(CALL_STATE_TERMINATED);
            break;
        default:
            // no-op
            break;
        }
    }

    protected void handleMediaSessionStarted() {
        // no-op
    }

    protected void handleMediaSessionDataUsageChanged(long dataSize) {
        log("changeCallDataUsage dataSize : " + dataSize);
        changeCallDataUsage(dataSize);
    }

    protected void handleMediaSessionMediaInfoChanged(
            int mediaInfo, int intParam, String strParam) {
        // no-op
    }

    protected void handleMediaSessionPeerFirstVideoReceived() {
        // no-op
    }

    protected void handleMediaSessionPeerDisplayOrientationChanged(final int orientation) {
        // no-op
    }

    protected void handleMediaSessionPeerDimensionsChanged(final int videoResolution) {
        changePeerDimensions(getVideoWidth(videoResolution), getVideoHeight(videoResolution));
    }

    protected void handleMediaSessionSelectCameraCompleted(final int result) {
        int finalResult = 1;

        if (result != IUMtcMedia.Reason.REASON_NOERROR) {
            finalResult = 0;
        }
    }

    protected void handleMediaSessionStartBackgroundCompleted(final int result) {
        int finalResult = 1;

        if (result != IUMtcMedia.Reason.REASON_NOERROR) {
            finalResult = 0;
        }
    }

    protected void handleMediaSessionStopBackgroundCompleted(final int result) {
        int finalResult = 1;

        if (result != IUMtcMedia.Reason.REASON_NOERROR) {
            finalResult = 0;
        }
    }

    protected void handleMediaSessionSetAlternativeImageCompleted(final int result) {
        int finalResult = 1;

        if (result != IUMtcMedia.Reason.REASON_NOERROR) {
            finalResult = 0;
        }
    }

    protected void handleMediaSessionClearAlternativeImageCompleted(final int result) {
        int finalResult = 1;

        if (result != IUMtcMedia.Reason.REASON_NOERROR) {
            finalResult = 0;
        }
    }

    protected VideoDimension getCurrentVideoDimension() {
        return mVideoDimension;
    }

    protected void setCurrentVideoDimension(int width, int height) {
        mVideoDimension = null;
        mVideoDimension = new VideoDimension(width, height);
    }

    protected void setPauseImageInternal(String file) {
        if (mMediaSession == null) {
            if (TextUtils.isEmpty(file)) {
                handleMediaSessionClearAlternativeImageCompleted(
                        IUMtcMedia.Reason.REASON_EVENT_FAIL);
            } else {
                handleMediaSessionSetAlternativeImageCompleted(
                        IUMtcMedia.Reason.REASON_EVENT_FAIL);
            }
            return;
        }

        if (TextUtils.isEmpty(file)) {
            if (mPauseImageSet) {
                mPauseImageSet = false;
                mMediaSession.clearAlternativeImage();
            } else {
                log("Ignore clearAlternativeImage");
                handleMediaSessionClearAlternativeImageCompleted(
                        IUMtcMedia.Reason.REASON_NOERROR);
            }
        } else {
            mPauseImageSet = true;
            mMediaSession.setAlternativeImage(file);
        }
    }

    protected void updateReversedPeerDimensionFromMediaProfile(int orientation,
            boolean enforceUpdate) {
        ImsStreamMediaProfile mediaProfile
                = (mCallSession != null) ? mCallSession.getStreamMediaProfile() : null;

        if (mediaProfile != null) {
            VideoDimension videoDimension = null;
            int videoQuality = VideoCallUtils.getVideoQualityFromMediaProfileForMediaInfo(
                    mediaProfile.getVideoQuality());

            if ((orientation == VideoCallUtils.ORIENTATION_PORTRAIT)
                    && !VideoCallUtils.isVideoPortrait(videoQuality)) {
                videoDimension = VideoCallUtils.getReversedVideoDimension(videoQuality);
            } else if ((orientation == VideoCallUtils.ORIENTATION_LANDSCAPE)
                    && VideoCallUtils.isVideoPortrait(videoQuality)) {
                videoDimension = VideoCallUtils.getReversedVideoDimension(videoQuality);
            }

            if (videoDimension != null) {
                setCurrentVideoDimension(videoDimension.getWidth(), videoDimension.getHeight());
                changePeerDimensions(videoDimension.getWidth(), videoDimension.getHeight());
            } else if (enforceUpdate) {
                VideoDimension vd = VideoCallUtils.getVideoDimension(videoQuality);

                if (vd != null) {
                    handleMediaSessionPeerDimensionsChanged(
                            getVideoResolution(vd.getWidth(), vd.getHeight()));
                }
            }
        }
    }

    protected void updateReversedPeerDimensionFromVideoDimension(int orientation,
            boolean enforceUpdate) {
        boolean videoDimensionChanged = false;
        int width = mVideoDimension.getWidth();
        int height = mVideoDimension.getHeight();

        if (orientation == VideoCallUtils.ORIENTATION_PORTRAIT) {
            if (width > height) {
                int temp = height;
                height = width;
                width = temp;

                videoDimensionChanged = true;
            }
        } else if (orientation == VideoCallUtils.ORIENTATION_LANDSCAPE) {
            if (width < height) {
                int temp = height;
                height = width;
                width = temp;

                videoDimensionChanged = true;
            }
        }

        if (videoDimensionChanged) {
            setCurrentVideoDimension(width, height);
            changePeerDimensions(width, height);
        } else if (enforceUpdate) {
            changePeerDimensions(width, height);
        }
    }

    protected static boolean checkRadius(int angle, int criteria, int threshold) {
        int limitLeft = criteria - threshold;
        int limitRight = criteria + threshold;

        return (angle >= limitLeft) && (angle < limitRight);
    }

    protected static int convertDeviceOrientation(int angle) {
        int orientation = ORIENTATION_0;

        // This is for the reference devices.
        final int orientationThreshold = 45;

        if (checkRadius(angle, 90, orientationThreshold)) {
            orientation = ORIENTATION_90;
        } else if (checkRadius(angle, 180, orientationThreshold)) {
            orientation = ORIENTATION_180;
        } else if (checkRadius(angle, 270, orientationThreshold)) {
            orientation = ORIENTATION_270;
        }

        return orientation;
    }

    protected static int getOrientationForUC(int orientation) {
        switch (orientation) {
        case ORIENTATION_90:
            return MtcMediaSession.ORIENTATION_90;
        case ORIENTATION_180:
            return MtcMediaSession.ORIENTATION_180;
        case ORIENTATION_270:
            return MtcMediaSession.ORIENTATION_270;
        default:
            return MtcMediaSession.ORIENTATION_0;
        }
    }

    protected static int getVideoResolution(int width, int height) {
        return (int)(((width << 16) & 0xFFFF0000) | (height & 0x0000FFFF));
    }

    protected static int getVideoHeight(int resolution) {
        return (int)(resolution & 0x0000FFFF);
    }

    protected static int getVideoWidth(int resolution) {
        return (int)((resolution >> 16) & 0x0000FFFF);
    }

    protected static String getDefaultDialerPackage() {
        TelecomManager tm = ImsGlobal.getInstance().getSystemService(
                TelecomManager.class);
        return (tm != null) ? tm.getDefaultDialerPackage() : null;
    }

    protected static boolean is3rdPartyDialerSetAsDefaultDialerForVideoCall() {
        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            // It's always 3rd party dialer in JUMP.
            return true;
        }

        String packageName = getDefaultDialerPackage();

        if (TextUtils.isEmpty(packageName)
                || DEFAULT_DIALER.equals(packageName)) {
            return false;
        }

        PackageManager pm = ImsGlobal.getInstance().getPackageManager();

        if (pm == null) {
            return true;
        }

        List<ServiceInfo> infos = new ArrayList<>();
        Intent serviceIntent = new Intent(InCallService.SERVICE_INTERFACE);

        serviceIntent.setPackage(packageName);

        for (ResolveInfo entry : pm.queryIntentServicesAsUser(
                serviceIntent,
                PackageManager.GET_META_DATA,
                ActivityManager.getCurrentUser())) {
            if (entry.serviceInfo != null) {
                infos.add(entry.serviceInfo);
            }
        }

        if (infos.isEmpty()) {
            // Dialer only package (w/o InCallUI)
            log("3rd party dialer : No InCallService - " + packageName);
            return false;
        }

        boolean is3rdPartyCallUI = false;

        for (ServiceInfo serviceInfo : infos) {
            if (serviceInfo.metaData != null
                    && serviceInfo.metaData.getBoolean(
                        TelecomManager.METADATA_IN_CALL_SERVICE_UI, false)) {
                // Package to display Call UI
                log("3rd party dialer : packageName="
                        + serviceInfo.packageName + ", name=" + serviceInfo.name);
                is3rdPartyCallUI = true;
            }
        }

        return is3rdPartyCallUI;
    }

    protected static int getCameraIdInt(String cameraId) {
        try {
            int camId = Integer.parseInt(cameraId);

            if (camId >= ImsCamera.CAMERA_REAR) {
                return camId;
            }
        } catch (NumberFormatException e) {
            logi("Invalid cameraId=" + cameraId);
        }

        return (-1);
    }

    protected static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    protected static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    private class PauseImageTask extends AsyncTask<Uri, Void, String> {
        private int mVideoQuality;

        public PauseImageTask(int videoQuality) {
            mVideoQuality = videoQuality;
        }

        // Save alternative image (in IMS internal storage) in background
        @Override
        protected String doInBackground(Uri... params) {
            if (isCancelled()) {
                logi("PauseImageTask :: already cancelled");
                return null;
            }

            Uri uri = params[0];

            logi("PauseImageTask :: uri=" + ImsLog.hiddenString(uri.toString())
                    + ", videoQuality=" + mVideoQuality);

            return VideoCallUtils.getPauseImageFile(uri, mVideoQuality);
        }

        @Override
        protected void onPostExecute(String file) {
            if (file == null) {
                logi("PauseImageTask :: Uri is malformed or not found");
                handleMediaSessionSetAlternativeImageCompleted(
                        IUMtcMedia.Reason.REASON_EVENT_FAIL);
                return;
            }

            setPauseImageInternal(file);
        }

        @Override
        protected void onCancelled (String result) {
            log("PauseImageTask :: onCancelled");
        }
    }

    private class MtcMediaSessionListenerProxy extends MtcMediaSession.Listener {
        @Override
        public void onMediaSessionAudioStarted(MtcMediaSession session) {
            // no-op
        }

        @Override
        public void onMediaSessionAudioStopped(MtcMediaSession session) {
            // no-op
        }

        @Override
        public void onMediaSessionAudioPaused(MtcMediaSession session) {
            // no-op
        }

        @Override
        public void onMediaSessionStarted(MtcMediaSession session) {
            handleMediaSessionStarted();
        }

        @Override
        public void onMediaSessionDebugInfoChanged(MtcMediaSession session,
                OnScreenDebugInfoVideo debugInfo) {
            // no-op
        }

        @Override
        public void onMediaSessionDataUsageChanged(MtcMediaSession session,
                long dataSize) {
            handleMediaSessionDataUsageChanged(dataSize);
        }

        @Override
        public void onMediaSessionMediaInfoChanged(MtcMediaSession session,
                int mediaInfo, int intParam, String strParam) {
            handleMediaSessionMediaInfoChanged(mediaInfo, intParam, strParam);
        }

        @Override
        public void onMediaSessionPeerFirstVideoReceived(MtcMediaSession session) {
            handleMediaSessionPeerFirstVideoReceived();
        }

        @Override
        public void onMediaSessionPeerDisplayOrientationChanged(MtcMediaSession session,
                final int orientation) {
            handleMediaSessionPeerDisplayOrientationChanged(orientation);
        }

        @Override
        public void onMediaSessionPeerDimensionsChanged(MtcMediaSession session,
                final int videoResolution) {
            handleMediaSessionPeerDimensionsChanged(videoResolution);
        }

        @Override
        public void onMediaSessionSurfaceUpdateRequired(MtcMediaSession session) {
            // no-op
        }

        @Override
        public void onMediaSessionSelectCameraCompleted(MtcMediaSession session,
                final int result) {
            handleMediaSessionSelectCameraCompleted(result);
        }

        @Override
        public void onMediaSessionSetCameraZoomCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        @Override
        public void onMediaSessionSetCameraBrightnessCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        @Override
        public void onMediaSessionCameraFrameReceived(MtcMediaSession session) {
            // no-op
        }

        @Override
        public void onMediaSessionStartBackgroundCompleted(MtcMediaSession session,
                final int result) {
            handleMediaSessionStartBackgroundCompleted(result);
        }

        @Override
        public void onMediaSessionStopBackgroundCompleted(MtcMediaSession session,
                final int result) {
            handleMediaSessionStopBackgroundCompleted(result);
        }

        @Override
        public void onMediaSessionCaptureVideoCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        @Override
        public void onMediaSessionSetAlternativeImageCompleted(MtcMediaSession session,
                final int result) {
            handleMediaSessionSetAlternativeImageCompleted(result);
        }

        @Override
        public void onMediaSessionClearAlternativeImageCompleted(MtcMediaSession session,
                final int result) {
            handleMediaSessionClearAlternativeImageCompleted(result);
        }

        @Override
        public void onMediaSessionSwapDisplayCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        @Override
        public void onMediaSessionUpdateDisplayCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        @Override
        public void onMediaSessionSetDisplaySizeCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        @Override
        public void onMediaSessionSetDisplayOrientationCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }
    };
}
