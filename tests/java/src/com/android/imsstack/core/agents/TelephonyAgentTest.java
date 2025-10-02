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

package com.android.imsstack.core.agents;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SLOT1;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class TelephonyAgentTest {
    @Mock private PhoneStateInterface mPhoneStateInterface;
    @Mock private IDcNetWatcher mDcNetWatcher;

    private TestAppContext mTestAppContext;
    private TelephonyManagerProxy mTelephonyManagerProxy;
    private TelephonyAgent mTelephonyAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();

        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        AgentFactory.getInstance().setAgent(PhoneStateInterface.class, mPhoneStateInterface, SLOT0);
        DcFactory.setDcAgent(IDcNetWatcher.class, mDcNetWatcher, SLOT0);

        mTelephonyAgent = new TelephonyAgent(SLOT0);
        mTelephonyAgent.init(mTestAppContext.getContext());
    }

    @After
    public void tearDown() throws Exception {
        mTelephonyManagerProxy = null;
        mTelephonyAgent.cleanup();
        mTelephonyAgent = null;
        DcFactory.setDcAgent(IDcNetWatcher.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(PhoneStateInterface.class, null, SLOT0);
        DeviceConfig.setSimCount(1, 1);
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void testGetCsCallState() {
        mTelephonyAgent.getCsCallState();

        verify(mPhoneStateInterface).getCsCallState();

        AgentFactory.getInstance().setAgent(PhoneStateInterface.class, null, SLOT0);

        // Expect a default value.
        assertEquals(TelephonyManager.CALL_STATE_IDLE, mTelephonyAgent.getCsCallState());
    }

    @Test
    @SmallTest
    public void testGetCsCallStateInOtherSlot() {
        DeviceConfig.setSimCount(1, 1);

        int callState = mTelephonyAgent.getCsCallStateInOtherSlot();
        // Skipped for same SIM.
        assertEquals(TelephonyManager.CALL_STATE_IDLE, callState);

        DeviceConfig.setSimCount(2, 2);
        AgentFactory.getInstance().setAgent(PhoneStateInterface.class, mPhoneStateInterface, SLOT1);
        when(mPhoneStateInterface.getCsCallState()).thenReturn(TelephonyManager.CALL_STATE_OFFHOOK);
        callState = mTelephonyAgent.getCsCallStateInOtherSlot();

        // Called once for SIM2.
        verify(mPhoneStateInterface).getCsCallState();
        assertEquals(TelephonyManager.CALL_STATE_OFFHOOK, callState);

        AgentFactory.getInstance().setAgent(PhoneStateInterface.class, null, SLOT1);

        // Expect a default value.
        callState = mTelephonyAgent.getCsCallStateInOtherSlot();
        assertEquals(TelephonyManager.CALL_STATE_IDLE, callState);
    }

    @Test
    @SmallTest
    public void testGetNetworkType() {
        // LTE
        when(mTelephonyManagerProxy.getDataNetworkType(eq(SLOT0)))
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE);

        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, mTelephonyAgent.getNetworkType());
        verify(mDcNetWatcher).updateTelephonyNetworkType(eq(TelephonyManager.NETWORK_TYPE_LTE));

        // NR
        when(mTelephonyManagerProxy.getDataNetworkType(eq(SLOT0)))
                .thenReturn(TelephonyManager.NETWORK_TYPE_NR);
        when(mDcNetWatcher.isImsSupportedNetworkType(TelephonyManager.NETWORK_TYPE_NR))
                .thenReturn(true);

        assertEquals(TelephonyManager.NETWORK_TYPE_NR, mTelephonyAgent.getNetworkType());
        verify(mDcNetWatcher).updateTelephonyNetworkType(eq(TelephonyManager.NETWORK_TYPE_NR));
    }

    @Test
    @SmallTest
    public void testGetVoiceNetworkType() {
        // LTE
        when(mTelephonyManagerProxy.getVoiceNetworkType(eq(SLOT0)))
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE);

        assertEquals(TelephonyManager.NETWORK_TYPE_LTE, mTelephonyAgent.getVoiceNetworkType());
        verify(mDcNetWatcher)
                .updateTelephonyVoiceNetworkType(eq(TelephonyManager.NETWORK_TYPE_LTE));

        // NR
        when(mTelephonyManagerProxy.getVoiceNetworkType(eq(SLOT0)))
                .thenReturn(TelephonyManager.NETWORK_TYPE_NR);
        when(mDcNetWatcher.isImsSupportedNetworkType(TelephonyManager.NETWORK_TYPE_NR))
                .thenReturn(true);

        assertEquals(TelephonyManager.NETWORK_TYPE_NR, mTelephonyAgent.getVoiceNetworkType());
        verify(mDcNetWatcher)
                .updateTelephonyVoiceNetworkType(eq(TelephonyManager.NETWORK_TYPE_NR));
    }

    @Test
    @SmallTest
    public void testGetPhoneNumber() {
        mTelephonyAgent.getPhoneNumber();

        SubscriptionManagerProxy smp =
                mTestAppContext.getSystemServiceProxy(SubscriptionManagerProxy.class);
        verify(smp).getPhoneNumber(
                eq(TestAppContext.SUB_ID_1), eq(SubscriptionManager.PHONE_NUMBER_SOURCE_UICC));
    }

    @Test
    @SmallTest
    public void testGetSimOperator() {
        when(mTelephonyManagerProxy.getSimOperator()).thenReturn("00101");
        String operator = mTelephonyAgent.getSimOperator();
        String mcc = mTelephonyAgent.getSimMcc();
        String mnc = mTelephonyAgent.getSimMnc();

        assertEquals("00101", operator);
        assertEquals("001", mcc);
        assertEquals("01", mnc);

        when(mTelephonyManagerProxy.getSimOperator()).thenReturn("001002");
        operator = mTelephonyAgent.getSimOperator();
        mcc = mTelephonyAgent.getSimMcc();
        mnc = mTelephonyAgent.getSimMnc();

        assertEquals("001002", operator);
        assertEquals("001", mcc);
        assertEquals("002", mnc);

        when(mTelephonyManagerProxy.getSimOperator()).thenReturn("0012");
        operator = mTelephonyAgent.getSimOperator();
        mcc = mTelephonyAgent.getSimMcc();
        mnc = mTelephonyAgent.getSimMnc();

        assertEquals("0012", operator);
        assertNull(mcc);
        assertNull(mnc);

        when(mTelephonyManagerProxy.getSimOperator()).thenReturn(null);
        operator = mTelephonyAgent.getSimOperator();
        mcc = mTelephonyAgent.getSimMcc();
        mnc = mTelephonyAgent.getSimMnc();

        assertNull(operator);
        assertNull(mcc);
        assertNull(mnc);
    }

    @Test
    @SmallTest
    public void testGetNetworkOperator() {
        when(mTelephonyManagerProxy.getNetworkOperator()).thenReturn("00101");
        String operator = mTelephonyAgent.getNetworkOperator();
        String mcc = mTelephonyAgent.getNetworkMcc();
        String mnc = mTelephonyAgent.getNetworkMnc();

        assertEquals("00101", operator);
        assertEquals("001", mcc);
        assertEquals("01", mnc);

        when(mTelephonyManagerProxy.getNetworkOperator()).thenReturn("001002");
        operator = mTelephonyAgent.getNetworkOperator();
        mcc = mTelephonyAgent.getNetworkMcc();
        mnc = mTelephonyAgent.getNetworkMnc();

        assertEquals("001002", operator);
        assertEquals("001", mcc);
        assertEquals("002", mnc);

        when(mTelephonyManagerProxy.getNetworkOperator()).thenReturn("0012");
        operator = mTelephonyAgent.getNetworkOperator();
        mcc = mTelephonyAgent.getNetworkMcc();
        mnc = mTelephonyAgent.getNetworkMnc();

        assertEquals("0012", operator);
        assertNull(mcc);
        assertNull(mnc);

        when(mTelephonyManagerProxy.getNetworkOperator()).thenReturn(null);
        operator = mTelephonyAgent.getNetworkOperator();
        mcc = mTelephonyAgent.getNetworkMcc();
        mnc = mTelephonyAgent.getNetworkMnc();

        assertNull(operator);
        assertNull(mcc);
        assertNull(mnc);
    }

    @Test
    @SmallTest
    public void testIsEmergencyNumber() {
        final String eNumber = "911";
        final String formattedNumber = "+9-1-1";
        mTelephonyAgent.isEmergencyNumber(eNumber);

        verify(mTelephonyManagerProxy).isEmergencyNumber(eq(eNumber));

        mTelephonyAgent.isEmergencyNumber(formattedNumber);

        verify(mTelephonyManagerProxy).isEmergencyNumber(eq(eNumber));
    }

    @Test
    @SmallTest
    public void testGetTelephonyStates() {
        mTelephonyAgent.getSimState();
        mTelephonyAgent.getImei();
        mTelephonyAgent.getDeviceSoftwareVersion();
        mTelephonyAgent.getSubscriberId();
        mTelephonyAgent.getSimCountryIso();
        mTelephonyAgent.getSimSerialNumber();
        mTelephonyAgent.getSimGid1();
        mTelephonyAgent.getSimOperatorName();
        mTelephonyAgent.getNetworkCountryIso();

        verify(mTelephonyManagerProxy).getSimState(eq(SLOT0));
        verify(mTelephonyManagerProxy).getImei(eq(SLOT0));
        verify(mTelephonyManagerProxy).getDeviceSoftwareVersion(eq(SLOT0));
        verify(mTelephonyManagerProxy).getSubscriberId();
        verify(mTelephonyManagerProxy).getSimCountryIso();
        verify(mTelephonyManagerProxy).getSimSerialNumber();
        verify(mTelephonyManagerProxy).getGroupIdLevel1();
        verify(mTelephonyManagerProxy).getSimOperatorName();
        verify(mTelephonyManagerProxy).getNetworkCountryIso();
    }

    @Test
    @SmallTest
    public void testGetTelephonyStatesWhenSubscriptionInvalid() {
        SubscriptionManagerProxy smp =
                mTestAppContext.getSystemServiceProxy(SubscriptionManagerProxy.class);
        when(smp.getSubscriptionId(eq(SLOT0)))
                .thenReturn(SubscriptionManager.INVALID_SUBSCRIPTION_ID);

        mTelephonyAgent.getSimState();
        mTelephonyAgent.getImei();
        mTelephonyAgent.getDeviceSoftwareVersion();
        mTelephonyAgent.getSubscriberId();
        mTelephonyAgent.getSimCountryIso();
        mTelephonyAgent.getSimSerialNumber();
        mTelephonyAgent.getSimGid1();
        mTelephonyAgent.getSimOperatorName();
        mTelephonyAgent.getNetworkCountryIso();

        verify(mTelephonyManagerProxy).getSimState(eq(SLOT0));
        verify(mTelephonyManagerProxy).getImei(eq(SLOT0));
        verify(mTelephonyManagerProxy).getDeviceSoftwareVersion(eq(SLOT0));
        verify(mTelephonyManagerProxy).getSubscriberId();
        verify(mTelephonyManagerProxy).getSimCountryIso();
        verify(mTelephonyManagerProxy).getSimSerialNumber();
        verify(mTelephonyManagerProxy).getGroupIdLevel1();
        verify(mTelephonyManagerProxy).getSimOperatorName();
        verify(mTelephonyManagerProxy).getNetworkCountryIso();
    }
}
