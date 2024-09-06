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
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.SharedPreferences;
import android.telephony.CarrierConfigManager;
import android.telephony.CellIdentity;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityNr;
import android.telephony.CellIdentityWcdma;
import android.telephony.CellInfo;
import android.telephony.CellInfoGsm;
import android.telephony.CellInfoLte;
import android.telephony.CellInfoNr;
import android.telephony.CellInfoWcdma;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.Executor;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class CellInfoAgentTest {
    private static final long TIMESTAMP = 55004107851687L;
    private static final long RECENT_TIMESTAMP = 55004907851687L;
    private static final String LAST_ANI =
            "13,2023-05-27T10:25:45Z,1685183145247,001,01,d8237,b112,FDD";

    @Mock private SharedPreferences mSp;
    @Mock private SharedPreferences.Editor mSpEditor;
    @Mock private ConfigInterface mConfigInterface;
    @Mock private CarrierConfig mCarrierConfig;
    @Mock private SimInterface mSimInterface;
    @Mock private IDcNetWatcher mDcNetWatcher;

    private final List<CellInfo> mCellInfos = new ArrayList<>();
    private ContextFixture mContextFixture;
    private TestAppContext mTestAppContext;
    private TelephonyManagerProxy mTelephonyManagerProxy;
    private TestableLooper mTestableLooper;
    private CellInfoAgent mCellInfoAgent;
    private boolean mInvalidMcc;
    private boolean mInvalidMnc;
    private boolean mInvalidCellId;
    private boolean mInvalidLacOrTac;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestableLooper = TestableLooper.get(this);
        mContextFixture = new ContextFixture();
        mTestAppContext = new TestAppContext(mContextFixture.getTestDouble());
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());

        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);

        when(mTestAppContext.getContext().getSharedPreferences(anyString(), anyInt()))
                .thenReturn(mSp);
        when(mSp.getString(eq(ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO),
                anyString())).thenReturn("");
        when(mSp.edit()).thenReturn(mSpEditor);

        AgentFactory.getInstance().setAgent(ConfigInterface.class, mConfigInterface, SLOT0);
        when(mSimInterface.getSubId()).thenReturn(SUB_ID_1);
        AgentFactory.getInstance().setAgent(SimInterface.class, mSimInterface, SLOT0);
        when(mDcNetWatcher.getVoiceNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        when(mDcNetWatcher.getVoiceNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_LTE);
        DcFactory.setDcAgent(IDcNetWatcher.class, mDcNetWatcher, SLOT0);

        mCellInfoAgent = new CellInfoAgent(SLOT0);
        mCellInfoAgent.init(mTestAppContext.getContext());
    }

    @After
    public void tearDown() throws Exception {
        if (mCellInfoAgent != null) {
            mCellInfoAgent.cleanup();
            mCellInfoAgent = null;
        }

        mCellInfos.clear();

        mInvalidMcc = false;
        mInvalidMnc = false;
        mInvalidCellId = false;
        mInvalidLacOrTac = false;
        DcFactory.setDcAgent(IDcNetWatcher.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        mConfigInterface = null;
        mCarrierConfig = null;
        mSimInterface = null;
        mDcNetWatcher = null;
        mSpEditor = null;
        mSp = null;
        mTelephonyManagerProxy = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
        mContextFixture = null;
        mTestableLooper = null;
    }

    @Test
    @SmallTest
    public void testGetAccessNetworkInfoWhenNoCellInfo() {
        assertNull(mCellInfoAgent.getAccessNetworkInfo());
        assertNull(mCellInfoAgent.getAccessNetworkInfo(TelephonyManager.NETWORK_TYPE_LTE));

        when(mSp.getString(eq(ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO),
                anyString())).thenReturn(LAST_ANI);

        String[] expectedAni = LAST_ANI.split(",");
        String[] ani = mCellInfoAgent.getAccessNetworkInfo();

        assertNotNull(expectedAni);
        assertNotNull(ani);
        // cell-info-age will be different according to the method call.
        expectedAni[CellInfoAgent.ANI_INDEX_CELL_INFO_AGE] = "";
        ani[CellInfoAgent.ANI_INDEX_CELL_INFO_AGE] = "";
        assertEquals(expectedAni, ani);
    }

    @Test
    @SmallTest
    public void testStartTrackingCellInfo() {
        mCellInfoAgent.startTrackingCellInfo();
        // No additional actions because it's already started.
        mCellInfoAgent.startTrackingCellInfo();
        mCellInfoAgent.stopTrackingCellInfo();

        verify(mTelephonyManagerProxy).registerTelephonyCallback(any(Executor.class),
                any(TelephonyCallback.class));
        verify(mDcNetWatcher).addListener(any(IDcNetWatcher.Listener.class));
        verify(mTelephonyManagerProxy).unregisterTelephonyCallback(any(TelephonyCallback.class));
        verify(mDcNetWatcher).removeListener(any(IDcNetWatcher.Listener.class));
    }

    @Test
    @SmallTest
    public void testStartTrackingCellInfoWhenSimInterfaceNull() {
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT0);
        mCellInfoAgent.startTrackingCellInfo();

        verify(mTelephonyManagerProxy, never())
                .registerTelephonyCallback(any(Executor.class), any(TelephonyCallback.class));
        verify(mDcNetWatcher, never()).addListener(any(IDcNetWatcher.Listener.class));
    }

    @Test
    @SmallTest
    public void testStopTrackingCellInfoWhenNotStarted() {
        mCellInfoAgent.stopTrackingCellInfo();

        verify(mTelephonyManagerProxy, never())
                .unregisterTelephonyCallback(any(TelephonyCallback.class));
        verify(mDcNetWatcher, never()).removeListener(any(IDcNetWatcher.Listener.class));
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoUsingTelephonyCallbackForEutran() {
        testUpdateAllCellInfoUsingTelephonyCallback(TelephonyManager.NETWORK_TYPE_LTE, false);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoUsingTelephonyCallbackForNgran() {
        testUpdateAllCellInfoUsingTelephonyCallback(TelephonyManager.NETWORK_TYPE_NR, false);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoUsingTelephonyCallbackForUtran() {
        testUpdateAllCellInfoUsingTelephonyCallback(TelephonyManager.NETWORK_TYPE_UMTS, false);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoUsingTelephonyCallbackForGeran() {
        testUpdateAllCellInfoUsingTelephonyCallback(TelephonyManager.NETWORK_TYPE_GPRS, false);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoWithInvalidCellIdentityForEutran() {
        testUpdateAllCellInfoWithInvalidCellIdentity(TelephonyManager.NETWORK_TYPE_LTE);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoWithInvalidCellIdentityForNgran() {
        testUpdateAllCellInfoWithInvalidCellIdentity(TelephonyManager.NETWORK_TYPE_NR);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoWithInvalidCellIdentityForUtran() {
        testUpdateAllCellInfoWithInvalidCellIdentity(TelephonyManager.NETWORK_TYPE_UMTS);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoWithInvalidCellIdentityForGeran() {
        testUpdateAllCellInfoWithInvalidCellIdentity(TelephonyManager.NETWORK_TYPE_GPRS);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoWithSameTimestampForEutranAndUtran() {
        // Expected result: LTE
        testUpdateAllCellInfoUsingTelephonyCallback(
                TelephonyManager.NETWORK_TYPE_UMTS, TelephonyManager.NETWORK_TYPE_LTE,
                TIMESTAMP, TIMESTAMP);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoWithDifferentTimestampForEutranAndUtran() {
        // Expected result: UMTS
        testUpdateAllCellInfoUsingTelephonyCallback(
                TelephonyManager.NETWORK_TYPE_UMTS, TelephonyManager.NETWORK_TYPE_LTE,
                RECENT_TIMESTAMP, TIMESTAMP);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoWithTimeOffsetEnabledForEutran() {
        testUpdateAllCellInfoUsingTelephonyCallback(TelephonyManager.NETWORK_TYPE_LTE, true);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoFromCellInfoCallback() {
        setUpAllCellInfo(TelephonyManager.NETWORK_TYPE_UNKNOWN, true);

        mCellInfoAgent.startTrackingCellInfo();
        processAllMessages();

        ArgumentCaptor<TelephonyManager.CellInfoCallback> captor =
                ArgumentCaptor.forClass(TelephonyManager.CellInfoCallback.class);
        verify(mTelephonyManagerProxy).requestCellInfoUpdate(any(Executor.class), captor.capture());

        TelephonyManager.CellInfoCallback callback =
                (TelephonyManager.CellInfoCallback) captor.getValue();
        setUpAllCellInfo(TelephonyManager.NETWORK_TYPE_LTE, true);
        callback.onCellInfo(mCellInfos);

        verify(mSpEditor).putString(
                eq(ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO), anyString());
        verify(mSpEditor).commit();

        mCellInfos.clear();
        callback.onCellInfo(mCellInfos);

        verifyNoMoreInteractions(mSpEditor);
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoWhenDataNetworkTypeChanged() {
        setUpAllCellInfo(TelephonyManager.NETWORK_TYPE_LTE, true);
        mCellInfoAgent.startTrackingCellInfo();
        ArgumentCaptor<IDcNetWatcher.Listener> captor =
                ArgumentCaptor.forClass(IDcNetWatcher.Listener.class);
        verify(mDcNetWatcher).addListener(captor.capture());
        IDcNetWatcher.Listener listener = captor.getValue();
        assertNotNull(listener);

        listener.onDataNetworkTypeChanged();
        processAllMessages();

        verify(mSpEditor, times(2)).putString(
                eq(ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO), anyString());
        verify(mSpEditor, times(2)).commit();
    }

    @Test
    @SmallTest
    public void testUpdateAllCellInfoWhenVoiceNetworkTypeChanged() {
        setUpAllCellInfo(TelephonyManager.NETWORK_TYPE_LTE, true);
        mCellInfoAgent.startTrackingCellInfo();
        ArgumentCaptor<IDcNetWatcher.Listener> captor =
                ArgumentCaptor.forClass(IDcNetWatcher.Listener.class);
        verify(mDcNetWatcher).addListener(captor.capture());
        IDcNetWatcher.Listener listener = captor.getValue();
        assertNotNull(listener);

        listener.onVoiceNetworkTypeChanged();
        processAllMessages();

        verify(mSpEditor, times(2)).putString(
                eq(ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO), anyString());
        verify(mSpEditor, times(2)).commit();
    }

    @Test
    @SmallTest
    public void testGetAccessNetworkInfoWhenCarrierConfigChangedWithUtcOffsetEnabled() {
        setUpAllCellInfo(TelephonyManager.NETWORK_TYPE_LTE, true);
        mCellInfoAgent.startTrackingCellInfo();
        processAllMessages();
        when(mConfigInterface.getCarrierConfig()).thenReturn(mCarrierConfig);
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_CELLULAR_NETWORK_INFO_UTC_OFFSET_ENABLED_BOOL)))
                .thenReturn(true);

        ArgumentCaptor<ConfigInterface.Listener> configListenerCaptor =
                ArgumentCaptor.forClass(ConfigInterface.Listener.class);
        verify(mConfigInterface).addListener(configListenerCaptor.capture());
        ConfigInterface.Listener listener = configListenerCaptor.getValue();
        listener.onCarrierConfigChanged(SLOT0, SUB_ID_1);

        String[] defaultAni = mCellInfoAgent.getAccessNetworkInfo();
        assertFalse(defaultAni[CellInfoAgent.ANI_INDEX_UTC_TIME_FORMAT].endsWith("Z"));
    }

    @Test
    @SmallTest
    public void testGetAccessNetworkInfoWhenCarrierConfigChangedWithUtcOffsetDisabled() {
        setUpAllCellInfo(TelephonyManager.NETWORK_TYPE_LTE, true);
        mCellInfoAgent.startTrackingCellInfo();
        processAllMessages();
        when(mConfigInterface.getCarrierConfig()).thenReturn(mCarrierConfig);
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_CELLULAR_NETWORK_INFO_UTC_OFFSET_ENABLED_BOOL)))
                .thenReturn(false);

        ArgumentCaptor<ConfigInterface.Listener> configListenerCaptor =
                ArgumentCaptor.forClass(ConfigInterface.Listener.class);
        verify(mConfigInterface).addListener(configListenerCaptor.capture());
        ConfigInterface.Listener listener = configListenerCaptor.getValue();
        listener.onCarrierConfigChanged(SLOT0, SUB_ID_1);

        String[] defaultAni = mCellInfoAgent.getAccessNetworkInfo();
        assertTrue(defaultAni[CellInfoAgent.ANI_INDEX_UTC_TIME_FORMAT].endsWith("Z"));
    }

    private void testUpdateAllCellInfoWithInvalidCellIdentity(int networkType) {
        setUpVoNrEnabled(networkType == TelephonyManager.NETWORK_TYPE_NR);
        setUpAllCellInfo(networkType, false);

        mCellInfoAgent.startTrackingCellInfo();
        processAllMessages();

        verify(mSpEditor, never()).putString(
                eq(ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO), anyString());
    }

    private void testUpdateAllCellInfoUsingTelephonyCallback(int networkType,
            boolean timeOffsetEnabled) {
        setUpVoNrEnabled(networkType == TelephonyManager.NETWORK_TYPE_NR);
        mCellInfoAgent.setTimeOffsetEnabledForUtcTimeFormat(timeOffsetEnabled);
        mCellInfoAgent.startTrackingCellInfo();
        notifyCellInfoChanged(networkType, TelephonyManager.NETWORK_TYPE_UNKNOWN,
                TIMESTAMP, TIMESTAMP);

        verify(mSpEditor).putString(
                eq(ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO), anyString());

        String[] defaultAni = mCellInfoAgent.getAccessNetworkInfo();
        String[] specificAni = mCellInfoAgent.getAccessNetworkInfo(networkType);
        assertNotNull(defaultAni);
        assertNotNull(specificAni);
        assertEquals(defaultAni, specificAni);
        assertEquals(String.valueOf(networkType), defaultAni[CellInfoAgent.ANI_INDEX_NETWORK_TYPE]);

        if (timeOffsetEnabled) {
            assertFalse(defaultAni[CellInfoAgent.ANI_INDEX_UTC_TIME_FORMAT].endsWith("Z"));
        } else {
            assertTrue(defaultAni[CellInfoAgent.ANI_INDEX_UTC_TIME_FORMAT].endsWith("Z"));
        }
    }

    private void testUpdateAllCellInfoUsingTelephonyCallback(
            int networkType1, int networkType2, long timestamp1, long timestamp2) {
        setUpVoNrEnabled((networkType1 == TelephonyManager.NETWORK_TYPE_NR)
                || (networkType2 == TelephonyManager.NETWORK_TYPE_NR));

        mCellInfoAgent.startTrackingCellInfo();
        notifyCellInfoChanged(networkType1, networkType2, timestamp1, timestamp2);

        verify(mSpEditor).putString(
                eq(ImsPrivateProperties.Persistent.KEY_LAST_ACCESS_NETWORK_INFO), anyString());

        int networkType;
        if (timestamp1 > timestamp2) {
            networkType = networkType1;
        } else if (timestamp1 < timestamp2) {
            networkType = networkType2;
        } else {
            networkType = (networkType1 > networkType2) ? networkType1 : networkType2;
        }

        String[] defaultAni = mCellInfoAgent.getAccessNetworkInfo();
        String[] specificAni = mCellInfoAgent.getAccessNetworkInfo(networkType);
        assertNotNull(defaultAni);
        assertNotNull(specificAni);
        assertEquals(defaultAni, specificAni);
        assertEquals(String.valueOf(networkType), defaultAni[CellInfoAgent.ANI_INDEX_NETWORK_TYPE]);
    }

    private void setUpAllCellInfo(int networkType, boolean validCellIdentity) {
        if (networkType != TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            if (validCellIdentity) {
                mCellInfos.add(createCellInfo(networkType, true, TIMESTAMP));
            } else {
                mInvalidMcc = true;
                mCellInfos.add(createCellInfo(networkType, true, TIMESTAMP));
                mInvalidMcc = false;

                mInvalidMnc = true;
                mCellInfos.add(createCellInfo(networkType, true, TIMESTAMP));
                mInvalidMnc = false;

                mInvalidCellId = true;
                mCellInfos.add(createCellInfo(networkType, true, TIMESTAMP));
                mInvalidCellId = false;

                mInvalidLacOrTac = true;
                mCellInfos.add(createCellInfo(networkType, true, TIMESTAMP));
                mInvalidLacOrTac = false;
            }
        }
        when(mTelephonyManagerProxy.getAllCellInfo()).thenReturn(mCellInfos);
    }

    private void notifyCellInfoChanged(int networkType1, int networkType2,
            long timestamp1, long timestamp2) {
        if (networkType1 != TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            mCellInfos.add(createCellInfo(networkType1, true, timestamp1));
        }
        if (networkType2 != TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            mCellInfos.add(createCellInfo(networkType2, true, timestamp2));
        }
        ArgumentCaptor<TelephonyCallback> captor = ArgumentCaptor.forClass(TelephonyCallback.class);
        verify(mTelephonyManagerProxy).registerTelephonyCallback(any(), captor.capture());
        TelephonyCallback.CellInfoListener cellInfoListener =
                (TelephonyCallback.CellInfoListener) captor.getValue();
        cellInfoListener.onCellInfoChanged(mCellInfos);
    }

    private void setUpVoNrEnabled(boolean enabled) {
        when(mConfigInterface.getCarrierConfig()).thenReturn(mCarrierConfig);
        when(mCarrierConfig.getIntArray(
                eq(CarrierConfigManager.KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY)))
                .thenReturn(enabled
                        ? new int[] { CarrierConfigManager.CARRIER_NR_AVAILABILITY_SA }
                        : null);
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }

    private CellInfo createCellInfo(int networkType, boolean registered, long timestamp) {
        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_LTE:
                return new CellInfoLte(CellInfo.CONNECTION_PRIMARY_SERVING, registered, timestamp,
                        (CellIdentityLte) createCellIdentity(networkType), null, null);
            case TelephonyManager.NETWORK_TYPE_NR:
                return new CellInfoNr(CellInfo.CONNECTION_PRIMARY_SERVING, registered, timestamp,
                        (CellIdentityNr) createCellIdentity(networkType), null);
            case TelephonyManager.NETWORK_TYPE_UMTS:
                return new CellInfoWcdma(CellInfo.CONNECTION_PRIMARY_SERVING, registered, timestamp,
                        (CellIdentityWcdma) createCellIdentity(networkType), null);
            case TelephonyManager.NETWORK_TYPE_GPRS:
                return new CellInfoGsm(CellInfo.CONNECTION_PRIMARY_SERVING, registered, timestamp,
                        (CellIdentityGsm) createCellIdentity(networkType), null);
            default:
                return null;
        }
    }

    private CellIdentity createCellIdentity(int networkType) {
        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_LTE:
                return new CellIdentityLte(
                        mInvalidCellId ? CellInfo.UNAVAILABLE : 0x1111111, 13,
                        mInvalidLacOrTac ? CellInfo.UNAVAILABLE : 0x2222, 0, new int[] {}, 0,
                        mInvalidMcc ? "" : "001",
                        mInvalidMnc ? "" : "01", "Test-SIM", "Test", Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_NR:
                return new CellIdentityNr(20,
                        mInvalidLacOrTac ? CellInfo.UNAVAILABLE : 0x333333,
                        633693, new int[] {1, 78},
                        mInvalidMcc ? "" : "001",
                        mInvalidMnc ? "" : "01",
                        mInvalidCellId ? CellInfo.UNAVAILABLE_LONG : 0x555555555L,
                        "Test-SIM", "Test", Collections.emptyList());
            case TelephonyManager.NETWORK_TYPE_UMTS:
                return new CellIdentityWcdma(
                        mInvalidLacOrTac ? CellInfo.UNAVAILABLE : 0x6666,
                        mInvalidCellId ? CellInfo.UNAVAILABLE : 0x7777777, 3, 0,
                        mInvalidMcc ? "" : "001",
                        mInvalidMnc ? "" : "01", "Test-SIM", "Test", Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_GPRS:
                return new CellIdentityGsm(
                        mInvalidLacOrTac ? CellInfo.UNAVAILABLE : 0x8888,
                        mInvalidCellId ? CellInfo.UNAVAILABLE : 0x9999, 0, 1,
                        mInvalidMcc ? "" : "001",
                        mInvalidMnc ? "" : "01", "Test-SIM", "Test", Collections.emptyList());
            default:
                return null;
        }
    }
}
