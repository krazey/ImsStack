/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.util;

import static org.junit.Assert.fail;

import com.android.imsstack.util.Log;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * A wrapper class for a single {@link CountDownLatch}.
 */
public class SingleLatch {
    /**
     * The timeout values for waiting for a specific event.
     */
    public static final int SHORT_TIMEOUT_MS = 5 * 1000; // 5s
    public static final int MEDIUM_TIMEOUT_MS = 10 * 1000; // 10s
    public static final int LONG_TIMEOUT_MS = 15 * 1000; // 15s
    /**
     * The timeout values for sleep.
     */
    public static final int SHORT_SLEEP_MS = 1 * 1000; // 1s
    public static final int LONG_SLEEP_MS = 3 * 1000; // 3s

    private final String mTag;
    private CountDownLatch mLatch;

    /**
     * Holds the current thread for the given timeout value.
     * This method uses an independent {@link CountDownLatch} for the delay.
     *
     * @param millis The timeout value to be held.
     */
    public static void delay(long millis) {
        try {
            new CountDownLatch(1).await(millis, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            Log.d(SingleLatch.class, "delay interrupted: " + e.toString());
        }
    }

    public SingleLatch(String tag) {
        mTag = tag;
        init();
    }

    /**
     * Initializes a new single {@link CountDownLatch}.
     */
    public final void init() {
        if (mLatch != null && mLatch.getCount() == 1) {
            return;
        }
        mLatch = new CountDownLatch(1);
    }

    /**
     * Waits for the event completion with a default timeout value.
     */
    public void await() {
        await(MEDIUM_TIMEOUT_MS);
    }

    /**
     * Waits for the event completion with the given timeout value.
     * Fails the associated test if the event does not complete within the timeout period.
     *
     * @param millis The timeout value for waiting.
     */
    public void await(long millis) {
        boolean completed = false;

        try {
            if (mLatch != null) {
                completed = mLatch.await(millis, TimeUnit.MILLISECONDS);
            }
        } catch (InterruptedException e) {
            Log.d(this, "await interrupted: " + e.toString());
        }

        if (!completed) {
            fail("Failed to wait for " + mTag + " for " + millis + "ms.");
        }
    }

    /**
     * Waits for the timeout with the given timeout value.
     * Fails the associated test if the event completes within the timeout period.
     *
     * @param millis The timeout value for waiting.
     */
    public void awaitTimeout(long millis) {
        boolean completed = false;

        try {
            if (mLatch != null) {
                completed = mLatch.await(millis, TimeUnit.MILLISECONDS);
            }
        } catch (InterruptedException e) {
            Log.d(this, "await interrupted: " + e.toString());
        }

        if (completed) {
            fail("Event " + mTag + " interrupted unexpectedly within " + millis + "ms.");
        }
    }

    /**
     * Sends a signal to wake up all the waiting threads.
     */
    public void countDown() {
        if (mLatch != null) {
            mLatch.countDown();
        }
    }

    /**
     * Sends a signal to wake up all the waiting threads and reinitializes the latch.
     */
    public void countDownAndInit() {
        if (mLatch != null) {
            mLatch.countDown();
        }
        init();
    }

    /**
     * Holds the current thread for the given timeout value.
     *
     * @param millis The timeout value to be held.
     */
    public void sleep(long millis) {
        try {
            if (mLatch != null) {
                mLatch.await(millis, TimeUnit.MILLISECONDS);
            }
        } catch (InterruptedException e) {
            Log.d(this, "sleep interrupted: " + e.toString());
        }

        init();
    }
}
