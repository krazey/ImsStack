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

package com.android.imsstack.internal.enabler;

import static com.android.imsstack.internal.enabler.ImsStateStore.NETWORK_TYPE_LTE;
import static com.android.imsstack.internal.enabler.ImsStateStore.STATE_ACTIVE;
import static com.android.imsstack.internal.enabler.ImsStateStore.STATE_INACTIVE;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ImsStateStoreTest {

    private final int mPhoneId = 1;
    private ImsStateStore.Listener mListener;

    @Before
    public void setUp() throws Exception {
        mListener = mock(ImsStateStore.Listener.class);
        ImsStateStore.init(mPhoneId);
    }

    @After
    public void tearDown() throws Exception {
        mListener = null;
    }

    @Test
    public void imsState_all_Test() {
        assertNotNull(ImsStateStore.getImsState(mPhoneId));
        assertNotNull(ImsStateStore.getCallState(mPhoneId));
        assertNotNull(ImsStateStore.getRegState(mPhoneId));
        assertNotNull(ImsStateStore.getMmTelState(mPhoneId));
    }

    @Test
    public void regState_setStateTest() {
        ImsStateStore.getRegState(mPhoneId).init();

        ImsStateStore.getRegState(mPhoneId).setState(STATE_INACTIVE);
        assertEquals(STATE_INACTIVE, ImsStateStore.getRegState(mPhoneId).getState());

        ImsStateStore.getRegState(mPhoneId).setState(STATE_ACTIVE);
        assertEquals(STATE_ACTIVE, ImsStateStore.getRegState(mPhoneId).getState());

    }

    @Test
    public void regState_setNetworkTypeTest() {
        ImsStateStore.getRegState(mPhoneId).setNetworkType(NETWORK_TYPE_LTE);
        assertEquals(NETWORK_TYPE_LTE, ImsStateStore.getRegState(mPhoneId).getNetworkType());
        assertFalse(ImsStateStore.getRegState(mPhoneId).isNetworkTypeWifi());
    }

    @Test
    public void regState_removeListenerTest() {
        ImsStateStore.getRegState(mPhoneId).setState(STATE_INACTIVE);
        ImsStateStore.getRegState(mPhoneId).removeListener(mListener);
        verify(mListener, Mockito.times(0)).onStateChanged();
    }

    @Test
    public void callState_setState() {
        Mockito.reset(mListener);

        ImsStateStore.getCallState(mPhoneId).init();

        ImsStateStore.getCallState(mPhoneId).setState(STATE_ACTIVE);
        assertEquals(STATE_ACTIVE, ImsStateStore.getCallState(mPhoneId).getState());

        ImsStateStore.getCallState(mPhoneId).setState(STATE_INACTIVE);
        assertEquals(STATE_INACTIVE, ImsStateStore.getCallState(mPhoneId).getState());

    }

    @Test
    public void callState_removeListenerTest() {
        ImsStateStore.getCallState(mPhoneId).setState(STATE_ACTIVE);
        ImsStateStore.getCallState(mPhoneId).removeListener(mListener);
        Mockito.verify(mListener, Mockito.times(0)).onStateChanged();
    }

    @Test
    public void mmTelState_setProvisionedTest() {
        Mockito.reset(mListener);
        ImsStateStore.getMmTelState(mPhoneId).addListener(mListener);

        ImsStateStore.getMmTelState(mPhoneId).setProvisioned(STATE_ACTIVE, STATE_ACTIVE,
                STATE_ACTIVE);
        verify(mListener).onStateChanged();

        Mockito.reset(mListener);
        ImsStateStore.getMmTelState(mPhoneId).setProvisioned(STATE_ACTIVE, STATE_INACTIVE,
                STATE_INACTIVE);
        verify(mListener).onStateChanged();

        Mockito.reset(mListener);
        ImsStateStore.getMmTelState(mPhoneId).setProvisioned(STATE_INACTIVE, STATE_ACTIVE,
                STATE_INACTIVE);
        verify(mListener).onStateChanged();
    }

    @Test
    public void mmTelState_Provisioned_allTest() {
        assertFalse(ImsStateStore.getMmTelState(mPhoneId).isVoLteProvisioned());
        assertFalse(ImsStateStore.getMmTelState(mPhoneId).isVtProvisioned());
        assertFalse(ImsStateStore.getMmTelState(mPhoneId).isWfcProvisioned());

        ImsStateStore.getMmTelState(mPhoneId).setVoLteProvisioned(STATE_ACTIVE);
        assertEquals(true, ImsStateStore.getMmTelState(mPhoneId).isVoLteProvisioned());

        ImsStateStore.getMmTelState(mPhoneId).setWfcProvisioned(STATE_ACTIVE);
        assertEquals(true, ImsStateStore.getMmTelState(mPhoneId).isWfcProvisioned());

        ImsStateStore.getMmTelState(mPhoneId).setVtProvisioned(STATE_ACTIVE);
        assertEquals(true, ImsStateStore.getMmTelState(mPhoneId).isVtProvisioned());
    }
}
