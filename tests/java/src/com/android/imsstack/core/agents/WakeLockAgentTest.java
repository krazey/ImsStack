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
package com.android.imsstack.core.agents;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import android.os.PowerManager;
import android.os.SystemClock;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.TestAppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class WakeLockAgentTest {
    private static final int WAKE_LOCK_TIMEOUT = 200; // 200ms

    private ContextFixture mContextFixture;
    private TestableLooper mTestableLooper;
    private TestAppContext mTestAppContext;
    private WakeLockAgent mWakeLockAgent;

    @Before
    public void setUp() throws Exception {
        mTestableLooper = TestableLooper.get(this);
        mContextFixture = new ContextFixture();
        mTestAppContext = new TestAppContext(mContextFixture.getTestDouble());
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());

        mWakeLockAgent = new WakeLockAgent();
        mWakeLockAgent.init(mTestAppContext.getContext());
    }

    @After
    public void tearDown() throws Exception {
        if (mWakeLockAgent != null) {
            mWakeLockAgent.cleanup();
            mWakeLockAgent = null;
        }

        mTestAppContext.tearDown();
        mTestAppContext = null;
        mContextFixture = null;
        mTestableLooper = null;
    }

    @Test
    @SmallTest
    public void testAcquire() {
        mWakeLockAgent.acquire(WAKE_LOCK_TIMEOUT);

        PowerManager.WakeLock wakeLock = mWakeLockAgent.getWakeLock();

        assertNotNull(wakeLock);
        assertTrue(wakeLock.isHeld());

        SystemClock.sleep(WAKE_LOCK_TIMEOUT + 10);

        assertFalse(wakeLock.isHeld());
    }

    @Test
    @SmallTest
    public void testAcquireForNative() {
        mWakeLockAgent.acquireForNative(WAKE_LOCK_TIMEOUT);
        processAllMessages();

        PowerManager.WakeLock wakeLock = mWakeLockAgent.getWakeLock();

        assertNotNull(wakeLock);
        assertTrue(wakeLock.isHeld());

        SystemClock.sleep(WAKE_LOCK_TIMEOUT + 10);

        assertFalse(wakeLock.isHeld());
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
