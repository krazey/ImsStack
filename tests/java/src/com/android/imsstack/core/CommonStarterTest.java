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

package com.android.imsstack.core;


import static com.android.imsstack.core.CommonStarter.STATE_IDLE;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.jni.JniIms;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class CommonStarterTest {
    private static final int SLOT_ID = 0;
    private CommonStarter mCommonStarter;
    private ContextFixture mContextFixture;

    @Before
    public void setUp() throws Exception {
        mContextFixture = new ContextFixture();
        AppContext.init(mContextFixture.getTestDouble());
        JniIms jniIms = Mockito.mock(JniIms.class);
        JniImsProxy.setJniIms(jniIms);
        mCommonStarter = CommonStarter.getInstance();
    }

    @After
    public void tearDown() throws Exception {
        JniImsProxy.setJniIms(null);
        AppContext.deinit();
        mCommonStarter = null;
        mContextFixture = null;
    }

    @Test
    @SmallTest
    public void getState() throws Exception {
        assertEquals(STATE_IDLE, mCommonStarter.getState(-1));
        assertEquals(STATE_IDLE, mCommonStarter.getState(SLOT_ID));
    }

    @Test
    @SmallTest
    public void isCommonAgentReady() {
        assertFalse(mCommonStarter.isCommonAgentReady());
        mCommonStarter.setCommonAgentCompleted();
        assertTrue(mCommonStarter.isCommonAgentReady());
    }

    @Test
    @SmallTest
    public void createJNI() {
        assertFalse(mCommonStarter.isJNIReady());
        mCommonStarter.createJNI();
        assertTrue(mCommonStarter.isJNIReady());
    }

    @Test
    @SmallTest
    public void addVolteListener() {
        // verify notifyVoltePackageReady
        IVoltePackageListener listener = Mockito.mock(IVoltePackageListener.class);
        mCommonStarter.addVolteListener(listener);
        mCommonStarter.notifyVoltePackageReady(SLOT_ID);
        verify(listener).onVoltePackageReady(SLOT_ID);

        mCommonStarter.removeVolteListener(listener);
        mCommonStarter.notifyVoltePackageReady(SLOT_ID);
        verifyNoMoreInteractions(listener);

        mCommonStarter.addVolteListener(null);
        mCommonStarter.notifyVoltePackageReady(SLOT_ID);
        verifyNoMoreInteractions(listener);
    }

    @Test
    @SmallTest
    public void notifyPackageStop() {
        // verify notifyPackageStop
        ICommonPackageListener listener = Mockito.mock(ICommonPackageListener.class);
        mCommonStarter.addListener(listener);
        mCommonStarter.stopAgents(SLOT_ID);
        verify(listener).onCommonPackageStop(SLOT_ID);

        mCommonStarter.removeListener(listener);
        mCommonStarter.notifyVoltePackageReady(SLOT_ID);
        verifyNoMoreInteractions(listener);

        mCommonStarter.addListener(null);
        mCommonStarter.stopAgents(SLOT_ID);
        verifyNoMoreInteractions(listener);
    }
}
