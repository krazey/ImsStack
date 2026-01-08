/*
 * Copyright (C) 2025 The Android Open Source Project
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

import android.media.AudioManager;
import android.media.ToneGenerator;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/**
 * Manages the lifecycle and playback of DTMF tones.
 * This class encapsulates the ToneGenerator to ensure it is created only once
 * and handles initialization failures gracefully.
 */
public class DtmfToneGenerator {
    private ToneGenerator mToneGenerator;
    private boolean mIsToneGeneratorCreationFailed = false;
    private final IBaseContext mContext;

    public DtmfToneGenerator(IBaseContext context) {
        mContext = context;
    }

    /**
     * Plays a DTMF tone for a specified duration.
     *
     * @param dtmfTone The DTMF tone to play, as defined in {@link ToneGenerator}.
     * @param durationMs The duration to play the tone in milliseconds.
     */
    public void play(int dtmfTone, int durationMs) {
        if (mIsToneGeneratorCreationFailed) {
            // Do not try to create the tone generator again if it has already failed.
            ImsLog.d(this, "ToneGeneration has failed, skipping playback.");
            return;
        }

        try {
            if (mToneGenerator == null) {
                ImsLog.d(this, "Creating ToneGenerator instance.");
                mToneGenerator = createToneGenerator();
            }

            int toneDuration = (durationMs > 0) ? durationMs : 300;
            ImsLog.d(this, "startTone: tone=" + dtmfTone + ", duration=" + toneDuration);
            mToneGenerator.startTone(dtmfTone, toneDuration);

        } catch (RuntimeException e) {
            ImsLog.e(this, mContext.getSlotId(), "Exception caught while playing tone: " + e);
            mIsToneGeneratorCreationFailed = true;
            if (mToneGenerator != null) {
                mToneGenerator.release();
                mToneGenerator = null;
            }
        }
    }

    /**
     * Releases the ToneGenerator resources and resets the failure state.
     * This should be called when the audio session is closed.
     */
    public void release() {
        if (mToneGenerator != null) {
            mToneGenerator.release();
            mToneGenerator = null;
        }
        mIsToneGeneratorCreationFailed = false;
    }

    /**
     * Create Tone instance
     */
    @VisibleForTesting
    public ToneGenerator createToneGenerator() {
        return new ToneGenerator(AudioManager.STREAM_VOICE_CALL, 80);
    }

    /**
     * Returns true if the tone generator creation has failed.
     */
    @VisibleForTesting
    public boolean isToneGeneratorCreationFailed() {
        return mIsToneGeneratorCreationFailed;
    }
}
