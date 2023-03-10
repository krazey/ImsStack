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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.telephony.TelephonyManager;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.ImsStackTest;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.config.FeatureConfig;
import com.android.imsstack.imsservice.ImsServiceController;
import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.imsservice.mmtel.base.IMmTelCallListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.ConcurrentHashMap;

@RunWith(JUnit4.class)
public class ImsServiceManagerTest extends ImsStackTest {
    public static final int DEFAULT_PHONE_ID = 0;
    public static final int INVALID_PHONE_ID = (-1);
    public static final int SUBSCRIPTION = 1;
    private static Context sMockContext;
    private static ISubscription sMockISubscription;
    private ImsServiceRecord mMockServiceRecord;
    private ImsCallApp mMockImsCallApp;
    private TestImsServiceManager mServiceManager;
    private MessageExecutor mExecutor;
    private ConcurrentHashMap<Integer, ImsCallApp> mCallAppMap = null;
    private ConcurrentHashMap<Integer, ImsServiceRecord> mServiceRecordMap = null;

    static ContextFixture sContext;

    @Mock Context mMockContext;
    @Mock TelephonyManager mMockTelephonyManager;

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture();
        AppContext.init(sContext.getTestDouble());
        sMockISubscription = Mockito.mock(ISubscription.class);
        AgentFactory.getInstance().setDefaultAgent(SUBSCRIPTION, sMockISubscription);
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mMockTelephonyManager = sContext.getTestDouble().getSystemService(TelephonyManager.class);
        when(AppContext.getTelephonyManager()).thenReturn(mMockTelephonyManager);
        when(mMockTelephonyManager.getSupportedModemCount()).thenReturn(1);

