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

import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.telephony.ims.ImsExternalCallState;

import com.android.ims.internal.IImsExternalCallStateListener;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.List;

@RunWith(JUnit4.class)
public class ImsMultiEndpointImplTest {
    private ImsMultiEndpointImpl mImsMultiEndpointImpl;
    private List<ImsExternalCallState> mImsExternalCallState;

    @Mock ICallContext mICallContext;
    @Mock IImsExternalCallStateListener mListener;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        mImsMultiEndpointImpl = new ImsMultiEndpointImpl(mICallContext);
        mImsExternalCallState = new ArrayList<ImsExternalCallState>();
        mImsMultiEndpointImpl.getIImsMultiEndpoint().setListener(mListener);
    }

    @After
    public void tearDown() throws Exception {
        mImsMultiEndpointImpl.dispose();
        mImsMultiEndpointImpl = null;
    }

    @Test
    public void test_requestImsExternalCallStateInfo() throws Exception {
        mImsMultiEndpointImpl.updateDialogState(mImsExternalCallState);
        mImsMultiEndpointImpl.requestImsExternalCallStateInfo();
        verify(mListener, times(2)).onImsExternalCallStateUpdate(mImsExternalCallState);
    }

    @Test
    public void test_updateDialogState() throws Exception {
        mImsMultiEndpointImpl.updateDialogState(mImsExternalCallState);
        verify(mListener).onImsExternalCallStateUpdate(mImsExternalCallState);
    }
}
