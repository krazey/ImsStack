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
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.verify;

import com.android.ims.internal.IImsEcbmListener;
import com.android.imsstack.enabler.mtc.EcbmListener;
import com.android.imsstack.enabler.mtc.IECallStateTracker;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsEcbmImplTest {
    private ImsEcbmImpl mImsEcbmImpl;

    //Mocked Classes
    @Mock private ICallContext mMockCallContext;
    @Mock private IECallStateTracker mMockIECallStateTracker;
    @Mock private IImsEcbmListener mMockListener;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mImsEcbmImpl = new ImsEcbmImpl(mMockCallContext);

        doReturn(mMockIECallStateTracker).when(mMockCallContext).getECallStateTracker();
        mImsEcbmImpl.init();
        mImsEcbmImpl.getImsEcbm().setListener(mMockListener);
    }

    private EcbmListener setupListener() {
        ArgumentCaptor<EcbmListener> callbackArg =
                ArgumentCaptor.forClass(EcbmListener.class);
        verify(mMockIECallStateTracker).addEcbmListener(callbackArg.capture());
        EcbmListener mListener = callbackArg.getValue();
        assertNotNull(mListener);
        return mListener;
    }

    @Test
    public void test_exitEmergencyCallbackMode() {
        mImsEcbmImpl.exitEmergencyCallbackMode();
        verify(mMockIECallStateTracker, Mockito.times(1)).exitEmergencyCallbackMode(false);
    }

    @Test
    public void test_onEcbmEntered() throws Exception {
        EcbmListener proxyListener = setupListener();
        proxyListener.onEcbmEntered();
        verify(mMockListener).enteredECBM();
    }

    @Test
    public void test_onEcbmExited() throws Exception {
        EcbmListener proxyListener = setupListener();
        proxyListener.onEcbmExited();
        verify(mMockListener).exitedECBM();
    }

    @Test
    public void test_exitEmergencyCallbackModeByESMS() {
        mImsEcbmImpl.exitEmergencyCallbackModeByESMS(true);
        verify(mMockIECallStateTracker, Mockito.times(1)).exitEmergencyCallbackMode(true);
    }

    @Test
    public void test_onEcbmEntered_Exception() throws Exception {
        EcbmListener proxyListener = setupListener();
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);
        doThrow(mockRuntimeException).when(mMockListener).enteredECBM();
        proxyListener.onEcbmEntered();
        verify(mockRuntimeException).getMessage();
    }

    @Test
    public void test_onEcbmExited_Exception() throws Exception {
        EcbmListener proxyListener = setupListener();
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);
        doThrow(mockRuntimeException).when(mMockListener).exitedECBM();
        proxyListener.onEcbmExited();
        verify(mockRuntimeException).getMessage();
    }

    @After
    public void tearDown() throws Exception {
        mImsEcbmImpl.dispose();
        mImsEcbmImpl = null;
    }
}
