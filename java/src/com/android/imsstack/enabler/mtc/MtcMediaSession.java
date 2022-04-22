/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20131015    hwangoo.park@           Created
    </table>
    Description
*/

package com.android.imsstack.enabler.mtc;

import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Parcel;
import android.view.Display;
import android.view.Surface;
import com.android.imsstack.R;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.media.IMediaListener;
import com.android.imsstack.enabler.media.MediaConstants;
import com.android.imsstack.enabler.media.MediaFactory;
import com.android.imsstack.enabler.media.MediaSession;
import com.android.imsstack.jni.JNIIms;
import com.android.imsstack.util.ImsLog;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MtcMediaSession {
    /**
     * Listener for events relating to an MTC media session.
     * <p>Many of these events are also received by {@link MtcMediaSession.Listener}.</p>
     */
    public static class Listener {
        /**
         * Called when the audio resource is ready to start.
         */
        public void onMediaSessionAudioStarted(MtcMediaSession session) {
            // no-op
        }

        /**
         * Called when the audio resource is completely released by the IMS service.
         */
        public void onMediaSessionAudioStopped(MtcMediaSession session) {
            // no-op
        }

        /**
         * Called when the audio is paused.
         */
        public void onMediaSessionAudioPaused(MtcMediaSession session) {
            // no-op
        }

        /**
         * Called when the media (audio or audio/video) session based on the service is started.
         */
        public void onMediaSessionStarted(MtcMediaSession session) {
            // no-op
        }

        /**
         * Called when the debug information for the media debug screen is changed.
         */
        public void onMediaSessionDebugInfoChanged(MtcMediaSession session,
                OnScreenDebugInfoVideo debugInfo) {
            // no-op
        }

        public void onMediaSessionDataUsageChanged(MtcMediaSession session,
                long dataSize) {
            // no-op
        }

        /**
         * Called when the media information is changed in the media session.
         */
        public void onMediaSessionMediaInfoChanged(MtcMediaSession session,
                int mediaInfo, int intParam, String strParam) {
            // no-op
        }

        /**
         * Called when the first video packet is received from the remote endpoint.
         */
        public void onMediaSessionPeerFirstVideoReceived(MtcMediaSession session) {
            // no-op
        }

        /**
         * Called when the orientation of the peer display is changed.
         */
        public void onMediaSessionPeerDisplayOrientationChanged(MtcMediaSession session,
                final int orientation) {
            // no-op
        }

        /**
         * Called when the peer dimension is changed
         */
        public void onMediaSessionPeerDimensionsChanged(MtcMediaSession session,
                final int videoResolution) {
            // no-op
        }

        /**
         * Called when the surface update is required.
         */
        public void onMediaSessionSurfaceUpdateRequired(MtcMediaSession session) {
            // no-op
        }

        /**
         * Called when the selectCamera() operation is done.
         */
        public void onMediaSessionSelectCameraCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the setCameraZoom() operation is done.
         */
        public void onMediaSessionSetCameraZoomCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the setCameraBrightness() operation is done.
         */
        public void onMediaSessionSetCameraBrightnessCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the first camera frame received operation is done.
         */
        public void onMediaSessionCameraFrameReceived(MtcMediaSession session) {
            // no-op
        }

        /**
         * Called when the startBackground() operation is done.
         */
        public void onMediaSessionStartBackgroundCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the stopBackground() operation is done.
         */
        public void onMediaSessionStopBackgroundCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the captureVideo() operation is done.
         */
        public void onMediaSessionCaptureVideoCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the setAlternativeImage() operation is done.
         */
        public void onMediaSessionSetAlternativeImageCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the clearAlternativeImage() operation is done.
         */
        public void onMediaSessionClearAlternativeImageCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the swapDisplay() operation is done.
         */
        public void onMediaSessionSwapDisplayCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the updateDisplay() operation is done.
         */
        public void onMediaSessionUpdateDisplayCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the setDisplaySize() operation is done.
         */
        public void onMediaSessionSetDisplaySizeCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }

        /**
         * Called when the setDisplayOrientation() operation is done.
         */
        public void onMediaSessionSetDisplayOrientationCompleted(MtcMediaSession session,
                final int result) {
            // no-op
        }
    }

    public static class RttListener {
        /**
         * Called when Rx RTT text is received from peer.
         */
        public void onRttMessageReceived(MtcMediaSession session, String data) {
            // no-op
        }

        /**
         * Called when Rx audio receiving is started/stopped while on-going RTT session
         */
        public void onRttAudioIndication(MtcMediaSession session, boolean status) {
            // no-op
        }
    }

    public static class MediaInfoEvent {
        private int mType = 0;
        private int mIntParam = 0;
        private String mStringParam = "";

        public MediaInfoEvent(int type, int intParam, String stringParam) {
            mType = type;
            mIntParam = intParam;
            mStringParam = stringParam;
        }

        @Override
        public String toString() {
            return "[ MediaInfoEvent: type=" + mType
                    + ", intP=" + mIntParam
                    + ", stringP=" + mStringParam + " ]";
        }

        public void dispose() {
            mType = 0;
            mIntParam = 0;
            mStringParam = "";
        }

        public int getType() {
            return mType;
        }

        public int getIntParam() {
            return mIntParam;
        }

        public String getStringParam() {
            return mStringParam;
        }

        public boolean isValidEvent() {
            return mType != 0;
        }

        public void setParams(int intParam, String stringParam) {
            mIntParam = intParam;
            mStringParam = stringParam;
        }
    }

    /**
     * Display orientation.
     */
    public final static int ORIENTATION_0 = 0;
    public final static int ORIENTATION_90 = 1;
    public final static int ORIENTATION_270 = 2;
    public final static int ORIENTATION_180 = 3;

    protected static final String NAME_DISPLAY_SURFACE = "mDisplaySurface";
    protected static final String NAME_PREVIEW_SURFACE = "mPreviewSurface";

    private static final String NAME_CLASS = MtcMediaSession.class.getName().replace(".", "/");

    private static final int STATE_NONE = 0x0;
    private static final int STATE_AUDIO_STARTED = 0x1;
    private static final int STATE_AUDIO_PAUSED = 0x2;
    private static final int STATE_MEDIA_STARTED = 0x4;

    private static final int MAX_SIZE = 10 * 1024 * 1024; //10MB Check for security
    //for swiveling support device
    private static final int STATE_SWIVEL_NORMAL = 0;
    private static final int STATE_SWIVELED = 1;
    /**
     * This surface variables will be accessed by the native layer.
     * It just grabs the object to be enable to access the surface.
     */
    private static Surface mPreviewSurface = null;
    private static Surface mDisplaySurface = null;

    private final Object mLock = new Object();
    private Context mContext;
    private Call mCall;
    private int mState = STATE_NONE;
    private MtcMediaSession.Listener mListener = null;
    private MtcMediaSession.RttListener mRttListener = null;
    private IMediaListener mMediaListener = null;
    private MediaInfoEvent mMediaInfoEvent = null;
    private Bitmap mBackgroundBitmap = null;
    private boolean mVQINeedToBeReported = false;
    public int mSwivelState = STATE_SWIVEL_NORMAL;
    public int mPrevOrientation = 0;
    private final MediaSession mMediaSession;

    public MtcMediaSession(IBaseContext context, Call call) {
        mContext = context.getContext();
        mCall = call;
        mMediaSession = MediaFactory.createMediaSession(context, this);
    }

    public void dispose() {
        if (!isMediaSessionValid()) {
            return;
        }

        synchronized (mLock) {
            mCall = null;
            mListener = null;
            mRttListener = null;
            mMediaListener = null;
            mMediaInfoEvent = null;
        }
    }

    public void setListener(MtcMediaSession.Listener listener) {
        synchronized (mLock) {
            mListener = listener;
        }
    }

    public void setRttListener(MtcMediaSession.RttListener listener) {
        synchronized (mLock) {
            mRttListener = listener;
        }
    }

    public void setMediaListener(IMediaListener listener) {
        synchronized (mLock) {
            mMediaListener = listener;
        }
    }

    public boolean isSwivelSupport() {
        return false;
    }

    /**
     * Gets the call-id to identify the current call media.
     */
    public long getCallId() {
        return isMediaSessionValid() ? mCall.getNativeCallId() : 0;
    }

    public void setPreviewSurface(Surface surface) {
        log("setPreviewSurface :: surface=" + surface);

        Parcel parcel = Parcel.obtain();

        mPreviewSurface = surface;

        /**
         * The argument will be passed the following orders:
         *      surface-presence
         *      surface-type
         *      ref-class-name
         *      ref-global-member-name
         */
        parcel.writeInt(IUMtcMedia.SETSURFACE_CMD);
        parcel.writeInt((surface == null) ? 0 : 1);
        parcel.writeInt(IUMtcMedia.ParamValue.SURFACE_NEAR);

        if (surface != null) {
            parcel.writeString(NAME_CLASS);
            parcel.writeString(NAME_PREVIEW_SURFACE);
        }

        sendRequest(parcel);
    }

    public void startPreviewCamera(Surface surface)  {
        log("startPreviewCamera :: surface=" + surface);
        startPreviewCamera(surface, 1/* ImsCamera.CAMERA_FRONT */);
    }

    public void startPreviewCamera(Surface surface, int camera) {
        log("startPreviewCamera :: surface=" + surface + ", camera=" + camera);

        Parcel parcel = Parcel.obtain();

        mPreviewSurface = surface;

        /**
         * The argument will be passed the following orders:
         *      surface-presence
         *      surface-type
         *      ref-class-name
         *      ref-global-member-name
         */
        parcel.writeInt(IUMtcMedia.START_PREVIEW_CAMERA_CMD);
        parcel.writeInt(camera);
        parcel.writeInt((surface == null) ? 0 : 1);
        parcel.writeInt(IUMtcMedia.ParamValue.SURFACE_NEAR);

        if (surface != null) {
            parcel.writeString(NAME_CLASS);
            parcel.writeString(NAME_PREVIEW_SURFACE);
        }

        sendRequest(parcel);
    }

    public void stopPreview() {
        log("stopPreview");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.STOP_PREVIEW_CAMERA_CMD);
        // surface-presence
        parcel.writeInt(0);
        // surface-type
        parcel.writeInt(0);

        sendRequest(parcel);
    }

    public void setDisplaySurface(Surface surface) {
        log("setDisplaySurface :: surface=" + surface);

        Parcel parcel = Parcel.obtain();

        mDisplaySurface = surface;

        /**
         * The argument will be passed the following orders:
         *      surface-presence
         *      surface-type
         *      ref-class-name
         *      ref-global-member-name
         */
        parcel.writeInt(IUMtcMedia.SETSURFACE_CMD);
        parcel.writeInt((surface == null) ? 0 : 1);
        parcel.writeInt(IUMtcMedia.ParamValue.SURFACE_FAR);

        if (surface != null) {
            parcel.writeString(NAME_CLASS);
            parcel.writeString(NAME_DISPLAY_SURFACE);
        }

        sendRequest(parcel);
    }

    public static void deleteRecursive(File fileOrDirectory) {
        if (fileOrDirectory != null) {
            if (fileOrDirectory.isDirectory()) {
                File[] all = fileOrDirectory.listFiles();
                if (all != null) {
                    for (File child : all) {
                        deleteRecursive(child);
                    }
                }
            }
            fileOrDirectory.delete();
        }
    }

    public static void copyOrThrow(InputStream is, File dest) throws IOException {
        FileOutputStream fos = null;
        try {
            if (is == null) {
                return;
            }
            int size = is.available();
            if (size == 0 || size > MAX_SIZE) {
                log("check InputStream size");
                return;
            }
            byte[] buffer = new byte[size];
            if (is.read(buffer) > 0) {
                fos = new FileOutputStream(dest);
                fos.write(buffer);
                fos.flush();
            }
        } catch (IOException e) {
            e.printStackTrace();
        } catch (OutOfMemoryError oom) {
            oom.printStackTrace();
        } finally {
            try {
                if (is != null) {
                    is.close();
                }
            } catch (IOException ioe) {
                ioe.printStackTrace();
            }
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException ioe) {
                    ioe.printStackTrace();
                }
            }
        }
    }

    /**
     * Make folder and copy file from raw in app to files in absolute path.
     *
     * @param ctx context
     * @param folderName root file name
     * @param filesName  Name of file to copy in folderName
     */
    public static void copyRawData(Context ctx, String folderName, String[] filesName) {
        if (folderName == null || filesName == null || filesName.length <= 0) {
            log("check parameter");
            return;
        }
        File dataFolder = new File(folderName);
        if (false == dataFolder.exists() ) {
            dataFolder.mkdir();
        }
        if (dataFolder.exists() && dataFolder.isDirectory()) {
            boolean filesAreExist = true;
            File[] files = new File[filesName.length];
            for (int i = 0; i < filesName.length; i++) {
                files[i] = new File(dataFolder, filesName[i]);
                if (false == files[i].exists()) {
                    filesAreExist = false;
                }
            }
            if (filesAreExist) {
                log("Do Not Need Data File Copy!!!!");
                return;
            } else {
                for (int i = 0; i < filesName.length; i++) {
                    try {
                        String resourceName = filesName[i].substring(0,
                            filesName[i].indexOf("."));
                            int resId = ctx.getResources().getIdentifier("@raw/"
                                + resourceName, "raw", ctx.getPackageName());
                            InputStream is = ctx.getResources().openRawResource(resId);
                            copyOrThrow(is, files[i]);
                    } catch (Exception e) {
                        log(e.toString());
                        deleteRecursive(files[i]);
                    }
                }
            }
        }
    }

    /**
     * Releases the audio resources.
     */
    public void stopAudio() {
        log("stopAudio");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.AUDIO_STOP_CMD);

        sendRequest(parcel);
    }

    /**
     * MULTI-TASKING
     * Notifies the IMS stack that the application will go to the background.
     */
    public void startBackground() {
        log("startBackground");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.START_BACKGROUND_CMD);

        sendRequest(parcel);
    }

    /**
     * MULTI-TASKING
     * Notifies the IMS stack that the application will go to the foreground.
     */
    public void stopBackground() {
        log("stopBackground");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.STOP_BACKGROUND_CMD);

        sendRequest(parcel);
    }

    /**
     * VIDEO_CAPTURING
     * Captures the last video image.
     *
     * @param file the file locator which the image is stored
     * @param display the display which the image is captured
     */
    public void captureVideo(String file, int display) {
        log("captureVideo :: file=" + file + ", display=" + display);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.CAPTURE_CMD);
        parcel.writeString(file);
        parcel.writeInt(display);

        sendRequest(parcel);
    }

    /**
     * ALTERNATIVE_IMAGE
     * Sets the alternative image during a video call and stops the real-time video streaming.
     *
     * @param file the file locator which the image is located
     */
    public void setAlternativeImage(String file) {
        log("setAlternativeImage :: file=" + ImsLog.hiddenString(file));

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.START_ALTERNATE_IMAGE_CMD);
        parcel.writeString(file);

        sendRequest(parcel);
    }

    /**
     * ALTERNATIVE_IMAGE
     * Clears the alternative image and starts the real-time video streaming.
     */
    public void clearAlternativeImage() {
        log("clearAlternativeImage");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.STOP_ALTERNATE_IMAGE_CMD);

        sendRequest(parcel);
    }

    /**
     * CAMERA
     * Selects the camera type for video call.
     *
     * @param camera the camera type
     *          {@link ImsCamera#CAMERA_REAR}
     *          {@link ImsCamera#CAMERA_FRONT}
     */
    public void selectCamera(int camera) {
        log("selectCamera :: camera=" + camera);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.SELECT_CAMERA_CMD);
        parcel.writeInt(camera);

        sendRequest(parcel);
    }

    /**
     * CAMERA
     * Sets the zoom property of the camera.
     *
     * @param zoom the zoom (0 ~ 10)
     */
    public void setCameraZoom(int zoom) {
        log("setCameraZoom :: zoom=" + zoom);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.CHANGE_CAMERA_ZOOM_CMD);
        parcel.writeInt(zoom);

        sendRequest(parcel);
    }

    /**
     * CAMERA
     * Sets the brightness property of the camera.
     *
     * @param brightness the brightness (-5 ~ 5)
     */
    public void setCameraBrightness(int brightness) {
        log("setCameraBrightness :: brightness=" + brightness);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.CHANGE_CAMERA_BRIGHTNESS_CMD);
        parcel.writeInt(brightness);

        sendRequest(parcel);
    }

    /**
     * DISPLAY
     * Swaps the display between near and far.
     */
    public void swapDisplay() {
        log("swapDisplay");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.SWAP_DISPLAY_CMD);

        sendRequest(parcel);
    }

    /**
     * DISPLAY
     * Updates the display.
     *
     * @param display the display type
     */
    public void updateDisplay(int state) {
        log("updateDisplay :: state=" + state);
        if (isSwivelSupport() && state != mSwivelState) {
            mSwivelState = state;
            setDisplayOrientation(2, mPrevOrientation);
        }
    }

    /**
     * DISPLAY
     * Sets the size of the specified display.
     *
     * @param display the display type
     *          {@link ImsVideoCallProviderBase#DISPLAY_NEAR}
     *          {@link ImsVideoCallProviderBase#DISPLAY_FAR}
     * @param size the size of the specified display
     *          {@link ImsVideoCallProviderBase#DISPLAY_SIZE_NORMAL}
     *          {@link ImsVideoCallProviderBase#DISPLAY_SIZE_FULL}
     */
    public void setDisplaySize(int display, int size) {
        log("setDisplaySize :: display=" + display + ", size=" + size);

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.CHANGE_VIEW_SIZE_CMD);
        parcel.writeInt(display);
        parcel.writeInt(size);

        sendRequest(parcel);
    }

    /**
     * DISPLAY
     * Sets the orientation of the specified display.
     *
     * @param display the display type
     *          {@link ImsVideoCallProviderBase#DISPLAY_NEAR}
     *          {@link ImsVideoCallProviderBase#DISPLAY_FAR}
     *          {@link ImsVideoCallProviderBase#DISPLAY_NEAR_N_FAR}
     * @param orientation the orientation of the specified display
     *          {@link MtcMediaSession#ORIENTATION_0}
     *          {@link MtcMediaSession#ORIENTATION_90}
     *          {@link MtcMediaSession#ORIENTATION_270}
     *          {@link MtcMediaSession#ORIENTATION_180}
     */
    public void setDisplayOrientation(int display, int orientation) {
        log("setDisplayOrientation :: display=" + display +
            ", orientation=" + orientation + ", swivel=" + mSwivelState);
        mPrevOrientation = orientation;
        if (mSwivelState == STATE_SWIVELED) {
            orientation = getSwiveledOrientation(orientation);
        }
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcMedia.CHANGE_ORIENTATION_CMD);
        parcel.writeInt(display);
        parcel.writeInt(orientation);
        sendRequest(parcel);
    }

    /**
     * Releases the audio resources.
     */
    public void requestCallDataUsage() {
        log("requestCallDataUsage");

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.VIDEO_DATA_USAGE_CMD);

        sendRequest(parcel);
    }

    public void notifyMediaInfoChanged(int mediaInfo, int intParam, String strParam) {
        Listener listener = null;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener != null) {
            listener.onMediaSessionMediaInfoChanged(this, mediaInfo, intParam, strParam);
        }

        synchronized (mLock) {
            if ((mCall != null) && mCall.isConference()
                    && (mediaInfo == MtcCallUtils.INFO_TYPE_MEDIA_CVO_CAPABILITY)) {
                if (mMediaInfoEvent == null) {
                    log("CVO-Cap :: cache media-info event");
                    mMediaInfoEvent = new MediaInfoEvent(mediaInfo, intParam, strParam);
                } else if (mMediaInfoEvent.isValidEvent()) {
                    log("CVO-Cap :: cache media-info event parameters");
                    mMediaInfoEvent.setParams(intParam, strParam);
                }
            }
        }
    }

    public void onMessage(Parcel parcel) {
        int msg = parcel.readInt();

        if (isMessageForImsMediaManager(msg)) {
            parcel.setDataPosition(0);
            onImsMediaManagerMessage(parcel);
            return;
        }

        Listener listener = null;

        synchronized (mLock) {
            listener = mListener;
        }

        if (listener == null) {
            log("No listener: " + this);
            setOrClearMediaState(msg);

            if (msg == IUMtcMedia.ONSCREEN_DEBUG_INFO_VIDEO) {

                log(" listener is not set yet :: ");

                mVQINeedToBeReported = true;
            }

            return;
        }

        if ((msg == IUMtcMedia.FARFRAME_IND) && (mVQINeedToBeReported == true)) {

            int nVQIGap = 2; // BitRate >= 90%
            OnScreenDebugInfoVideo debugInfo = new OnScreenDebugInfoVideo(nVQIGap);
            listener.onMediaSessionDebugInfoChanged(this, debugInfo);

            mVQINeedToBeReported = false;

            log(" Video Quality Ind should be reported : vqi[ " + debugInfo.videoQualityIndicator + " ]");
        }

        switch (msg) {
        /**
         * Indication (IMS -> APP)
         */
        case IUMtcMedia.AUDIO_STARTED_IND:
        {
            setOrClearMediaState(msg);

            if (isCallSessionTerminated()) {
                break;
            }

            log("AUDIO_STARTED_IND :: ");

            listener.onMediaSessionAudioStarted(this);
            break;
        }

        case IUMtcMedia.AUDIO_STOPPED_IND:
        {
            setOrClearMediaState(msg);

            log("AUDIO_STOPPED_IND :: ");

            listener.onMediaSessionAudioStopped(this);
            break;
        }

        case IUMtcMedia.AUDIO_PAUSED_IND:
        {
            setOrClearMediaState(msg);

            if (isCallSessionTerminated()) {
                break;
            }

            log("AUDIO_PAUSED_IND :: ");

            listener.onMediaSessionAudioPaused(this);
            break;
        }

        case IUMtcMedia.MEDIA_STARTED_IND:
        {
            setOrClearMediaState(msg);

            if (isCallSessionTerminated()) {
                break;
            }

            log("MEDIA_STARTED_IND :: ");

            listener.onMediaSessionStarted(this);
            break;
        }

        case IUMtcMedia.ONSCREEN_DEBUG_INFO_VIDEO:
        {
            log("ONSCREEN_DEBUG_INFO_VIDEO :: ");

            OnScreenDebugInfoVideo debugInfo = new OnScreenDebugInfoVideo(parcel);
            debugInfo.logIn();
            listener.onMediaSessionDebugInfoChanged(this, debugInfo);
            break;
        }

        case IUMtcMedia.VIDEO_DATA_USAGE_INFO_IND:
        {
            long dataSize = parcel.readLong() + parcel.readLong();
            listener.onMediaSessionDataUsageChanged(this, dataSize);
            break;
        }

        case IUMtcMedia.FARFRAME_IND:
        {
            notifyCachedMediaInfoEvents();

            log("FARFRAME_IND :: ");

            listener.onMediaSessionPeerFirstVideoReceived(this);
            break;
        }

        case IUMtcMedia.FARFRAME_ORIENTATION_CHANGED:
        {
            int orientation = parcel.readInt();

            log("FARFRAME_ORIENTATION_CHANGED :: orientation=" + orientation);

            listener.onMediaSessionPeerDisplayOrientationChanged(this, orientation);
            break;
        }

        case IUMtcMedia.PEER_DIMENSION_CHANGED_IND:
        {
            int videoResolution = parcel.readInt();

            log("PEER RESOLUTION CHANGED: resolution=" + videoResolution);

            listener.onMediaSessionPeerDimensionsChanged(this, videoResolution);
            break;
        }

        case IUMtcMedia.UPDATE_VIDEO_SURFACE_IND:
        {
            log("UPDATE_VIDEO_SURFACE_IND :: ");

            listener.onMediaSessionSurfaceUpdateRequired(this);
            break;
        }

        /**
         * MULTI-TASKING
         */
        case IUMtcMedia.BACKGROUND_STARTED_IND:
        {
            int result = parcel.readInt();

            log("BACKGROUND_STARTED_IND :: result=" + result);

            listener.onMediaSessionStartBackgroundCompleted(this, result);
            break;
        }

        case IUMtcMedia.BACKGROUND_STOPPED_IND:
        {
            int result = parcel.readInt();

            log("BACKGROUND_STOPPED_IND :: result=" + result);

            listener.onMediaSessionStopBackgroundCompleted(this, result);
            break;
        }

        /**
         * VIDEO_CAPTURING
         */
        case IUMtcMedia.CAPTURED_IND:
        {
            if (isCallSessionTerminated()) {
                break;
            }

            int result = parcel.readInt();

            log("CAPTURED_IND :: result=" + result);

            listener.onMediaSessionCaptureVideoCompleted(this, result);
            break;
        }

        /**
         * ALTERNATIVE_IMAGE
         */
        case IUMtcMedia.ALTERNATE_IMAGE_STARTED_IND:
        {
            int result = parcel.readInt();

            log("ALTERNATE_IMAGE_STARTED_IND :: result=" + result);

            listener.onMediaSessionSetAlternativeImageCompleted(this, result);
            break;
        }

        case IUMtcMedia.ALTERNATE_IMAGE_STOPPED_IND:
        {
            int result = parcel.readInt();

            log("ALTERNATE_IMAGE_STOPPED_IND :: result=" + result);

            listener.onMediaSessionClearAlternativeImageCompleted(this, result);
            break;
        }

        /**
         * CAMERA
         */
        case IUMtcMedia.CAMERA_SELECTED_IND:
        {
            int result = parcel.readInt();

            log("CAMERA_SELECTED_IND :: result=" + result);

            listener.onMediaSessionSelectCameraCompleted(this, result);
            break;
        }

        case IUMtcMedia.CAMERA_ZOOM_CHANGED_IND:
        {
            int result = parcel.readInt();

            log("CAMERA_ZOOM_CHANGED_IND :: result=" + result);

            listener.onMediaSessionSetCameraZoomCompleted(this, result);
            break;
        }

        case IUMtcMedia.CAMERA_BRIGHTNESS_CHANGED_IND:
        {
            int result = parcel.readInt();

            log("CAMERA_BRIGHTNESS_CHANGED_IND :: result=" + result);

            listener.onMediaSessionSetCameraBrightnessCompleted(this, result);
            break;
        }

        /**
         * DISPLAY
         */
        case IUMtcMedia.DISPLAY_SWAPPED_IND:
        {
            int result = parcel.readInt();

            log("DISPLAY_SWAPPED_IND :: result=" + result);

            listener.onMediaSessionSwapDisplayCompleted(this, result);
            break;
        }

        case IUMtcMedia.DISPLAY_UPDATED_IND:
        {
            int result = parcel.readInt();

            log("DISPLAY_UPDATED_IND :: result=" + result);

            listener.onMediaSessionUpdateDisplayCompleted(this, result);
            break;
        }

        case IUMtcMedia.VIEW_SIZE_CHANGED_IND:
        {
            int result = parcel.readInt();

            log("VIEW_SIZE_CHANGED_IND :: result=" + result);

            listener.onMediaSessionSetDisplaySizeCompleted(this, result);
            break;
        }

        case IUMtcMedia.ORIENTATION_CHANGED_IND:
        {
            int result = parcel.readInt();

            log("ORIENTATION_CHANGED_IND :: result=" + result);

            listener.onMediaSessionSetDisplayOrientationCompleted(this, result);
            break;
        }

        /**
         * RECORDING
         */
        case IUMtcMedia.RECODING_STARTED_IND: // FALL-THROUGH
        case IUMtcMedia.RECODING_STOPPED_IND:
        {
            // no-op
            break;
        }

        case IUMtcMedia.CAMERA_FRAME_IND:
        {
            log("CAMERA_FRAME_IND");

            listener.onMediaSessionCameraFrameReceived(this);
            break;
        }

        case IUMtcMedia.RTT_TEXT_RECEIVED_IND:
        {
            String str = parcel.readString();

            log("RTT_TEXT_RECEIVED_IND :: text=" + str);


            if (mRttListener != null) {
                mRttListener.onRttMessageReceived(this, str);
            }
        }
        break;

        case IUMtcMedia.RTT_AUDIO_INDICATION_IND:
        {
            boolean status = (parcel.readInt()) > 0;

            log("RTT_AUDIO_INDICATION_IND :: status=" + status);


            if (mRttListener != null) {
                mRttListener.onRttAudioIndication(this, status);
            }
        }
        break;

        default:
            break;
        }
    }

    public static boolean isMessageForMediaSession(int msg) {
        return (((msg > IUMtcMedia.IMS_MSG_BASE_MEDIA)
                    && (msg < (IUMtcMedia.IMS_MSG_BASE_MEDIA + 100)))
                || (isMessageForImsMediaManager(msg)));
    }

    private void onImsMediaManagerMessage(Parcel parcel) {

        IMediaListener listener = null;

        synchronized (mLock) {
            listener = mMediaListener;
        }

        if (listener == null) {
            log("No listener: " + this);
            return;
        }

        listener.onMediaMessage(parcel);
    }

    private static boolean isMessageForImsMediaManager(int msg) {
        return (msg > MediaConstants.IMSMEDIA_REQUEST)
                && (msg < MediaConstants.IMSMEDIA_MAX);
    }

    private boolean isAudioPaused() {
        synchronized (mLock) {
            return (mState & STATE_AUDIO_PAUSED) != 0;
        }
    }

    private boolean isAudioStarted() {
        synchronized (mLock) {
            return (mState & STATE_AUDIO_STARTED) != 0;
        }
    }

    private boolean isCallSessionTerminated() {
        if (!isMediaSessionValid()) {
            return true;
        }

        synchronized (mLock) {
            return (mCall.isTerminated()
                    || (mCall.getCallState() == CallTracker.CALL_STATE_IDLE));
        }
    }

    private boolean isMediaSessionValid() {
        synchronized (mLock) {
            return mCall != null;
        }
    }

    private void notifyCachedMediaInfoEvents() {
        Listener listener = null;
        MediaInfoEvent event = null;

        synchronized (mLock) {
            listener = mListener;
            event = mMediaInfoEvent;
        }

        if ((listener != null) && (event != null)) {
            if (event.isValidEvent()) {
                log("notifyCachedMediaInfoEvents :: " + event);

                listener.onMediaSessionMediaInfoChanged(this,
                        event.getType(), event.getIntParam(), event.getStringParam());

                event.dispose();
            }
        }
    }

    private void clearState(int state) {
        synchronized (mLock) {
            mState &= (~state);
        }
    }

    private void setState(int state) {
        synchronized (mLock) {
            mState |= state;
        }
    }

    private void setOrClearMediaState(int msg) {
        if (msg == IUMtcMedia.AUDIO_STARTED_IND) {
            setState(STATE_AUDIO_STARTED);
            clearState(STATE_AUDIO_PAUSED);
        } else if (msg == IUMtcMedia.AUDIO_STOPPED_IND) {
            clearState(STATE_AUDIO_STARTED);
            clearState(STATE_AUDIO_PAUSED);
            clearState(STATE_MEDIA_STARTED);
        } else if (msg == IUMtcMedia.AUDIO_PAUSED_IND) {
            setState(STATE_AUDIO_PAUSED);
        } else if (msg == IUMtcMedia.MEDIA_STARTED_IND) {
            setState(STATE_MEDIA_STARTED);
        }
    }

    public void sendRttMessage(String data) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IUMtcMedia.RTT_TEXT_SEND_CMD);
        parcel.writeString(data);

        sendRequest(parcel);
    }

    public void sendRequest(Parcel parcel) {
        long sessionId = 0;

        synchronized (mLock) {
            sessionId = (mCall != null) ? mCall.getNativeCallId() : 0;
        }

        if (sessionId == 0) {
            parcel.recycle();
            parcel = null;

            ImsLog.e("[GII-MTC] Media session is already closed");
            return;
        }

        byte[] data = parcel.marshall();
        parcel.recycle();
        parcel = null;

        JNIIms.sendData(sessionId, data);
    }

    private static void log(String s) {
        ImsLog.d("[GII-MTC] " + s);
    }

    private int getSwiveledOrientation(int nOrientation)
    {
        switch (nOrientation) {
            default:
            case ORIENTATION_0:
                return ORIENTATION_90;
            case ORIENTATION_90:
                return ORIENTATION_180;
            case ORIENTATION_270:
                return ORIENTATION_0;
            case ORIENTATION_180:
                return ORIENTATION_270;
        }
    }
}
