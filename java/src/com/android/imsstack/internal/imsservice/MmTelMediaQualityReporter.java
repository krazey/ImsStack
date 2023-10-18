/*
 * Copyright (C) 2023 The Android Open Source Project
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

package com.android.imsstack.internal.imsservice;

import android.annotation.NonNull;
import android.telephony.AccessNetworkConstants.TransportType;
import android.telephony.ims.MediaQualityStatus;
import android.telephony.ims.feature.MmTelFeature;

import com.android.imsstack.util.ImsLog;

/**
 * This class is to get access to {@link MmTelMediaRegistry} and notifies the media quality
 * status changed when session meets the {@link MediaThreshold} criteria.
 */
public class MmTelMediaQualityReporter {
    private final MmTelMediaRegistry mMediaRegistry;
    private final MmTelFeature mMmTelFeature;
    private final String mCallId;

    MmTelMediaQualityReporter(@NonNull MmTelMediaRegistry mediaRegistry,
            @NonNull MmTelFeature mmTelFeature, @NonNull String callId) {
        mMediaRegistry = mediaRegistry;
        mMmTelFeature = mmTelFeature;
        mCallId = callId;
    }

    /**
     * Returns the {@link MmTelMediaRegistry} to access the MmTel media state
     * values from Media module.
     *
     * @return {@link MmTelMediaRegistry}
     */
    @NonNull
    public MmTelMediaRegistry getMediaRegistry() {
        return mMediaRegistry;
    }

    /**
     * Notifies the media quality status change to framework.
     *
     * @param mediaSessionType media session type of this quality status
     * @param transportType transport type of this quality status
     * @param rtpPacketLossRate measured RTP packet loss rate in percentage
     * @param rtpJitterMillis measured RTP jitter(RFC3550) in milliseconds
     * @param rptInactivityTimeMillis measured RTP inactivity time in milliseconds
     */
    public void notifyMediaQualityStatusChanged(
            @MediaQualityStatus.MediaSessionType int mediaSessionType,
            @TransportType int transportType, int rtpPacketLossRate, int rtpJitterMillis,
            long rptInactivityTimeMillis) {
        MediaQualityStatus status = new MediaQualityStatus(
                mCallId, mediaSessionType, transportType,
                rtpPacketLossRate, rtpJitterMillis, rptInactivityTimeMillis);

        try {
            mMmTelFeature.notifyMediaQualityStatusChanged(status);
        } catch (RuntimeException e) {
            ImsLog.e("notifyMediaQualityStatusChanged ::" + e.toString());
        }
    }
}
