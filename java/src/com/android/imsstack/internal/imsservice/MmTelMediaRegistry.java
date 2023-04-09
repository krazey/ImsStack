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

import android.annotation.Nullable;
import android.telephony.ims.MediaQualityStatus;
import android.telephony.ims.MediaThreshold;
import android.util.SparseArray;

import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * A class holds the information related to MmTel media like {@link MediaThreshold}
 * which can be accessed by Media module.
 * Using this registry class, Media module can access and get notified the MmTel media information
 * which is set/modified by Telephony framework.
 */
public class MmTelMediaRegistry {
    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();
    private final SparseArray<MediaThreshold> mMediaThresholds = new SparseArray<>(2);

    /**
     * Notifies the components who monitor the MmTel media state values change.
     */
    public interface Listener {
        /**
         * Notifies the components who monitor the {@link MediaThreshold} that it has changed.
         */
        default void onMediaThresholdChanged(
                @MediaQualityStatus.MediaSessionType int mediaSessionType,
                MediaThreshold mediaThreshold) {
        }
    }

    /**
     * Sets the {@link MediaThreshold} for session to monitor media quality status.
     *
     * @param mediaSessionType media session type for this threshold info
     * @param mediaThreshold media threshold information
     */
    public void setMediaThreshold(@MediaQualityStatus.MediaSessionType int mediaSessionType,
            MediaThreshold mediaThreshold) {
        MediaThreshold threshold =  mMediaThresholds.get(mediaSessionType);

        if (!Objects.equals(threshold, mediaThreshold)) {
            mMediaThresholds.put(mediaSessionType, mediaThreshold);
            notifyMediaThresholdChanged(mediaSessionType, mediaThreshold);
        }
    }

    /**
     * Returns the {@link MediaThreshold} to access the media threshold value.
     *
     * @param mediaSessionType media session type of this threshold info
     * @return {@link MediaThreshold} based on media session type
     */
    @Nullable
    public MediaThreshold getMediaThreshold(
            @MediaQualityStatus.MediaSessionType int mediaSessionType) {
        return mMediaThresholds.get(mediaSessionType);
    }

    /**
     * Adds the listener to monitor MmTel media state change.
     *
     * @param listener The listener to be added.
     */
    public void addListener(Listener listener) {
        mListeners.add(listener);
    }

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     */
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    private void notifyMediaThresholdChanged(
            @MediaQualityStatus.MediaSessionType int mediaSessionType,
            MediaThreshold mediaThreshold) {
        for (Listener l : mListeners) {
            l.onMediaThresholdChanged(mediaSessionType, mediaThreshold);
        }
    }
}
