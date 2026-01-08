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
package com.android.imsstack.base;

import static android.telephony.AccessNetworkConstants.TRANSPORT_TYPE_WWAN;

import static com.android.imsstack.base.TestAppContext.SLOT0;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.net.Uri;
import android.os.OutcomeReceiver;
import android.telephony.CellInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.ServiceState;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.telephony.TelephonyManager.BootstrapAuthenticationCallback;
import android.telephony.TelephonyManager.CellInfoCallback;
import android.telephony.gba.UaSecurityProtocolIdentifier;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.MockitoAnnotations;

import java.util.Collections;
import java.util.List;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class TelephonyManagerProxyImplTest {
    private ContextFixture mContextFixture;
    private Context mContext;
    private TelephonyManager mTelephonyManager;
    private TelephonyManagerProxyImpl mTelephonyManagerProxy;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        mTelephonyManager = mContext.getSystemService(TelephonyManager.class);

        mTelephonyManagerProxy = new TelephonyManagerProxyImpl(mContext);
    }

    @After
    public void tearDown() throws Exception {
        mTelephonyManagerProxy = null;
        mTelephonyManager = null;
        mContext = null;
        mContextFixture = null;
    }

    @Test
    @SmallTest
    public void testCreateForSubscriptionId() {
        TelephonyManagerProxy telephonyManagerProxy =
                mTelephonyManagerProxy.createForSubscriptionId(TestAppContext.SUB_ID_1);

        assertNotNull(telephonyManagerProxy);
        verify(mTelephonyManager).createForSubscriptionId(eq(TestAppContext.SUB_ID_1));

        // Expected that the TelephonyManager is null.
        mContextFixture.setSystemService(Context.TELEPHONY_SERVICE, null);
        assertThrows(IllegalStateException.class, () -> {
            mTelephonyManagerProxy.createForSubscriptionId(TestAppContext.SUB_ID_1);
        });
    }

    @Test
    @SmallTest
    public void testRegisterAndUnregisterTelephonyCallback() {
        Executor executor = mock(Executor.class);
        TelephonyCallback telephonyCallback = mock(TelephonyCallback.class);

        mTelephonyManagerProxy.registerTelephonyCallback(executor, telephonyCallback);
        verify(mTelephonyManager).registerTelephonyCallback(eq(executor), eq(telephonyCallback));

        mTelephonyManagerProxy.unregisterTelephonyCallback(telephonyCallback);
        verify(mTelephonyManager).unregisterTelephonyCallback(eq(telephonyCallback));

        setUpTelephonyManagerNull();
        assertThrows(IllegalStateException.class, () -> {
            mTelephonyManagerProxy.registerTelephonyCallback(executor, telephonyCallback);
        });

        mTelephonyManagerProxy.unregisterTelephonyCallback(telephonyCallback);
        verifyNoMoreInteractions(mTelephonyManager);
    }

    @Test
    @SmallTest
    public void testAccessors() {
        mTelephonyManagerProxy.getActiveModemCount();
        verify(mTelephonyManager).getActiveModemCount();

        mTelephonyManagerProxy.getSupportedModemCount();
        verify(mTelephonyManager).getSupportedModemCount();

        mTelephonyManagerProxy.getImei(TestAppContext.SLOT0);
        verify(mTelephonyManager).getImei(eq(TestAppContext.SLOT0));

        mTelephonyManagerProxy.getDeviceSoftwareVersion(TestAppContext.SLOT0);
        verify(mTelephonyManager).getDeviceSoftwareVersion(eq(TestAppContext.SLOT0));

        final String number = "911";
        mTelephonyManagerProxy.isEmergencyNumber(number);
        verify(mTelephonyManager).isEmergencyNumber(eq(number));

        mTelephonyManagerProxy.hasIccCard();
        verify(mTelephonyManager).hasIccCard();

        mTelephonyManagerProxy.isApplicationOnUicc(TelephonyManager.APPTYPE_ISIM);
        verify(mTelephonyManager).isApplicationOnUicc(eq(TelephonyManager.APPTYPE_ISIM));

        mTelephonyManagerProxy.getSimApplicationState();
        verify(mTelephonyManager).getSimApplicationState();

        mTelephonyManagerProxy.getSimState(TestAppContext.SLOT0);
        verify(mTelephonyManager).getSimState(eq(TestAppContext.SLOT0));

        mTelephonyManagerProxy.getSimCardState();
        verify(mTelephonyManager).getSimCardState();

        mTelephonyManagerProxy.getSimCarrierId();
        verify(mTelephonyManager).getSimCarrierId();

        mTelephonyManagerProxy.getSimCarrierIdName();
        verify(mTelephonyManager).getSimCarrierIdName();

        mTelephonyManagerProxy.getSimSpecificCarrierId();
        verify(mTelephonyManager).getSimSpecificCarrierId();

        mTelephonyManagerProxy.getCarrierIdFromSimMccMnc();
        verify(mTelephonyManager).getCarrierIdFromSimMccMnc();

        mTelephonyManagerProxy.getSubscriberId();
        verify(mTelephonyManager).getSubscriberId();

        mTelephonyManagerProxy.getSimOperator();
        verify(mTelephonyManager).getSimOperator();

        mTelephonyManagerProxy.getSimCountryIso();
        verify(mTelephonyManager).getSimCountryIso();

        mTelephonyManagerProxy.getSimSerialNumber();
        verify(mTelephonyManager).getSimSerialNumber();

        mTelephonyManagerProxy.getGroupIdLevel1();
        verify(mTelephonyManager).getGroupIdLevel1();

        mTelephonyManagerProxy.getSimOperatorName();
        verify(mTelephonyManager).getSimOperatorName();

        mTelephonyManagerProxy.getIsimDomain();
        verify(mTelephonyManager).getIsimDomain();

        mTelephonyManagerProxy.getImsPrivateUserIdentity();
        verify(mTelephonyManager).getImsPrivateUserIdentity();

        mTelephonyManagerProxy.getImsPublicUserIdentities();
        verify(mTelephonyManager).getImsPublicUserIdentities();

        mTelephonyManagerProxy.getImsPcscfAddresses();
        verify(mTelephonyManager).getImsPcscfAddresses();

        mTelephonyManagerProxy.requestUiccIari();
        verify(mTelephonyManager).requestUiccIari(any(Executor.class), any(OutcomeReceiver.class));

        final String data = "+/Qu0CsHvIXkqHjH4+8r5HywZGqmAQAAZIbcuUWhIpc=";
        mTelephonyManagerProxy.getIccAuthentication(
                TelephonyManager.APPTYPE_ISIM, TelephonyManager.AUTHTYPE_EAP_AKA, data);
        verify(mTelephonyManager).getIccAuthentication(
                eq(TelephonyManager.APPTYPE_ISIM), eq(TelephonyManager.AUTHTYPE_EAP_AKA), eq(data));

        mTelephonyManagerProxy.isDataEnabled();
        verify(mTelephonyManager).isDataEnabled();

        mTelephonyManagerProxy.isDataRoamingEnabled();
        verify(mTelephonyManager).isDataRoamingEnabled();

        mTelephonyManagerProxy.getServiceState(SLOT0);
        verify(mTelephonyManager).getServiceStateForSlot(eq(SLOT0));

        mTelephonyManagerProxy.getNetworkOperator();
        verify(mTelephonyManager).getNetworkOperator();

        mTelephonyManagerProxy.getNetworkCountryIso();
        verify(mTelephonyManager).getNetworkCountryIso();

        mTelephonyManagerProxy.getAllCellInfo();
        verify(mTelephonyManager).getAllCellInfo();

        mTelephonyManagerProxy.getEmergencyNumberList();
        verify(mTelephonyManager).getEmergencyNumberList();
    }

    @Test
    @SmallTest
    public void testAccessorsWhenTelephonyManagerNull() {
        setUpTelephonyManagerNull();

        assertEquals(1, mTelephonyManagerProxy.getActiveModemCount());
        assertEquals(1, mTelephonyManagerProxy.getSupportedModemCount());
        assertNull(mTelephonyManagerProxy.getImei(TestAppContext.SLOT0));
        assertNull(mTelephonyManagerProxy.getDeviceSoftwareVersion(TestAppContext.SLOT0));

        final String number = "911";
        assertFalse(mTelephonyManagerProxy.isEmergencyNumber(number));
        assertFalse(mTelephonyManagerProxy.hasIccCard());
        assertFalse(mTelephonyManagerProxy.isApplicationOnUicc(TelephonyManager.APPTYPE_ISIM));
        assertEquals(TelephonyManager.SIM_STATE_UNKNOWN,
                mTelephonyManagerProxy.getSimApplicationState());
        assertEquals(TelephonyManager.SIM_STATE_UNKNOWN,
                mTelephonyManagerProxy.getSimState(TestAppContext.SLOT0));
        assertEquals(TelephonyManager.SIM_STATE_UNKNOWN, mTelephonyManagerProxy.getSimCardState());
        assertEquals(TelephonyManager.UNKNOWN_CARRIER_ID, mTelephonyManagerProxy.getSimCarrierId());
        assertNull(mTelephonyManagerProxy.getSimCarrierIdName());
        assertEquals(TelephonyManager.UNKNOWN_CARRIER_ID,
                mTelephonyManagerProxy.getSimSpecificCarrierId());
        assertEquals(TelephonyManager.UNKNOWN_CARRIER_ID,
                mTelephonyManagerProxy.getCarrierIdFromSimMccMnc());
        assertNull(mTelephonyManagerProxy.getSubscriberId());
        assertEquals("", mTelephonyManagerProxy.getSimOperator());
        assertEquals("", mTelephonyManagerProxy.getSimCountryIso());
        assertNull(mTelephonyManagerProxy.getSimSerialNumber());
        assertNull(mTelephonyManagerProxy.getGroupIdLevel1());
        assertEquals("", mTelephonyManagerProxy.getSimOperatorName());
        assertArrayEquals(new byte[0],
                mTelephonyManagerProxy.getSimServiceTable(TelephonyManager.APPTYPE_ISIM));
        assertNull(mTelephonyManagerProxy.getIsimDomain());
        assertThrows(IllegalStateException.class, () -> {
            mTelephonyManagerProxy.getImsPrivateUserIdentity();
        });
        assertEquals(Collections.EMPTY_LIST, mTelephonyManagerProxy.getImsPublicUserIdentities());
        assertEquals(Collections.EMPTY_LIST, mTelephonyManagerProxy.getImsPcscfAddresses());
        assertEquals(Collections.EMPTY_SET, mTelephonyManagerProxy.requestUiccIari());

        final String data = "+/Qu0CsHvIXkqHjH4+8r5HywZGqmAQAAZIbcuUWhIpc=";
        assertNull(mTelephonyManagerProxy.getIccAuthentication(
                TelephonyManager.APPTYPE_ISIM, TelephonyManager.AUTHTYPE_EAP_AKA, data));
        assertFalse(mTelephonyManagerProxy.isDataEnabled());
        assertFalse(mTelephonyManagerProxy.isDataRoamingEnabled());
        assertNull(mTelephonyManagerProxy.getServiceState(SLOT0));
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN,
                mTelephonyManagerProxy.getDataNetworkType(SLOT0));
        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN,
                mTelephonyManagerProxy.getVoiceNetworkType(SLOT0));
        assertEquals("", mTelephonyManagerProxy.getNetworkOperator());
        assertEquals("", mTelephonyManagerProxy.getNetworkCountryIso());
        assertNull(mTelephonyManagerProxy.getAllCellInfo());
        assertEquals(Collections.emptyMap(), mTelephonyManagerProxy.getEmergencyNumberList());
    }

    @Test
    @SmallTest
    public void testGetDataNetworkType() {
        // ServiceState is null.
        when(mTelephonyManager.getServiceStateForSlot(eq(SLOT0))).thenReturn(null);

        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN,
                mTelephonyManagerProxy.getDataNetworkType(SLOT0));

        // No NetworkRegistrationInfo.
        ServiceState ss = mock(ServiceState.class);
        when(mTelephonyManager.getServiceStateForSlot(eq(SLOT0))).thenReturn(ss);

        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN,
                mTelephonyManagerProxy.getDataNetworkType(SLOT0));

        // PS & WWAN: LTE.
        NetworkRegistrationInfo wwanNri = mock(NetworkRegistrationInfo.class);
        when(ss.getNetworkRegistrationInfo(anyInt(), eq(TRANSPORT_TYPE_WWAN))).thenReturn(wwanNri);
        when(wwanNri.getAccessNetworkTechnology()).thenReturn(TelephonyManager.NETWORK_TYPE_LTE);

        assertEquals(TelephonyManager.NETWORK_TYPE_LTE,
                mTelephonyManagerProxy.getDataNetworkType(SLOT0));
    }

    @Test
    @SmallTest
    public void testGetVoiceNetworkType() {
        // ServiceState is null.
        when(mTelephonyManager.getServiceStateForSlot(eq(SLOT0))).thenReturn(null);

        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN,
                mTelephonyManagerProxy.getVoiceNetworkType(SLOT0));

        // No NetworkRegistrationInfo.
        ServiceState ss = mock(ServiceState.class);
        when(mTelephonyManager.getServiceStateForSlot(eq(SLOT0))).thenReturn(ss);

        assertEquals(TelephonyManager.NETWORK_TYPE_UNKNOWN,
                mTelephonyManagerProxy.getVoiceNetworkType(SLOT0));

        // CS & WWAN: LTE.
        NetworkRegistrationInfo wwanNri = mock(NetworkRegistrationInfo.class);
        when(ss.getNetworkRegistrationInfo(anyInt(), eq(TRANSPORT_TYPE_WWAN))).thenReturn(wwanNri);
        when(wwanNri.getAccessNetworkTechnology()).thenReturn(TelephonyManager.NETWORK_TYPE_LTE);

        assertEquals(TelephonyManager.NETWORK_TYPE_LTE,
                mTelephonyManagerProxy.getVoiceNetworkType(SLOT0));
    }

    @Test
    @SmallTest
    public void testSendEnvelopeWithStatus() {
        final String content = "abcd";

        mTelephonyManagerProxy.sendEnvelopeWithStatus(content);
        verify(mTelephonyManager).sendEnvelopeWithStatus(eq(content));

        // Expected that the TelephonyManager is null.
        setUpTelephonyManagerNull();
        assertEquals("", mTelephonyManagerProxy.sendEnvelopeWithStatus(content));
    }

    @Test
    @SmallTest
    public void testRequestCellInfoUpdate() {
        Executor executor = Runnable::run;
        CellInfoCallback cellInfoCallback = mock(CellInfoCallback.class);

        mTelephonyManagerProxy.requestCellInfoUpdate(executor, cellInfoCallback);
        verify(mTelephonyManager).requestCellInfoUpdate(eq(executor), eq(cellInfoCallback));

        // Expected that the TelephonyManager is null.
        setUpTelephonyManagerNull();
        mTelephonyManagerProxy.requestCellInfoUpdate(executor, cellInfoCallback);

        ArgumentCaptor<List<CellInfo>> captor = ArgumentCaptor.forClass(List.class);
        verify(cellInfoCallback).onCellInfo(captor.capture());
        List<CellInfo> cellInfos = captor.getValue();
        assertTrue(cellInfos.isEmpty());
    }

    @Test
    @SmallTest
    public void testBootstrapAuthenticationRequest() {
        Executor executor = Runnable::run;
        Uri nafId = mock(Uri.class);
        UaSecurityProtocolIdentifier securityProtocol = mock(UaSecurityProtocolIdentifier.class);
        BootstrapAuthenticationCallback callback = mock(BootstrapAuthenticationCallback.class);

        mTelephonyManagerProxy.bootstrapAuthenticationRequest(
                TelephonyManager.APPTYPE_ISIM, nafId, securityProtocol, false, executor, callback);
        verify(mTelephonyManager).bootstrapAuthenticationRequest(
                eq(TelephonyManager.APPTYPE_ISIM), eq(nafId), eq(securityProtocol), eq(false),
                eq(executor), eq(callback));

        // Expected that the TelephonyManager is null.
        setUpTelephonyManagerNull();
        mTelephonyManagerProxy.bootstrapAuthenticationRequest(
                TelephonyManager.APPTYPE_ISIM, nafId, securityProtocol, false, executor, callback);
        verify(callback).onAuthenticationFailure(
                eq(TelephonyManager.GBA_FAILURE_REASON_FEATURE_NOT_READY));
    }

    @Test
    @SmallTest
    public void testGetSimServiceTableForIsim() {
        mTelephonyManagerProxy.getSimServiceTable(TelephonyManager.APPTYPE_ISIM);
        verify(mTelephonyManager).getSimServiceTable(eq(TelephonyManager.APPTYPE_ISIM),
                any(Executor.class), any(OutcomeReceiver.class));
    }

    private void setUpTelephonyManagerNull() {
        mContextFixture.setSystemService(Context.TELEPHONY_SERVICE, null);
        mTelephonyManagerProxy = new TelephonyManagerProxyImpl(mContext);
    }
}
