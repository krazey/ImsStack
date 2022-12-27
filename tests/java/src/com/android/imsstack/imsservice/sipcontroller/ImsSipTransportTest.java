/*
 * Copyright (C) 2021 The Android Open Source Project
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
package com.android.imsstack.imsservice.sipcontroller;

import static com.google.common.truth.Truth.assertThat;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anySet;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;

import android.content.Context;
import android.telephony.ims.DelegateMessageCallback;
import android.telephony.ims.DelegateRequest;
import android.telephony.ims.DelegateStateCallback;
import android.telephony.ims.SipDelegateManager;
import android.telephony.ims.stub.SipDelegate;

import com.android.imsstack.enabler.sipcontroller.impl.SipControllerAgent;
import com.android.imsstack.imsservice.mmtel.ImsRegistrationImpl;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;

import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
@RunWith(JUnit4.class)
public class ImsSipTransportTest {
    @Mock
    Context mContextMock;
    ImsSipTransport mImsSipTransport;
    //Valid slot
    private final int mSlotId = 0;

    @Before
    public void setUp() {
        mContextMock = Mockito.mock(Context.class);
        AppContext.init(mContextMock);
        Executor e = Executors.newSingleThreadExecutor();
        SipControllerAgent agent = Mockito.mock(SipControllerAgent.class);
        mImsSipTransport = new ImsSipTransport(mSlotId, mContextMock, e, e,
                new ImsRegistrationImpl(), agent);
    }
    @After
    public void tearDown() {
        mImsSipTransport = null;
        AppContext.deinit();
    }
    @Test
    public void createSipDelegate_invalidSubId() {
        IllegalArgumentException thrown =
                assertThrows(
                        IllegalArgumentException.class,
                        () -> mImsSipTransport.createSipDelegate(-1, null, null, null));
        assertThat(thrown).hasMessageThat().contains("subId:");
    }
    @Test
    public void createSipDelegate_nullCallBacks() {
        Set<String> featureRequest = new HashSet<>();
        DelegateRequest request = new DelegateRequest(featureRequest);
        IllegalArgumentException thrown =
                assertThrows(
                        IllegalArgumentException.class,
                        () -> mImsSipTransport.createSipDelegate(mSlotId, request, null, null));
        assertThat(thrown).hasMessageThat().contains("Null parameter passed");
    }
    /**
     * This test case checks whether sip delegate created and callback is called properly.
     */
    @Test
    public void createSipDelegate_withAllvalidparameters() {
        Set<String> featureRequest = new HashSet<>();
        DelegateRequest request = new DelegateRequest(featureRequest);
        DelegateStateCallback stateCallback = Mockito.mock(DelegateStateCallback.class);
        DelegateMessageCallback messageCallback = Mockito.mock(DelegateMessageCallback.class);
        mImsSipTransport.createSipDelegate(mSlotId, request, stateCallback, messageCallback);
        verify(stateCallback, timeout(1000)).onCreated(any(SipDelegate.class), anySet());
    }
    /**
     * Delegate destroy called on null object should throw exception
     */
    @Test
    public void destroySipDelegate_withNullDelegate() {
        IllegalArgumentException thrown =
                assertThrows(
                        IllegalArgumentException.class,
                        () -> mImsSipTransport.destroySipDelegate(null,-1));
        assertThat(thrown).hasMessageThat().contains("Null parameter passed");
    }
    /**
     * Delegate destroy called on null object should throw exception
     */
    @Test
    public void destroySipDelegate_withAllvalidparameters() {
        //before destroy create the sip delegate object and capture the created object to destroy
        Set<String> featureRequest = new HashSet<>();
        DelegateRequest request = new DelegateRequest(featureRequest);
        DelegateStateCallback stateCallback = Mockito.mock(DelegateStateCallback.class);
        DelegateMessageCallback messageCallback = Mockito.mock(DelegateMessageCallback.class);
        //Capture the sip delegate objeject to call onDestroy.
        ArgumentCaptor<SipDelegate> delegate = ArgumentCaptor.forClass(SipDelegate.class);
        mImsSipTransport.createSipDelegate(mSlotId, request, stateCallback, messageCallback);
        verify(stateCallback, timeout(1000)).onCreated(delegate.capture(), anySet());
        //Use the sip delegate object to destroy the same.
        mImsSipTransport.destroySipDelegate(delegate.getValue(),
                SipDelegateManager.SIP_DELEGATE_DESTROY_REASON_REQUESTED_BY_APP);
        verify(stateCallback, timeout(1000)).
                onDestroyed(SipDelegateManager.SIP_DELEGATE_DESTROY_REASON_REQUESTED_BY_APP);
    }
    @Test
    public void getActiveSipDelegateManager(){
        assertNotNull(mImsSipTransport.getActiveSipDelegateManager());
    }
}
