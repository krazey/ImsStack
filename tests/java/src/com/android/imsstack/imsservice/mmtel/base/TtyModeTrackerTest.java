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

package com.android.imsstack.imsservice.mmtel;

import static org.junit.Assert.assertEquals;

import android.telecom.TelecomManager;
import android.util.Log;

import com.android.imsstack.imsservice.mmtel.base.TtyModeTracker;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class TtyModeTrackerTest {
    private static final String LOG_TAG = "TtyModeTrackerTest";

    private TtyModeTracker mTtyModeTracker;
    private TtyModeTracker.Listener mListener;
    private int mTtyModeChanged = TelecomManager.TTY_MODE_OFF;

    @Before
    public void setUp() throws Exception {
        mTtyModeTracker = new TtyModeTracker();
        mListener = new TtyListener();
        mTtyModeTracker.addListener(mListener);
    }

    @After
    public void tearDown() throws Exception {
        mTtyModeTracker = null;
        mListener = null;
    }

    @Test
    public void testAddListener() {
        mTtyModeTracker.setTtyMode(TelecomManager.TTY_MODE_FULL);
        sleep(500);
        assertEquals(TelecomManager.TTY_MODE_FULL, mTtyModeChanged);
        mTtyModeTracker.setTtyMode(TelecomManager.TTY_MODE_OFF);
        sleep(500);
        assertEquals(TelecomManager.TTY_MODE_OFF, mTtyModeChanged);
    }

    @Test
    public void testGetTtyMode() {
        assertEquals(mTtyModeTracker.getTtyMode(), TelecomManager.TTY_MODE_OFF);
        mTtyModeTracker.setTtyMode(TelecomManager.TTY_MODE_FULL);
        assertEquals(mTtyModeTracker.getTtyMode(), TelecomManager.TTY_MODE_FULL);
    }

    @Test
    public void testRemoveListener() {
        mTtyModeTracker.setTtyMode(TelecomManager.TTY_MODE_FULL);
        sleep(500);
        assertEquals(TelecomManager.TTY_MODE_FULL, mTtyModeChanged);
        mTtyModeTracker.removeListener(mListener);
        mTtyModeTracker.setTtyMode(TelecomManager.TTY_MODE_OFF);
        sleep(500);
        assertEquals(TelecomManager.TTY_MODE_FULL, mTtyModeChanged);
    }

    private class TtyListener implements TtyModeTracker.Listener {
        public void onTtyModeChanged(int newTtyMode) {
            mTtyModeChanged = newTtyMode;
            Log.d(LOG_TAG, "onTtyModeChanged " + mTtyModeChanged);
        }
    }

    private void sleep(long ms) {
        try {
            Thread.sleep(ms);
        } catch (Exception e) {
            Log.d(LOG_TAG, "InterruptedException");
        }
    }
}
