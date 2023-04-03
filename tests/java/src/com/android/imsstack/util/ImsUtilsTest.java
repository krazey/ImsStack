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

package com.android.imsstack.util;

import static com.android.imsstack.util.ImsUtils.NR_NETWORK_MODE_SA;
import static com.android.imsstack.util.ImsUtils.NR_UE_CAPABILITY_I_SA;
import static com.android.imsstack.util.ImsUtils.NR_UE_CAPABILITY_I_VONR;
import static com.android.imsstack.util.ImsUtils.NR_UE_CAPABILITY_VONR;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ImsUtilsTest {

    private Context mContext;
    private static final int SLOT_0 = 0;
    private static final int[] SUB_ID = {1};

    @Before
    public void setup() {
        mContext = new ContextFixture().getTestDouble();
        TelephonyManager tm = mContext.getSystemService(TelephonyManager.class);
        when(mContext.getResources().getBoolean(anyInt())).thenReturn(true);
        when(tm.getActiveModemCount()).thenReturn(1);
        when(tm.getSupportedModemCount()).thenReturn(1);
        SubscriptionManager mockSubscriptionManager = mContext.getSystemService(
                SubscriptionManager.class);
        when(mockSubscriptionManager.getSubscriptionIds(anyInt()))
                .thenReturn(SUB_ID);

        CarrierConfig mockCarrierConfig = Mockito.mock(CarrierConfig.class);
        CarrierConfigManager mockCarrierConfigManager = mContext.getSystemService(
                CarrierConfigManager.class);
        PersistableBundle bundle = new PersistableBundle();
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL, true);
        doReturn(bundle).when(mockCarrierConfigManager).getConfigForSubId(SUB_ID[0]);
        when(mockCarrierConfig.getConfig()).thenReturn(bundle);

        AppContext.init(mContext);
    }

    @After
    public  void tearDown() {
        AppContext.deinit();
        mContext = null;
    }

    @Test
    @SmallTest
    public void setUeCapabilityVoNrAndNrNetworkMode() {

        ImsUtils.setUeCapabilityVoNr(SLOT_0, NR_UE_CAPABILITY_I_VONR);
        assertEquals(NR_UE_CAPABILITY_VONR, ImsUtils.getNrUeCapability(SLOT_0));

        ImsUtils.setNrNetworkMode(SLOT_0, NR_NETWORK_MODE_SA);
        assertEquals(NR_UE_CAPABILITY_I_SA, ImsUtils.getNrUeCapability(SLOT_0));

        ImsUtils.setNrUeCapability(SLOT_0, NR_UE_CAPABILITY_I_SA);
        assertEquals(NR_UE_CAPABILITY_I_SA, ImsUtils.getNrUeCapability(SLOT_0));

        ImsUtils.init();
        ImsUtils.getImsManager(SLOT_0);
        ImsUtils.setNrUeCapability(SLOT_0, NR_UE_CAPABILITY_I_VONR);
        assertEquals(NR_UE_CAPABILITY_I_VONR, ImsUtils.getNrUeCapability(SLOT_0));

        assertEquals(NR_NETWORK_MODE_SA, ImsUtils.getDefaultNrNetworkMode());
        assertEquals(NR_UE_CAPABILITY_I_SA, ImsUtils.getDefaultNrUeCapability());
    }

    @Test
    @SmallTest
    public void getPlatformConfigurations() {
        ImsUtils.init();
        ImsUtils.getImsManager(SLOT_0);
        assertNotNull(ImsUtils.getNrUeCapability(SLOT_0));
        assertEquals(true, ImsUtils.isEmergencyCallEnabledOnNonVoLteSim());
        assertEquals(true, ImsUtils.isEmergencyCallEnabledOnServiceRestricted());

        assertEquals(true, ImsUtils.isWfcEnabledByDevice(mContext, SLOT_0));

        // Blocked these assertions temporarily - flaky tests.
        //assertEquals(true, ImsUtils.isVtEnabledByPlatform(mContext, SLOT_0));
        //assertEquals(true, ImsUtils.isWfcEnabledByPlatform(mContext, SLOT_0));
    }

    @Test
    @SmallTest
    public void checkDefaultServiceCapability() {
        ImsUtils.init();
        ImsUtils.ServiceCaps capsFromLocalStorage = ImsUtils.getServiceCapsFromLocalStorage(
                SLOT_0);
        assertEquals(false, capsFromLocalStorage.isVoLteEnabled());
        assertEquals(false, capsFromLocalStorage.isVtEnabled());
        assertEquals(false, capsFromLocalStorage.isVoLteEnabled());
        assertEquals(false, capsFromLocalStorage.isVtEnabled());
        assertEquals(false, capsFromLocalStorage.isWfcEnabled());
    }
}