        mMockServiceRecord = Mockito.mock(ImsServiceRecord.class);
        mMockImsCallApp = Mockito.mock(ImsCallApp.class);
        mExecutor = new MessageExecutor(ImsServiceController.class.getSimpleName());
        mServiceManager = new TestImsServiceManager(mMockContext, mExecutor);
        mCallAppMap = mServiceManager.getCallAppMap();
        mServiceRecordMap = mServiceManager.getServiceRecordMap();
        when(sMockISubscription.getPhoneId()).thenReturn(DEFAULT_PHONE_ID);
    }

    @After
    public void tearDown() throws Exception {
        mServiceRecordMap.clear();
        mCallAppMap.clear();
        mServiceManager.setDefault(null);
        mServiceManager = null;
        mCallAppMap = null;
        mServiceRecordMap = null;
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
        AgentFactory.getInstance().setDefaultAgent(SUBSCRIPTION, null);
    }

    @Test
    public void testConstructor() {
        verify(sMockISubscription).addListener(mServiceManager.getSubscriptionListenerProxy());
        assertEquals(1, mServiceRecordMap.size());
        assertNotNull(mServiceManager.getSubscriptionListenerProxy());
        assertNotNull(mServiceManager.getCommonPackageListener());
    }

    @Test
    public void testGetSetDefault() {
        assertNull(mServiceManager.getDefault());
        mServiceManager.setDefault(mServiceManager);
        assertEquals(mServiceManager, mServiceManager.getDefault());
    }

    @Test
    public void testGetServiceRecord() {
        assertNull(mServiceManager.getServiceRecord(DEFAULT_PHONE_ID));
        mServiceManager.setDefault(mServiceManager);
        assertEquals(1, mServiceRecordMap.size());
        assertEquals(mServiceRecordMap.get(DEFAULT_PHONE_ID),
                mServiceManager.getServiceRecord(DEFAULT_PHONE_ID));
    }

    @Test
    public void testDispose() {
        mServiceManager.dispose();
        verify(sMockISubscription).removeListener(mServiceManager.getSubscriptionListenerProxy());
        verify(mMockImsCallApp, never()).close();
        assertTrue(mCallAppMap.isEmpty());

        mCallAppMap.put(DEFAULT_PHONE_ID, mMockImsCallApp);
        assertEquals(1, mCallAppMap.size());

        mServiceRecordMap.clear();
        assertTrue(mServiceRecordMap.isEmpty());
        mServiceRecordMap.put(DEFAULT_PHONE_ID, mMockServiceRecord);
        when(mMockServiceRecord.isServiceUp()).thenReturn(true);
        mServiceManager.dispose();
        verify(mMockImsCallApp).close();
        assertTrue(mCallAppMap.isEmpty());
        verify(mMockServiceRecord).setCallApp(null);
        verify(mMockServiceRecord).broadcastServiceDown();
        assertTrue(mServiceRecordMap.isEmpty());
    }

    @Test
    public void testCreateCallApp() {
        IMmTelCallListener mockCallListener = Mockito.mock(IMmTelCallListener.class);
        IMmTelFeatureCapabilityListener mockFeatureCapaListener =
                Mockito.mock(IMmTelFeatureCapabilityListener.class);
        ImsContext mockImsContext = Mockito.mock(ImsContext.class);
        mServiceRecordMap.clear();
        when(mockImsContext.getPhoneId()).thenReturn(DEFAULT_PHONE_ID);

        ImsCallApp callApp = mServiceManager.createCallApp(mockImsContext, mockFeatureCapaListener,
                mockCallListener);
        assertNull(callApp);

        mServiceRecordMap.put(DEFAULT_PHONE_ID, mMockServiceRecord);
        callApp = mServiceManager.createCallApp(mockImsContext, mockFeatureCapaListener,
                mockCallListener);
        assertEquals(1, mCallAppMap.size());
        verify(mMockServiceRecord).setCallApp(any(ImsCallApp.class));
        assertNotNull(callApp);

        callApp = mServiceManager.createCallApp(mockImsContext, mockFeatureCapaListener,
                mockCallListener);
        verify(callApp).unbindCallApp();
        verify(callApp).bindCallApp();

        mServiceManager.setMultiImsEnabled(false);
        callApp = mServiceManager.createCallApp(mockImsContext, mockFeatureCapaListener,
                mockCallListener);
        verify(callApp).bindCallApp();
    }

    @Test
    public void testDestroyCallApp() {
        mCallAppMap.clear();
        mServiceRecordMap.clear();
        mServiceManager.destroyCallApp(DEFAULT_PHONE_ID);
        verify(mMockImsCallApp, never()).close();

        mCallAppMap.put(DEFAULT_PHONE_ID, mMockImsCallApp);
        mServiceRecordMap.put(DEFAULT_PHONE_ID, mMockServiceRecord);
        mServiceManager.destroyCallApp(DEFAULT_PHONE_ID);
        verify(mMockServiceRecord).setCallApp(null);
        verify(mMockImsCallApp).close();
        assertTrue(mCallAppMap.isEmpty());
    }

    @Test
    public void testGetCallApp() {
        mCallAppMap.clear();
        ImsCallApp callApp = mServiceManager.getCallApp(DEFAULT_PHONE_ID);
        assertNull(callApp);

        mCallAppMap.put(DEFAULT_PHONE_ID, mMockImsCallApp);
        callApp = mServiceManager.getCallApp(DEFAULT_PHONE_ID);
        assertEquals(mMockImsCallApp, callApp);
    }

    @Test
    public void testGetCallAppByPhoneId() {
        mServiceManager.setMultiImsEnabled(false);
        ImsCallApp callApp = mServiceManager.getCallAppByPhoneId(INVALID_PHONE_ID);
        assertNull(callApp);

        mServiceRecordMap.clear();
        callApp = mServiceManager.getCallAppByPhoneId(DEFAULT_PHONE_ID);
        assertNull(callApp);

        mServiceRecordMap.put(DEFAULT_PHONE_ID, mMockServiceRecord);
        when(mMockServiceRecord.getCallApp()).thenReturn(mMockImsCallApp);
        callApp = mServiceManager.getCallAppByPhoneId(DEFAULT_PHONE_ID);
        verify(mMockServiceRecord).getCallApp();
        assertEquals(mMockImsCallApp, callApp);
    }

    @Test
    public void testGetCallAppCount() {
        assertEquals(0, mServiceManager.getCallAppCount());
        mCallAppMap.put(DEFAULT_PHONE_ID, mMockImsCallApp);
        assertEquals(1, mServiceManager.getCallAppCount());
    }

    @Test
    public void testGetContext() {
        assertEquals(mMockContext, mServiceManager.getContext());
    }

    @Test
    public void testgetDefaultPhoneId() {
        assertEquals(DEFAULT_PHONE_ID, mServiceManager.getDefaultPhoneId());
        when(sMockISubscription.getPhoneId()).thenReturn(INVALID_PHONE_ID);
        mServiceManager = new TestImsServiceManager(mMockContext, mExecutor);
        assertEquals(INVALID_PHONE_ID, mServiceManager.getDefaultPhoneId());
    }

    @Test
    public void testgetDefaultServiceRecord() {
        mServiceRecordMap.clear();
        mServiceRecordMap.put(DEFAULT_PHONE_ID, mMockServiceRecord);
        assertEquals(mMockServiceRecord, mServiceManager.getDefaultServiceRecord());
    }

    @Test
    public void testIsValidPhoneId() {
        assertTrue(mServiceManager.isValidPhoneId(DEFAULT_PHONE_ID));
        assertFalse(mServiceManager.isValidPhoneId(INVALID_PHONE_ID));

        mServiceManager.setMultiImsEnabled(false);
        assertFalse(mServiceManager.isValidPhoneId(INVALID_PHONE_ID));
        assertTrue(mServiceManager.isValidPhoneId(DEFAULT_PHONE_ID));
    }

    @Test
    public void testOnDefaultSubscriptionChanged() {
        mServiceRecordMap.clear();
        mServiceRecordMap.put(DEFAULT_PHONE_ID, mMockServiceRecord);
        mServiceManager.getSubscriptionListenerProxy()
            .onDefaultSubscriptionChanged(DEFAULT_PHONE_ID);
        verify(mMockServiceRecord, never()).getCallApp();
        //switchImsService() is verified in below test
    }

    @Test
    public void testOnDefaultDataSubscriptionChanged() {
        int newPhoneId = 1;
        ImsServiceRecord mockServiceRecord1 = Mockito.mock(ImsServiceRecord.class);
        ImsCallApp mockImsCallApp1 = Mockito.mock(ImsCallApp.class);
        setUpOnDefaultDataSubscriptionChanged(newPhoneId, mockServiceRecord1);
        when(mMockServiceRecord.getCallApp()).thenReturn(mMockImsCallApp);
        when(mockServiceRecord1.getCallApp()).thenReturn(mockImsCallApp1);
        when(mMockServiceRecord.isServiceUp()).thenReturn(false);

        mServiceManager.getSubscriptionListenerProxy()
            .onDefaultDataSubscriptionChanged(newPhoneId);
        verify(mMockServiceRecord).getCallApp();
        verify(mockServiceRecord1).getCallApp();
        verify(mMockImsCallApp, never()).unbindCallApp();
        verify(mMockServiceRecord, never()).broadcastServiceDown();
        verify(mockServiceRecord1).broadcastServiceUp();
        verify(mockImsCallApp1).bindCallApp();

        mServiceManager = new TestImsServiceManager(mMockContext, mExecutor);
        setUpOnDefaultDataSubscriptionChanged(newPhoneId, mockServiceRecord1);
        when(mMockServiceRecord.isServiceUp()).thenReturn(true);

        mServiceManager.getSubscriptionListenerProxy()
            .onDefaultDataSubscriptionChanged(newPhoneId);
        verify(mMockServiceRecord, times(2)).getCallApp();
        verify(mockServiceRecord1, times(2)).getCallApp();
        verify(mMockImsCallApp).unbindCallApp();
        verify(mMockServiceRecord).broadcastServiceDown();
        verify(mockServiceRecord1, times(2)).broadcastServiceUp();
        verify(mockImsCallApp1, times(2)).bindCallApp();
    }

    private void setUpOnDefaultDataSubscriptionChanged(int phoneId, ImsServiceRecord sr) {
        mServiceRecordMap = mServiceManager.getServiceRecordMap();
        mServiceRecordMap.clear();
        mServiceRecordMap.put(DEFAULT_PHONE_ID, mMockServiceRecord);
        mServiceRecordMap.put(phoneId, sr);
        mServiceManager.setMultiImsEnabled(false);
        mServiceManager.setPhoneId(1);
    }

    @Test
    public void testOnCarrierConfigChanged() {
        int newPhoneId = -1;
        mServiceRecordMap.clear();
        mServiceRecordMap.put(DEFAULT_PHONE_ID, mMockServiceRecord);
        mServiceManager.getSubscriptionListenerProxy()
            .onCarrierConfigChanged(newPhoneId, 1);
        verify(mMockServiceRecord, never()).getCallApp();
        //ImsConstants.USE_CARRIER_CONFIG always true so required other code .?
    }

    private class TestImsServiceManager extends ImsServiceManager {
        int mSimPhoneId = 0;
        boolean mIsMultiImsEnabled = true;

        TestImsServiceManager(Context context, MessageExecutor executor) {
            super(context, executor);
        }

        public SubscriptionListenerProxy getSubscriptionListenerProxy() {
            return mSubscriptionListener;
        }

        public CommonPackageListener getCommonPackageListener() {
            return mCommonPackageListener;
        }

        @Override
        public int getVoLteServiceFeatures(int phoneId) {
            int volteServiceFeatures = 0;
            volteServiceFeatures |= FeatureConfig.FEATURE_S_VOLTE;
            return volteServiceFeatures;
        }

        @Override
        protected ImsCallApp createCallAppInternal(ImsContext imsContext, ImsServiceRecord isr,
                IMmTelFeatureCapabilityListener featureCapabilityListener,
                IMmTelCallListener callListener) {
            return mMockImsCallApp;
        }

        @Override
        protected boolean isMultiImsEnabled() {
            return mIsMultiImsEnabled;
        }

        @Override
        protected int getPhoneIdFromMSimUtils(int subId) {
            return mSimPhoneId;
        }

        public void setMultiImsEnabled(boolean value) {
            mIsMultiImsEnabled = value;
        }

        public void setPhoneId(int phoneId) {
            mSimPhoneId = phoneId;
        }
    }
}
