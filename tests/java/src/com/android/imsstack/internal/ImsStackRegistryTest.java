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

import static com.android.imsstack.base.TestAppContext.SLOT0;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.util.SparseBooleanArray;

import androidx.test.filters.SmallTest;

import com.android.imsstack.TestInstanceHolder;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.CopyOnWriteArraySet;

@RunWith(JUnit4.class)
public class ImsStackRegistryTest {
    private @Mock ImsStackRegistry.ImsServiceListener mImsServiceListener;

    private AutoCloseable mMocksCloseable;
    private final TestInstanceHolder mInstanceHolder = new TestInstanceHolder();

    @Before
    public void setUp() throws Exception {
        mMocksCloseable = MockitoAnnotations.openMocks(this);

        mInstanceHolder.replace(ImsStackRegistry.class, "sImsServiceStates", null,
                new SparseBooleanArray());
        mInstanceHolder.replace(ImsStackRegistry.class, "sImsServiceListeners", null,
                new CopyOnWriteArraySet<ImsStackRegistry.ImsServiceListener>());
        ImsStackRegistry.setImsServiceState(SLOT0, false);
        ImsStackRegistry.addImsServiceListener(mImsServiceListener);
    }

    @After
    public void tearDown() throws Exception {
        ImsStackRegistry.setImsServiceState(SLOT0, false);
        ImsStackRegistry.removeImsServiceListener(mImsServiceListener);

        mInstanceHolder.restoreAll();
        if (mMocksCloseable != null) {
            mMocksCloseable.close();
            mMocksCloseable = null;
        }
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
