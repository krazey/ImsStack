/*
 * Copyright (C) 2026 The Android Open Source Project
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

package com.android.imsstack.internal.enabler;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SLOT1;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNotSame;
import static org.junit.Assert.assertSame;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import androidx.test.ext.junit.runners.AndroidJUnit4;

import com.android.imsstack.enabler.mtc.Call;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

@RunWith(AndroidJUnit4.class)
public class MtcCallRegistryTest {

    @Rule public final MockitoRule mocks = MockitoJUnit.rule();

    @Mock private MtcCallRegistry.Listener mMockListener1;
    @Mock private MtcCallRegistry.Listener mMockListener2;
    @Mock private Call mMockCall;

    private MtcCallRegistry mMtcCallRegistry0;
    private MtcCallRegistry mMtcCallRegistry1;

    @Before
    public void setUp() {
        mMtcCallRegistry0 = MtcCallRegistry.getInstance(SLOT0);
        mMtcCallRegistry1 = MtcCallRegistry.getInstance(SLOT1);
        reset(mMockListener1, mMockListener2);
    }

    @After
    public void tearDown() {
        if (mMtcCallRegistry0 != null) {
            mMtcCallRegistry0.removeListener(mMockListener1);
            mMtcCallRegistry0.removeListener(mMockListener2);
        }
        if (mMtcCallRegistry1 != null) {
            mMtcCallRegistry1.removeListener(mMockListener1);
            mMtcCallRegistry1.removeListener(mMockListener2);
        }
        MtcCallRegistry.clearInstances();
    }

    @Test
    public void getInstanceIsNotNull() {
        assertNotNull("Instance for SLOT0 should not be null", mMtcCallRegistry0);
        assertNotNull("Instance for SLOT1 should not be null", mMtcCallRegistry1);
    }

    @Test
    public void getInstanceSingletonPerSlot() {
        MtcCallRegistry instance0 = MtcCallRegistry.getInstance(SLOT0);
        assertSame(
                "Should return the same instance for SLOT0",
                mMtcCallRegistry0,
                instance0);

        MtcCallRegistry instance1 = MtcCallRegistry.getInstance(SLOT1);
        assertSame(
                "Should return the same instance for SLOT1",
                mMtcCallRegistry1,
                instance1);

        assertNotSame(
                "Should return different instances for different slot IDs",
                mMtcCallRegistry0,
                mMtcCallRegistry1);
    }

    @Test
    public void notifyCallCreated() {
        mMtcCallRegistry0.addListener(mMockListener1);

        mMtcCallRegistry0.notifyCallCreated(mMockCall);

        verify(mMockListener1).onCallCreated(mMockCall);
        verify(mMockListener1, never()).onCallDestroyed(any());
    }

    @Test
    public void notifyCallDestroyed() {
        mMtcCallRegistry0.addListener(mMockListener1);

        mMtcCallRegistry0.notifyCallDestroyed(mMockCall);

        verify(mMockListener1).onCallDestroyed(mMockCall);
        verify(mMockListener1, never()).onCallCreated(any());
    }

    @Test
    public void removeListener() {
        mMtcCallRegistry0.addListener(mMockListener1);
        mMtcCallRegistry0.notifyCallCreated(mMockCall);
        verify(mMockListener1, times(1)).onCallCreated(mMockCall);

        reset(mMockListener1);
        mMtcCallRegistry0.removeListener(mMockListener1);

        mMtcCallRegistry0.notifyCallCreated(mMockCall);
        verify(mMockListener1, never()).onCallCreated(any());
    }

    @Test
    public void removeNonExistentListener() {
        mMtcCallRegistry0.addListener(mMockListener1);
        mMtcCallRegistry0.removeListener(mMockListener2);

        mMtcCallRegistry0.notifyCallCreated(mMockCall);

        verify(mMockListener1).onCallCreated(mMockCall);
        verify(mMockListener2, never()).onCallCreated(any());
    }

    @Test
    public void multipleListeners() {
        mMtcCallRegistry0.addListener(mMockListener1);
        mMtcCallRegistry0.addListener(mMockListener2);

        mMtcCallRegistry0.notifyCallCreated(mMockCall);
        verify(mMockListener1).onCallCreated(mMockCall);
        verify(mMockListener2).onCallCreated(mMockCall);

        reset(mMockListener1, mMockListener2);

        mMtcCallRegistry0.notifyCallDestroyed(mMockCall);
        verify(mMockListener1).onCallDestroyed(mMockCall);
        verify(mMockListener2).onCallDestroyed(mMockCall);
    }

    @Test
    public void listenersOnDifferentSlotsAreIsolated() {
        mMtcCallRegistry0.addListener(mMockListener1);
        mMtcCallRegistry1.addListener(mMockListener2);

        mMtcCallRegistry0.notifyCallCreated(mMockCall);
        verify(mMockListener1).onCallCreated(mMockCall);
        verify(mMockListener2, never()).onCallCreated(any());

        reset(mMockListener1, mMockListener2);

        mMtcCallRegistry1.notifyCallDestroyed(mMockCall);
        verify(mMockListener1, never()).onCallDestroyed(any());
        verify(mMockListener2).onCallDestroyed(mMockCall);
    }
}
