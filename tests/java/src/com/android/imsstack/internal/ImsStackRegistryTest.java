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
package com.android.imsstack.internal;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.test.suitebuilder.annotation.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsStackRegistryTest {
    private static final int SLOT0 = 0;

    @Mock ImsStackRegistry.ImsServiceListener mImsServiceListener;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        ImsStackRegistry.setImsServiceState(SLOT0, false);
        ImsStackRegistry.addImsServiceListener(mImsServiceListener);
    }

    @After
    public void tearDown() throws Exception {
        ImsStackRegistry.setImsServiceState(SLOT0, false);
        ImsStackRegistry.removeImsServiceListener(mImsServiceListener);
    }

    @Test
    @SmallTest
    public void testSetImsServiceState() {
        assertFalse(ImsStackRegistry.isImsServiceStarted(SLOT0));

        ImsStackRegistry.setImsServiceState(SLOT0, true);

        assertTrue(ImsStackRegistry.isImsServiceStarted(SLOT0));
        verify(mImsServiceListener).onImsServiceStarted(eq(SLOT0));

        ImsStackRegistry.setImsServiceState(SLOT0, false);

        assertFalse(ImsStackRegistry.isImsServiceStarted(SLOT0));
        verify(mImsServiceListener).onImsServiceStopped(eq(SLOT0));

        ImsStackRegistry.setImsServiceState(SLOT0, false);

        verifyNoMoreInteractions(mImsServiceListener);
    }
}
