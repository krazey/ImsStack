package com.android.imsstack.imsservice.mmtel.base;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.IECallStateTracker;

public interface ICallContext extends IBaseContext {
    public static final int MEDIA_AUDIO = 1;
    public static final int MEDIA_VIDEO = 2;

    /**
     * Returns ImsApp instance.
     */
    public ImsApp getApp();

    /**
     * Returns call location policy to contain the device's location information
     * when making a call.
     * @Nullable
     */
    public ICallLocationPolicy getCallLocationPolicy();

    /**
     * Returns emergency call state tracker if it's required for this call context.
     * @Nullable
     */
    public IECallStateTracker getECallStateTracker();

    /**
     * Returns SRVCC state tracker if it's required for this call context.
     * @Nullable
     */
    public ISrvccStateTracker getSrvccStateTracker();

    /**
     * Return TTY mode tracker if it's required for this call context.
     * @Nullable
     */
    public TtyModeTracker getTtyModeTracker();

    /**
     * Returns the media capabilities based on the type.
     *
     * @param callType the call type (ImsCallProfile#CALL_TYPE_XXX)
     * @param mediaType the media type (ImsApp#MEDIA_AUDIO, ImsApp#MEDIA_VIDEO)
     * @return the best media quality
     * @see
     *  ImsStreamMediaProfile#AUDIO_XXX
     *  ImsStreamMediaProfile#VIDEO_XXX
     */
    public int getMediaCapabilities(int callType, int mediaType);

    /**
     * Returns MtcCall object if present.
     */
    public Object getMtcCall(long callId);

    /**
     * Checks if location information is required when making a call.
     */
    public boolean isLocationRequiredForCall();
}
