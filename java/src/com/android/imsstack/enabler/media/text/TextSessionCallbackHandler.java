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

package com.android.imsstack.enabler.media;

import android.annotation.NonNull;
import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.TextConfig;

import com.android.imsstack.enabler.mtc.IMtcMediaInterface;
import com.android.imsstack.util.ImsLog;

/**
 * This handles callbacks received from {@link ImsMediaManager} and passes it to
 * ImsStack (Media Enabler Native)
 */
public class TextSessionCallbackHandler {

    private final IMtcMediaInterface mMtcMediaInterface;

    public TextSessionCallbackHandler(@NonNull final IMtcMediaInterface mtcMediaInterface) {
        mMtcMediaInterface = mtcMediaInterface;
        ImsLog.v("Constructor - Exit");
    }

    private IMtcMediaInterface getMtcMediaInterface() {
        return mMtcMediaInterface;
    }

    /**
     * Handles the response for open session request
     *
     * @param result result of the open session request
     */
    public void openSessionResponse(@ImsMediaSession.SessionOperationResult int result) {
        ImsLog.d("openSession Result=" + result);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.RESPONSE_OPEN_SESSION);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        parcel.writeInt(result);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles response when any change occurs to the RTP session
     *
     * @param state session state
     */
    public void sessionChanged(int state) {
        ImsLog.v("sessionChanged");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.RESPONSE_SESSION_CHANGED);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        parcel.writeInt(state);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles response for Modify Session request
     *
     * @param textConfig The TextConfig passed in ImsMediaSession#modifySession()
     * @param result The result of modify session
     */
    public void modifySessionResponse(TextConfig textConfig, int result) {
        ImsLog.v("modifySessionResponse");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.RESPONSE_MODIFY_SESSION);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        textConfig.writeToParcel(parcel, Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
        parcel.writeInt(result);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles text stream received request
     *
     * @param text The text string received from the network
     */
    public void onRttReceived(@NonNull String text) {
        ImsLog.v("onRttReceived");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.NOTIFY_RTT_RECEIVED);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        if (text == null) {
            text = "";
        }
        parcel.writeString(text);

        getMtcMediaInterface().sendRequest(parcel);
    }

    /**
     * Handles notification when media inactivity observed as per thresholds set by
     * setMediaQualityThreshold()
     *
     * @param packetType either RTP or RTCP
     */
    public void onNotifyMediaInactivity(int packetType) {
        ImsLog.v("onNotifyMediaInactivity");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(MediaConstants.NOTIFY_MEDIA_INACTIVITY);
        parcel.writeInt(ImsMediaSession.SESSION_TYPE_RTT);
        parcel.writeInt(packetType);

        getMtcMediaInterface().sendRequest(parcel);
    }
}
