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
package com.android.imsstack.core.agents.dcm;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.when;

import android.telephony.AccessNetworkConstants;
import android.telephony.CellIdentity;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityNr;
import android.telephony.CellIdentityWcdma;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.dcmif.IDcUtils;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.Log;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Collections;

@RunWith(JUnit4.class)
public class DcUtilsTest {
    private static final String TAG = DcUtilsTest.class.getSimpleName();
    private static final int SLOT_ID = 0;

    static ContextFixture sContext;

    @Mock ServiceState mServiceState;

    DcUtils mDcUtils;

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture();
        AppContext.init(sContext.getTestDouble());
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        TelephonyManager tm = sContext.getTestDouble().getSystemService(TelephonyManager.class);
        when(tm.getServiceState()).thenReturn(mServiceState);

        mDcUtils = new DcUtils(SLOT_ID);
        mDcUtils.init(AppContext.getInstance());
    }

    @After
    public void tearDown() throws Exception {
        if (mDcUtils != null) {
            mDcUtils.cleanup();
            mDcUtils = null;
        }
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfo_defaultNetworkTypeUnknown() throws Exception {
        int[] testNetworkTypes = {
            TelephonyManager.NETWORK_TYPE_LTE,
            TelephonyManager.NETWORK_TYPE_NR
        };

        int[] testDuplexModes = {
            ServiceState.DUPLEX_MODE_FDD,
            ServiceState.DUPLEX_MODE_TDD
        };

        for (int i = 0; i < testNetworkTypes.length; ++i) {
            NetworkRegistrationInfo nri = createNetworkRegistrationInfo(testNetworkTypes[i]);

            when(mServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(nri);
            when(mServiceState.getDuplexMode()).thenReturn(testDuplexModes[i]);

            IDcUtils.AccessNetworkInfo ani =
                    mDcUtils.getAccessNetworkInfo(TelephonyManager.NETWORK_TYPE_UNKNOWN);

            assertNotNull(ani);
            assertEquals(ani.mNetworkType, testNetworkTypes[i]);
        }
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfo_specificNetworkTypeLte() throws Exception {
        int testNetworkType = TelephonyManager.NETWORK_TYPE_LTE;
        NetworkRegistrationInfo nri = createNetworkRegistrationInfo(testNetworkType);

        when(mServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(nri);
        when(mServiceState.getDuplexMode()).thenReturn(ServiceState.DUPLEX_MODE_FDD);

        IDcUtils.AccessNetworkInfo ani = mDcUtils.getAccessNetworkInfo(testNetworkType);

        assertNotNull(ani);
        assertEquals(ani.mNetworkType, testNetworkType);

        CellIdentityLte ci = getCellIdentity(nri, CellIdentityLte.class);

        assertNotNull(ani.mAni);
        assertEquals(ani.mAni.length, DcUtils.ANI_ITEM_SIZE);
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MCC], ci.getMccString());
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MNC], ci.getMncString());
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_CELL_ID], Integer.toHexString(ci.getCi()));
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_TAC_OR_LAC], Integer.toHexString(ci.getTac()));
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MODE], DcUtils.MODE_FDD);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfo_specificNetworkTypeNr() throws Exception {
        int testNetworkType = TelephonyManager.NETWORK_TYPE_NR;
        NetworkRegistrationInfo nri = createNetworkRegistrationInfo(testNetworkType);

        when(mServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(nri);
        when(mServiceState.getDuplexMode()).thenReturn(ServiceState.DUPLEX_MODE_TDD);

        IDcUtils.AccessNetworkInfo ani = mDcUtils.getAccessNetworkInfo(testNetworkType);

        assertNotNull(ani);
        assertEquals(ani.mNetworkType, testNetworkType);

        CellIdentityNr ci = getCellIdentity(nri, CellIdentityNr.class);

        assertNotNull(ani.mAni);
        assertEquals(ani.mAni.length, DcUtils.ANI_ITEM_SIZE);
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MCC], ci.getMccString());
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MNC], ci.getMncString());
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_CELL_ID], Long.toHexString(ci.getNci()));
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_TAC_OR_LAC], Integer.toHexString(ci.getTac()));
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MODE], DcUtils.MODE_TDD);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfo_specificNetworkTypeWcdma() throws Exception {
        int testNetworkType = TelephonyManager.NETWORK_TYPE_UMTS;
        NetworkRegistrationInfo nri = createNetworkRegistrationInfo(testNetworkType);

        when(mServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(nri);

        IDcUtils.AccessNetworkInfo ani = mDcUtils.getAccessNetworkInfo(testNetworkType);

        assertNotNull(ani);
        assertEquals(ani.mNetworkType, testNetworkType);

        CellIdentityWcdma ci = getCellIdentity(nri, CellIdentityWcdma.class);

        assertNotNull(ani.mAni);
        assertEquals(ani.mAni.length, DcUtils.ANI_ITEM_SIZE);
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MCC], ci.getMccString());
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MNC], ci.getMncString());
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_CELL_ID], Long.toHexString(ci.getCid()));
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_TAC_OR_LAC], Integer.toHexString(ci.getLac()));
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MODE], "");
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfo_specificNetworkTypeGsm() throws Exception {
        int testNetworkType = TelephonyManager.NETWORK_TYPE_GPRS;
        NetworkRegistrationInfo nri = createNetworkRegistrationInfo(testNetworkType);

        when(mServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(nri);

        IDcUtils.AccessNetworkInfo ani = mDcUtils.getAccessNetworkInfo(testNetworkType);

        assertNotNull(ani);
        assertEquals(ani.mNetworkType, testNetworkType);

        CellIdentityGsm ci = getCellIdentity(nri, CellIdentityGsm.class);

        assertNotNull(ani.mAni);
        assertEquals(ani.mAni.length, DcUtils.ANI_ITEM_SIZE);
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MCC], ci.getMccString());
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MNC], ci.getMncString());
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_CELL_ID], Long.toHexString(ci.getCid()));
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_TAC_OR_LAC], Integer.toHexString(ci.getLac()));
        assertEquals(ani.mAni[DcUtils.ANI_INDEX_MODE], "");
    }

    private static NetworkRegistrationInfo createNetworkRegistrationInfo(int networkType) {
        return new NetworkRegistrationInfo.Builder()
                .setDomain(NetworkRegistrationInfo.DOMAIN_PS)
                .setTransportType(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)
                .setAccessNetworkTechnology(networkType)
                .setCellIdentity(createCellIdentity(networkType)).build();
    }

    private static CellIdentity createCellIdentity(int networkType) {
        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_LTE:
                return new CellIdentityLte(0x1111111, 13, 0x2222, 0, new int[] {}, 0,
                        "001", "01", "Test-SIM", "Test", Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_NR:
                return new CellIdentityNr(20, 0x333333, 2, new int[] {},
                        "001", "01", 0x555555555L, "Test-SIM", "Test", Collections.emptyList());
            case TelephonyManager.NETWORK_TYPE_UMTS: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSDPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSUPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                return new CellIdentityWcdma(0x6666, 0x7777777, 3, 0,
                        "001", "01", "Test-SIM", "Test", Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_GPRS: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_EDGE:
                return new CellIdentityGsm(0x8888, 0x9999, 0, 1,
                        "001", "01", "Test-SIM", "Test", Collections.emptyList());
            default:
                return null;
        }
    }

    private static <T> T getCellIdentity(NetworkRegistrationInfo nri, Class<T> clazz) {
        try {
            return (T) nri.getCellIdentity();
        } catch (ClassCastException e) {
            Log.d(TAG, "getCellIdentity: " + e);
            return null;
        }
    }
}
