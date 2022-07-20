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

package com.android.imsstack.enabler.acs;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.clearInvocations;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.enabler.acs.impl.CallbackManager;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;

public class CallbackManagerTest {
    private static final String TAG = CallbackManagerTest.class.getSimpleName();

    private static final int SLOT_ID = 0;
    private static final int SUB_ID = 1234;

    @Mock
    IAcServiceImplCallback mIAcServiceImplCallback1;
    @Mock
    IAcServiceImplCallback mIAcServiceImplCallback2;
    @Mock
    IAcServiceImplCallback mIAcServiceImplCallback3;

    private final ArrayList<IAcServiceImplCallback> mCallbackList =
            new ArrayList<IAcServiceImplCallback>();

    private CallbackManager mCallbackManager;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mCallbackList.add(mIAcServiceImplCallback1);
        mCallbackList.add(mIAcServiceImplCallback2);
        mCallbackList.add(mIAcServiceImplCallback3);

        for (IAcServiceImplCallback cb : mCallbackList) {
            doNothing().when(cb).onReceivedProvisioning(any(), anyBoolean());
            doNothing().when(cb).onReceivedPreProvisioning(any());
            doNothing().when(cb).onReceivedError(anyInt(), anyString());
        }

        mCallbackManager = new CallbackManager(SLOT_ID, SUB_ID);
    }

    @After
    public void tearDown() throws Exception {
        mCallbackManager = null;
    }

    @Test
    @SmallTest
    public void registerCallback_withMultiple() throws Exception {
        // registering success
        for (IAcServiceImplCallback cb : mCallbackList) {
            assertTrue(mCallbackManager.registerCallback(cb));
        }

        assertEquals(mCallbackList.size(), mCallbackManager.getCallbackCount());

        // registering fail, because callbacks were registered already
        for (IAcServiceImplCallback cb : mCallbackList) {
            assertFalse(mCallbackManager.registerCallback(cb));
        }

        mCallbackManager.clear();

        assertEquals(0, mCallbackManager.getCallbackCount());
    }

    @Test
    @SmallTest
    public void unregisterCallback_withMultiple() throws Exception {
        // registering success
        for (IAcServiceImplCallback cb : mCallbackList) {
            assertTrue(mCallbackManager.registerCallback(cb));
        }

        assertEquals(mCallbackList.size(), mCallbackManager.getCallbackCount());

        // unregistering success, because callbacks were registered already
        for (IAcServiceImplCallback cb : mCallbackList) {
            try {
                mCallbackManager.unregisterCallback(cb);
            } catch (IllegalArgumentException e) {
                // not expected result
                throw new AssertionError("not expected exception", e);
            }
        }

        assertEquals(0, mCallbackManager.getCallbackCount());

        // unregistering fail, because callbacks were unregistered already
        for (IAcServiceImplCallback cb : mCallbackList) {
            try {
                mCallbackManager.unregisterCallback(cb);
            } catch (IllegalArgumentException e) {
                // expected result
            }
        }
    }

    @Test
    @SmallTest
    public void notifyCallback() throws Exception {
        // registering success
        for (IAcServiceImplCallback cb : mCallbackList) {
            mCallbackManager.registerCallback(cb);
        }

        mCallbackManager.notifyOnReceivedProvisioning(new byte[] {0}, true);

        for (IAcServiceImplCallback cb : mCallbackList) {
            verify(cb, times(1))
                    .onReceivedProvisioning(any(), eq(true));
            clearInvocations(cb);
        }

        mCallbackManager.notifyOnReceivedPreProvisioning(new byte[] {0});

        for (IAcServiceImplCallback cb : mCallbackList) {
            verify(cb, times(1)).onReceivedPreProvisioning(any());
            clearInvocations(cb);
        }

        int errorCode = 403;
        String errorString = "Forbidden";
        mCallbackManager.notifyOnReceivedError(errorCode, errorString);

        for (IAcServiceImplCallback cb : mCallbackList) {
            verify(cb, times(1))
                    .onReceivedError(eq(errorCode), eq(errorString));
            verifyNoMoreInteractions(cb);
        }
    }
}
