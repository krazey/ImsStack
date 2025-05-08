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

package com.android.imsstack.imsservice.mmtel.call;

import android.telecom.VideoProfile;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsVideoCallProvider;

import com.android.imsstack.imsservice.mmtel.base.ICallContext;

public interface IVideoCallSession {
    public static final int MODIFICATION_NONE = 0;
    public static final int MODIFICATION_CALL_TYPE = 1;
    public static final int MODIFICATION_VIDEO_PROFILE = 2;

    public static final int EVENT_CALL_IDLE = 0;
    // Originating call is started
    public static final int EVENT_CALL_INITIATING = 1;
    // Incoming call is notified to framework and user alert will be done soon.
    public static final int EVENT_CALL_ALERTING = 2;
    // Call establishment is completed
    public static final int EVENT_CALL_ESTABLISHED = 3;
    // Call is terminated
    public static final int EVENT_CALL_TERMINATED = 4;

    /**
     * Parameter values to turn on/off the camera.
     */
    public final static int CAMERA_OFF = 0;
    public final static int CAMERA_ON = 1;

    /*
     * Multitasking state
     *    NONE: Not a multitasking state; Call screen is on the foreground
     *    ACTIVATED: Multitasking is activated; Call screen is on the background
     */
    public final static int MULTITASKING_NONE = 0;
    public final static int MULTITASKING_ACTIVATED = 1;

    public static interface EventListener {
        public void onCallEvent(int event);
        public void onSessionModificationAbortedByCameraOff();
    }

    public ICallContext getCallContext();
    public int getCallType();
    public int getSessionModificationType();
    public ImsStreamMediaProfile getProposedStreamMediaProfile();
    public ImsStreamMediaProfile getStreamMediaProfile();
    public boolean isMoCall();
    public boolean isVrbtEnabled();
    public void sendSessionModifyRequest(VideoProfile fromProfile, VideoProfile toProfile);
    public void sendSessionModifyResponse(VideoProfile responseProfile);
    public void setCameraSetting(int setting);
    public void setMultitaskingState(int state);
    public void setVideoCallProvider(ImsVideoCallProvider provider);
    public void setEventListener(EventListener listener);
    /**
     * Sets Camera ID to handle multitasking.
     */
    void onSetCamera(String cameraId);
}
