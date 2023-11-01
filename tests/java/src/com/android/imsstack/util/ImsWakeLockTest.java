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
package com.android.imsstack.util;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.os.PowerManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class ImsWakeLockTest {
    private ContextFixture mContextFixture;
    private boolean mDebuggable;
    private ImsWakeLock mWakeLock;

    @Before
    public void setUp() throws Exception {
        mContextFixture = new ContextFixture();
        mDebuggable = ImsLog.isDebuggable();
        ImsLog.setDebugOn(true);

        mWakeLock = new ImsWakeLock(
                mContextFixture.getTestDouble().getSystemService(PowerManager.class),
                ImsWakeLockTest.class.getSimpleName());
    }

    @After
    public void tearDown() throws Exception {
        if (mWakeLock != null) {
            mWakeLock.clear();
            mWakeLock = null;
        }

        ImsLog.setDebugOn(mDebuggable);
        mContextFixture = null;
    }

    @Test
    @SmallTest
    public void testAcquireAndRelease() {
        assertFalse(mWakeLock.isHeld());

        mWakeLock.acquire(this);

        assertTrue(mWakeLock.isHeld());

        mWakeLock.release(this);

        assertFalse(mWakeLock.isHeld());
    }

    @Test
    @SmallTest
    public void testAcquireAndClear() {
        assertFalse(mWakeLock.isHeld());

        mWakeLock.acquire(this);

        assertTrue(mWakeLock.isHeld());

        mWakeLock.clear();

        assertFalse(mWakeLock.isHeld());
    }
}
