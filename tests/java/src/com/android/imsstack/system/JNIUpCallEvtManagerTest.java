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

package com.android.imsstack.system;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import android.test.suitebuilder.annotation.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class JNIUpCallEvtManagerTest {

    private static final int SLOT_0 = 0;
    private JNIUpCallEvtManager mJNIUpCallEvtManager;

    @Before
    public void setUp() throws Exception {
        mJNIUpCallEvtManager = new JNIUpCallEvtManager().getInstance();
    }

    @After
    public void tearDown() throws Exception {
        mJNIUpCallEvtManager = null;
    }

    @Test
    @SmallTest
    public void start() {
        mJNIUpCallEvtManager.start(SLOT_0);
        assertNotNull(mJNIUpCallEvtManager.getJNIUpCallEvt(SLOT_0));

        mJNIUpCallEvtManager.stop(SLOT_0);
        assertNull(mJNIUpCallEvtManager.getJNIUpCallEvt(SLOT_0));
    }

    @Test
    @SmallTest
    public void processEvent() {
        mJNIUpCallEvtManager.start(SLOT_0);
        ISystemAPISendEvent jniEvt = (ISystemAPISendEvent) mJNIUpCallEvtManager.getJNIUpCallEvt(
                SLOT_0);
        assertNotNull(jniEvt);
        assertEquals(0, jniEvt.processEvent4Sys(ImsEventDef.IMS_EVENT_WAKE_LOCK, 1, 1));
        assertEquals(1, jniEvt.processEvent4Sys(ImsEventDef.IMS_EVENT_NATIVE_BOOT_COMPLETED,
                1, 1));

        IJNIUpCallEvt callEvt = (IJNIUpCallEvt) mJNIUpCallEvtManager.getJNIUpCallEvt(SLOT_0);
        assertNotNull(callEvt);
        assertEquals(SLOT_0, callEvt.getSlotId());

        mJNIUpCallEvtManager.stop(SLOT_0);
        assertNull(mJNIUpCallEvtManager.getJNIUpCallEvt(SLOT_0));
    }
}
