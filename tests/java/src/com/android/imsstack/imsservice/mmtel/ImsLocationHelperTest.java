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

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import android.location.Location;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.Log;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class ImsLocationHelperTest {
    private static ContextFixture sContext;
    private ImsLocationHelper mLocHelper = null;
    private static final String TAG = ImsLocationHelperTest.class.getName();

    private HandlerThread mTestThread = new HandlerThread("TestThread");
    private Handler mTestHandler;

    private CountDownLatch mLatch;

    private static final int VALIDITY_PERIOD = 10000;
    private static final int WAITING_TIME = 2000;
    private static final int LOCATION_UPDATE_GUARD_TIMEOUT = 3000;
    private static final int MAX_LATCH_TIME = 3000;
    private static final int EVENT_RELEASE_LATCH = 20;

    private class LocationBasedCallTest implements ImsLocationHelper.Listener {

        @Override
        public void onLocationUpdateTimeout() {
            Log.i(TAG, "Timeout received in LocationBasedCallTest");
            onLocationUpdated();
        }

        @Override
        public void onLocationUpdated() {
            Log.i(TAG, "Location update received in LocationBasedCallTest");
            mLatch.countDown();
        }
    }

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture();
        AppContext.init(sContext.getTestDouble());
    }

    @Before
    public void setUp() {

        MockitoAnnotations.initMocks(this);
        mTestThread.start();

        /* handler bound to this looper */
        mTestHandler = new Handler(mTestThread.getLooper()) {
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case EVENT_RELEASE_LATCH:
                        if (mLatch != null) {
                            Log.i(TAG, "countdown mLatch expiry");
                            mLatch.countDown();
                        }
                        break;
                }
            }
        };
    }

    @After
    public void tearDown() {
        mLocHelper = null;
        if (mTestThread != null) {
            mTestThread.quit();
            mTestThread = null;
        }

        if (mLatch != null) {
            mLatch.countDown();
            mLatch = null;
        }

        mTestHandler = null;
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
    }

    @Test
    public void testCreationOfLocationHelper() throws InterruptedException {

        ImsCallContext callContext = Mockito.mock(ImsCallContext.class);

        LocationBasedCallTest listener = new LocationBasedCallTest();
        mLocHelper = new ImsLocationHelper(callContext, listener, mTestThread.getLooper());
        assertNotNull(mLocHelper);

        mLocHelper.setValidityOption(true, VALIDITY_PERIOD);

        mLatch = new CountDownLatch(1);
        mLocHelper.startLocationUpdates(WAITING_TIME);

        Log.i(TAG, "Waiting for the location update for 2secs");

        mTestHandler.sendEmptyMessageDelayed(EVENT_RELEASE_LATCH, LOCATION_UPDATE_GUARD_TIMEOUT);

        assertTrue(mLatch.await(MAX_LATCH_TIME, TimeUnit.SECONDS));

        Log.i(TAG, "Out of Latch");

        Location location = mLocHelper.getCurrentLocation();
        if (location != null) {
            assertTrue(location.isComplete());
        }

        location = mLocHelper.getLastKnownLocationInfo();
        if (location != null) {
            assertTrue(location.isComplete());
        }

        location = mLocHelper.getCachedLocation();
        if (location != null) {
            assertTrue(location.isComplete());
        }

        // Call dispose to stop location updates
        mLocHelper.dispose();
    }
}
