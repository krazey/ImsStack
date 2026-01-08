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
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.ContentProviderProxy.SettingsProxy;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.config.FeatureConfig;
import com.android.imsstack.imsservice.ImsServiceController;
import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.imsservice.mmtel.base.IMmTelCallListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.internal.ImsStackRegistry;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.ConcurrentHashMap;

@RunWith(JUnit4.class)
public class ImsServiceManagerTest extends ImsStackTest {
    @Mock private ImsServiceRecord mMockServiceRecord;
    @Mock private ImsCallApp mMockImsCallApp;
    @Mock private SettingsProxy mGlobalSettings;

    private TestAppContext mTestAppContext;
    private MessageExecutor mExecutor;
    private ConcurrentHashMap<Integer, ImsCallApp> mCallAppMap = null;
    private ConcurrentHashMap<Integer, ImsServiceRecord> mServiceRecordMap = null;
    private TestImsServiceManager mServiceManager;

    @Before
    public void setUp() throws Exception {
        super.setUp(ImsServiceManagerTest.class.getSimpleName());
        MockitoAnnotations.initMocks(this);
        mTestAppContext = new TestAppContext(mContext);
        mTestAppContext.setUp();

        when(mTestAppContext.getContentProviderProxy().getGlobalSettings())
                .thenReturn(mGlobalSettings);
        mExecutor = new MessageExecutor(ImsServiceController.class.getSimpleName());
        mServiceManager = new TestImsServiceManager(mContext, mExecutor);
        mCallAppMap = mServiceManager.getCallAppMap();
        mServiceRecordMap = mServiceManager.getServiceRecordMap();
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
        ImsServiceManager.setDefault(null);
        if (mServiceManager != null) {
            mServiceManager.dispose();
            mServiceManager = null;
        }
        mCallAppMap = null;
        mServiceRecordMap = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    public void testConstructor() {
        assertEquals(1, mServiceRecordMap.size());
        assertNotNull(mServiceManager.getImsServiceListener());
    }

    @Test
    public void testGetSetDefault() {
        assertNull(ImsServiceManager.getDefault());
        ImsServiceManager.setDefault(mServiceManager);
        assertEquals(mServiceManager, ImsServiceManager.getDefault());
    }

    @Test
    public void testGetServiceRecord() {
        assertNull(ImsServiceManager.getServiceRecord(MSimUtils.DEFAULT_PHONE_ID));
        ImsServiceManager.setDefault(mServiceManager);
        assertEquals(1, mServiceRecordMap.size());
        assertEquals(mServiceRecordMap.get(MSimUtils.DEFAULT_PHONE_ID),
                ImsServiceManager.getServiceRecord(MSimUtils.DEFAULT_PHONE_ID));
    }

    @Test
    public void testDispose() {
        mServiceManager.dispose();
        verify(mMockImsCallApp, never()).close();
        assertTrue(mCallAppMap.isEmpty());

        mCallAppMap.put(MSimUtils.DEFAULT_PHONE_ID, mMockImsCallApp);
        assertEquals(1, mCallAppMap.size());

        mServiceRecordMap.clear();
        assertTrue(mServiceRecordMap.isEmpty());
        mServiceRecordMap.put(MSimUtils.DEFAULT_PHONE_ID, mMockServiceRecord);
        when(mMockServiceRecord.isServiceUp()).thenReturn(true);
        mServiceManager.dispose();
        mServiceManager = null;
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
        when(mockImsContext.getPhoneId()).thenReturn(MSimUtils.DEFAULT_PHONE_ID);

        ImsCallApp callApp = mServiceManager.createCallApp(mockImsContext, mockFeatureCapaListener,
                mockCallListener);
        assertNull(callApp);

        mServiceRecordMap.put(MSimUtils.DEFAULT_PHONE_ID, mMockServiceRecord);
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
        mServiceManager.destroyCallApp(MSimUtils.DEFAULT_PHONE_ID);
        verify(mMockImsCallApp, never()).close();

        mCallAppMap.put(MSimUtils.DEFAULT_PHONE_ID, mMockImsCallApp);
        mServiceRecordMap.put(MSimUtils.DEFAULT_PHONE_ID, mMockServiceRecord);
        mServiceManager.destroyCallApp(MSimUtils.DEFAULT_PHONE_ID);
        verify(mMockServiceRecord).setCallApp(null);
        verify(mMockImsCallApp).close();
        assertTrue(mCallAppMap.isEmpty());
    }

    @Test
    public void testGetCallApp() {
        mCallAppMap.clear();
        ImsCallApp callApp = mServiceManager.getCallApp(MSimUtils.DEFAULT_PHONE_ID);
        assertNull(callApp);

        mCallAppMap.put(MSimUtils.DEFAULT_PHONE_ID, mMockImsCallApp);
        callApp = mServiceManager.getCallApp(MSimUtils.DEFAULT_PHONE_ID);
        assertEquals(mMockImsCallApp, callApp);
    }

    @Test
    public void testGetCallAppByPhoneId() {
        mServiceManager.setMultiImsEnabled(false);
        ImsCallApp callApp = mServiceManager.getCallAppByPhoneId(MSimUtils.INVALID_PHONE_ID);
        assertNull(callApp);

        mServiceRecordMap.clear();
        callApp = mServiceManager.getCallAppByPhoneId(MSimUtils.DEFAULT_PHONE_ID);
        assertNull(callApp);

        mServiceRecordMap.put(MSimUtils.DEFAULT_PHONE_ID, mMockServiceRecord);
        when(mMockServiceRecord.getCallApp()).thenReturn(mMockImsCallApp);
        callApp = mServiceManager.getCallAppByPhoneId(MSimUtils.DEFAULT_PHONE_ID);
        verify(mMockServiceRecord).getCallApp();
        assertEquals(mMockImsCallApp, callApp);
    }

    @Test
    public void testGetCallAppCount() {
        assertEquals(0, mServiceManager.getCallAppCount());
        mCallAppMap.put(MSimUtils.DEFAULT_PHONE_ID, mMockImsCallApp);
        assertEquals(1, mServiceManager.getCallAppCount());
    }

    @Test
    public void testGetContext() {
        assertEquals(mContext, mServiceManager.getContext());
    }

    @Test
    public void testGetDefaultPhoneId() {
        assertEquals(MSimUtils.DEFAULT_PHONE_ID, mServiceManager.getDefaultPhoneId());
    }

    @Test
    public void testgetDefaultServiceRecord() {
        mServiceRecordMap.clear();
        mServiceRecordMap.put(MSimUtils.DEFAULT_PHONE_ID, mMockServiceRecord);
        assertEquals(mMockServiceRecord, mServiceManager.getDefaultServiceRecord());
    }

    @Test
    public void testIsValidPhoneId() {
        assertTrue(mServiceManager.isValidPhoneId(MSimUtils.DEFAULT_PHONE_ID));
        assertFalse(mServiceManager.isValidPhoneId(MSimUtils.INVALID_PHONE_ID));

        mServiceManager.setMultiImsEnabled(false);
        assertFalse(mServiceManager.isValidPhoneId(MSimUtils.INVALID_PHONE_ID));
        assertTrue(mServiceManager.isValidPhoneId(MSimUtils.DEFAULT_PHONE_ID));
    }

    private class TestImsServiceManager extends ImsServiceManager {
        int mSimPhoneId = 0;
        boolean mIsMultiImsEnabled = true;

        TestImsServiceManager(Context context, MessageExecutor executor) {
            super(context, executor);
        }

        public ImsStackRegistry.ImsServiceListener getImsServiceListener() {
            return mImsServiceListener;
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
