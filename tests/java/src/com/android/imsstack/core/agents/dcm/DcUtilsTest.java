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
import static org.junit.Assert.assertNull;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.SharedPreferences;
import android.telephony.AccessNetworkConstants;
import android.telephony.CellIdentity;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityNr;
import android.telephony.CellIdentityWcdma;
import android.telephony.CellInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.dcmif.IDcUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Collections;

@RunWith(JUnit4.class)
public class DcUtilsTest {
    @Mock private ServiceState mServiceState;
    @Mock private SharedPreferences mSp;

    private TestAppContext mTestAppContext;
    private TelephonyManagerProxy mTelephonyManagerProxy;
    private DcUtils mDcUtils;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();

        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.getServiceState(anyInt())).thenReturn(mServiceState);

        mDcUtils = new DcUtils(TestAppContext.SLOT0);
        mDcUtils.init(mTestAppContext.getContext());
    }

    @After
    public void tearDown() throws Exception {
        if (mDcUtils != null) {
            mDcUtils.cleanup();
            mDcUtils = null;
        }

        mServiceState = null;
        mSp = null;
        mTelephonyManagerProxy = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfo_serviceStateNull() {
        when(mTelephonyManagerProxy.getServiceState(anyInt())).thenReturn(null);

        IDcUtils.AccessNetworkInfo ani =
                mDcUtils.getAccessNetworkInfo(TelephonyManager.NETWORK_TYPE_LTE);

        assertNotNull(ani);
        assertNull(ani.mAni);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfo_defaultNetworkTypeUnknown() {
        int[] testNetworkTypes = {
            TelephonyManager.NETWORK_TYPE_LTE,
            TelephonyManager.NETWORK_TYPE_NR
        };

        int[] testDuplexModes = {
            ServiceState.DUPLEX_MODE_FDD,
            ServiceState.DUPLEX_MODE_TDD
        };

        doReturn(mSp).when(mTestAppContext.getContext())
                .getSharedPreferences(anyString(), anyInt());

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
    public void getAccessNetworkInfo_specificNetworkTypeLte() {
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
    public void getAccessNetworkInfo_specificNetworkTypeNr() {
        int testNetworkType = TelephonyManager.NETWORK_TYPE_NR;
        NetworkRegistrationInfo nri = createNetworkRegistrationInfo(testNetworkType);

        when(mServiceState.getNetworkRegistrationInfo(anyInt(), anyInt())).thenReturn(nri);
        doReturn(mSp).when(mTestAppContext.getContext())
                .getSharedPreferences(anyString(), anyInt());

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
    public void getAccessNetworkInfo_specificNetworkTypeWcdma() {
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
    public void getAccessNetworkInfo_specificNetworkTypeGsm() {
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

    @Test
    @SmallTest
    public void isMobileDataEnabled() {
        mDcUtils.isMobileDataEnabled();
        verify(mTelephonyManagerProxy).isDataEnabled();
    }

    @Test
    @SmallTest
    public void updateAllCellInfoForcinglyOnLimitedServiceState() {
        mDcUtils.updateAllCellInfoForcinglyOnLimitedServiceState();
        // TODO: no actual implementations
    }

    @Test
    @SmallTest
    public void testGetServiceState() {
        ServiceState ss = mDcUtils.getServiceState();

        assertEquals(mServiceState, ss);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoFromCache_noCachedNetwork() {
        String[] networks = mDcUtils.getAccessNetworkInfoFromCache(
                TelephonyManager.NETWORK_TYPE_LTE);

        assertNull(networks);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoFromCache_cachedNetwork() {
        mDcUtils.storeAccessNetworkInfoToCache(TelephonyManager.NETWORK_TYPE_LTE,
                new String[] {"310"});

        String[] networks = mDcUtils.getAccessNetworkInfoFromCache(
                TelephonyManager.NETWORK_TYPE_LTE);

        assertNotNull(networks);
        assertEquals("310", networks[DcUtils.ANI_INDEX_MCC]);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForLte_noCellIdentity() {
        NetworkRegistrationInfo nri =
                createNetworkRegistrationInfoWithoutCellIdentity(TelephonyManager.NETWORK_TYPE_LTE);
        mDcUtils.storeAccessNetworkInfoToCache(TelephonyManager.NETWORK_TYPE_LTE,
                new String[] {"310"});

        String[] networks = mDcUtils.getAccessNetworkInfoForLte(nri, ServiceState.DUPLEX_MODE_TDD);

        assertNotNull(networks);
        assertEquals("310", networks[DcUtils.ANI_INDEX_MCC]);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForLte_invalidCellIdentity() {
        NetworkRegistrationInfo nri = createNetworkRegistrationInfoWithInvalidCellIdentity(
                TelephonyManager.NETWORK_TYPE_LTE);
        mDcUtils.storeAccessNetworkInfoToCache(TelephonyManager.NETWORK_TYPE_LTE,
                new String[] {"310"});

        String[] networks = mDcUtils.getAccessNetworkInfoForLte(nri, ServiceState.DUPLEX_MODE_TDD);

        assertNotNull(networks);
        assertEquals("310", networks[DcUtils.ANI_INDEX_MCC]);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForLte_lteAccessNetworkInfo() {
        NetworkRegistrationInfo nri = createNetworkRegistrationInfo(
                TelephonyManager.NETWORK_TYPE_LTE);
        CellIdentityLte ci = getCellIdentity(nri, CellIdentityLte.class);

        String[] networks = mDcUtils.getAccessNetworkInfoForLte(nri, ServiceState.DUPLEX_MODE_TDD);

        assertNotNull(networks);
        assertEquals(DcUtils.ANI_ITEM_SIZE, networks.length);
        assertEquals(ci.getMccString(), networks[DcUtils.ANI_INDEX_MCC]);
        assertEquals(ci.getMncString(), networks[DcUtils.ANI_INDEX_MNC]);
        assertEquals(Integer.toHexString(ci.getCi()), networks[DcUtils.ANI_INDEX_CELL_ID]);
        assertEquals(Integer.toHexString(ci.getTac()), networks[DcUtils.ANI_INDEX_TAC_OR_LAC]);
        assertEquals(DcUtils.MODE_TDD, networks[DcUtils.ANI_INDEX_MODE]);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForNr_noCellIdentity() {
        NetworkRegistrationInfo nri =
                createNetworkRegistrationInfoWithoutCellIdentity(TelephonyManager.NETWORK_TYPE_NR);
        mDcUtils.storeAccessNetworkInfoToCache(TelephonyManager.NETWORK_TYPE_NR,
                new String[] {"310"});

        String[] networks = mDcUtils.getAccessNetworkInfoForNr(nri);

        assertNotNull(networks);
        assertEquals("310", networks[DcUtils.ANI_INDEX_MCC]);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForNr_invalidCellIdentity() {
        NetworkRegistrationInfo nri = createNetworkRegistrationInfoWithInvalidCellIdentity(
                TelephonyManager.NETWORK_TYPE_NR);
        mDcUtils.storeAccessNetworkInfoToCache(TelephonyManager.NETWORK_TYPE_NR,
                new String[] {"310"});

        doReturn(mSp).when(mTestAppContext.getContext())
                .getSharedPreferences(anyString(), anyInt());

        String[] networks = mDcUtils.getAccessNetworkInfoForNr(nri);

        assertNotNull(networks);
        assertEquals("310", networks[DcUtils.ANI_INDEX_MCC]);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForNr_nrAccessNetworkInfo() {
        doReturn(mSp).when(mTestAppContext.getContext())
                .getSharedPreferences(anyString(), anyInt());

        NetworkRegistrationInfo nri = createNetworkRegistrationInfo(
                TelephonyManager.NETWORK_TYPE_NR);
        CellIdentityNr ci = getCellIdentity(nri, CellIdentityNr.class);

        String[] networks = mDcUtils.getAccessNetworkInfoForNr(nri);

        assertNotNull(networks);
        assertEquals(DcUtils.ANI_ITEM_SIZE, networks.length);
        assertEquals(ci.getMccString(), networks[DcUtils.ANI_INDEX_MCC]);
        assertEquals(ci.getMncString(), networks[DcUtils.ANI_INDEX_MNC]);
        assertEquals(Long.toHexString(ci.getNci()), networks[DcUtils.ANI_INDEX_CELL_ID]);
        assertEquals(Integer.toHexString(ci.getTac()), networks[DcUtils.ANI_INDEX_TAC_OR_LAC]);
        assertEquals(DcUtils.MODE_TDD, networks[DcUtils.ANI_INDEX_MODE]);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForWcdma_noCellIdentity() {
        NetworkRegistrationInfo nri = createNetworkRegistrationInfoWithoutCellIdentity(
                TelephonyManager.NETWORK_TYPE_UMTS);

        String[] networks = mDcUtils.getAccessNetworkInfoForWcdma(nri);

        assertNull(networks);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForWcdma_invalidCellIdentity() {
        NetworkRegistrationInfo nri = createNetworkRegistrationInfoWithInvalidCellIdentity(
                TelephonyManager.NETWORK_TYPE_UMTS);

        String[] networks = mDcUtils.getAccessNetworkInfoForWcdma(nri);

        assertNull(networks);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForWcdma_wcdmaAccessNetworkInfo() {
        NetworkRegistrationInfo nri = createNetworkRegistrationInfo(
                TelephonyManager.NETWORK_TYPE_UMTS);
        CellIdentityWcdma ci = getCellIdentity(nri, CellIdentityWcdma.class);

        String[] networks = mDcUtils.getAccessNetworkInfoForWcdma(nri);

        assertNotNull(networks);
        assertEquals(DcUtils.ANI_ITEM_SIZE, networks.length);
        assertEquals(ci.getMccString(), networks[DcUtils.ANI_INDEX_MCC]);
        assertEquals(ci.getMncString(), networks[DcUtils.ANI_INDEX_MNC]);
        assertEquals(Integer.toHexString(ci.getCid()), networks[DcUtils.ANI_INDEX_CELL_ID]);
        assertEquals(Integer.toHexString(ci.getLac()), networks[DcUtils.ANI_INDEX_TAC_OR_LAC]);
        assertEquals("", networks[DcUtils.ANI_INDEX_MODE]);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForGsm_noCellIdentity() {
        NetworkRegistrationInfo nri = createNetworkRegistrationInfoWithoutCellIdentity(
                TelephonyManager.NETWORK_TYPE_EDGE);

        String[] networks = mDcUtils.getAccessNetworkInfoForGsm(nri);

        assertNull(networks);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForGsm_invalidCellIdentity() {
        NetworkRegistrationInfo nri = createNetworkRegistrationInfoWithInvalidCellIdentity(
                TelephonyManager.NETWORK_TYPE_EDGE);

        String[] networks = mDcUtils.getAccessNetworkInfoForGsm(nri);

        assertNull(networks);
    }

    @Test
    @SmallTest
    public void getAccessNetworkInfoForGsm_wcdmaAccessNetworkInfo() {
        NetworkRegistrationInfo nri = createNetworkRegistrationInfo(
                TelephonyManager.NETWORK_TYPE_EDGE);
        CellIdentityGsm ci = getCellIdentity(nri, CellIdentityGsm.class);

        String[] networks = mDcUtils.getAccessNetworkInfoForGsm(nri);

        assertNotNull(networks);
        assertEquals(DcUtils.ANI_ITEM_SIZE, networks.length);
        assertEquals(ci.getMccString(), networks[DcUtils.ANI_INDEX_MCC]);
        assertEquals(ci.getMncString(), networks[DcUtils.ANI_INDEX_MNC]);
        assertEquals(Integer.toHexString(ci.getCid()), networks[DcUtils.ANI_INDEX_CELL_ID]);
        assertEquals(Integer.toHexString(ci.getLac()), networks[DcUtils.ANI_INDEX_TAC_OR_LAC]);
        assertEquals("", networks[DcUtils.ANI_INDEX_MODE]);
    }

    private NetworkRegistrationInfo createNetworkRegistrationInfo(int networkType) {
        return new NetworkRegistrationInfo.Builder()
                .setDomain(NetworkRegistrationInfo.DOMAIN_PS)
                .setTransportType(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)
                .setAccessNetworkTechnology(networkType)
                .setCellIdentity(createCellIdentity(networkType)).build();
    }

    private NetworkRegistrationInfo createNetworkRegistrationInfoWithInvalidCellIdentity(
            int networkType) {
        return new NetworkRegistrationInfo.Builder()
                .setDomain(NetworkRegistrationInfo.DOMAIN_PS)
                .setTransportType(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)
                .setAccessNetworkTechnology(networkType)
                .setCellIdentity(createInvalidCellIdentity(networkType)).build();
    }

    private NetworkRegistrationInfo createNetworkRegistrationInfoWithoutCellIdentity(
            int networkType) {
        return new NetworkRegistrationInfo.Builder()
                .setDomain(NetworkRegistrationInfo.DOMAIN_PS)
                .setTransportType(AccessNetworkConstants.TRANSPORT_TYPE_WWAN)
                .setAccessNetworkTechnology(networkType).build();
    }

    private CellIdentity createCellIdentity(int networkType) {
        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_LTE:
                return new CellIdentityLte(0x1111111, 13, 0x2222, 0, new int[] {}, 0,
                        "001", "01", "Test-SIM", "Test", Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_NR:
                return new CellIdentityNr(20, 0x333333, 633693, new int[] {1, 78},
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

    private CellIdentity createInvalidCellIdentity(int networkType) {
        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_LTE:
                return new CellIdentityLte(0x1111111, 13, 0x2222, 0, new int[] {}, 0, null, "01",
                        "Test-SIM", "Test", Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_NR:
                return new CellIdentityNr(20, 0x333333, 2, new int[] {}, "001", "01",
                        CellInfo.UNAVAILABLE_LONG, "Test-SIM", "Test", Collections.emptyList());
            case TelephonyManager.NETWORK_TYPE_UMTS: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSDPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSUPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSPA: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                return new CellIdentityWcdma(CellInfo.UNAVAILABLE, 0x7777777, 3, 0, "001", "01",
                        "Test-SIM", "Test", Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_GPRS: // FALL-THROUGH
            case TelephonyManager.NETWORK_TYPE_EDGE:
                return new CellIdentityGsm(0x8888, 0x9999, 0, 1, "001", null, "Test-SIM", "Test",
                        Collections.emptyList());
            default:
                return null;
        }
    }

    @SuppressWarnings("unchecked")
    private <T> T getCellIdentity(NetworkRegistrationInfo nri, Class<T> clazz) {
        try {
            return (T) nri.getCellIdentity();
        } catch (ClassCastException e) {
            fail("getCellIdentity : " + e + " class: " + clazz.getName());
            return null;
        }
    }
}
