/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.imsservice.mmtel.config.base;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.atLeastOnce;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.PersistableBundle;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.stub.ImsConfigImplBase;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Queue;
import java.util.Set;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;

@RunWith(JUnit4.class)
public class ConfigProxyTest {
    private TestAppContext mTestAppContext;
    private ConfigProxy mConfigProxy;
    private Queue<Integer> mDataQueue = new ConcurrentLinkedQueue<>();

    @Mock CarrierConfig mCarrierConfig;
    @Mock ConfigurationListener mConfigListener;
    @Mock ConfigInterface mConfigInterface;
    @Mock Context mContext;
    @Mock ExecutorService mExecutorService;
    @Mock IContext mContextInterface;
    @Mock ImsConfigImplBase mImsConfigImplBase;
    @Mock SharedPreferences mSp;
    @Mock SimInterface mSimInterface;
    @Mock SharedPreferences.Editor mEditor;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();

        AgentFactory.getInstance()
                .setAgent(ConfigInterface.class, mConfigInterface, TestAppContext.SLOT0);
        AgentFactory.getInstance()
                .setAgent(SimInterface.class, mSimInterface, TestAppContext.SLOT0);

        when(mSimInterface.getSubId()).thenReturn(TestAppContext.SUB_ID_1);
        when(mConfigInterface.getCarrierConfig()).thenReturn(mCarrierConfig);
        when(mContextInterface.getContext()).thenReturn(mContext);
        when(mContext.getSharedPreferences(anyString(), anyInt())).thenReturn(mSp);
        when(mSp.edit()).thenReturn(mEditor);

        mConfigProxy = new ConfigProxy(mContextInterface, mImsConfigImplBase);
    }

    @After
    public void tearDown() {
        mDataQueue = null;
        mConfigProxy = null;

        AgentFactory.getInstance()
                .setAgent(SimInterface.class, null, TestAppContext.SLOT0);
        AgentFactory.getInstance()
                .setAgent(ConfigInterface.class, null, TestAppContext.SLOT0);

        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void init_loadDefaultValueForNewSubscriber() {
        when(mSp.getAll()).thenReturn(new HashMap<>());
        mConfigProxy.init();

        verify(mConfigInterface, atLeastOnce()).getCarrierConfig();
        verify(mEditor, atLeastOnce()).putInt(anyString(), anyInt());
        verify(mEditor, atLeastOnce()).putString(anyString(), anyString());
        mConfigProxy.clear();
    }

    @Test
    @SmallTest
    public void getValueInt_nonIntegerItem_returnResultUnknown() {
        int key = ProvisioningManager.KEY_REGISTRATION_DOMAIN_NAME;

        int result = mConfigProxy.getValueInt(key);

        verifyNoMoreInteractions(mSp);
        assertEquals(ImsConfigImplBase.CONFIG_RESULT_UNKNOWN, result);
    }

    @Test
    @SmallTest
    public void getValueInt_failToGetSharedPreference_returnResultUnknown() {
        when(mContext.getSharedPreferences(anyString(), anyInt())).thenReturn(null);
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;

        int result = mConfigProxy.getValueInt(key);

        assertEquals(ImsConfigImplBase.CONFIG_RESULT_UNKNOWN, result);
    }

    @Test
    @SmallTest
    public void getValueInt_returnStoredIntegerValue() {
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;

        mConfigProxy.getValueInt(key);

        ProvisioningItem item = ConfigUtils.getProvisioningItem(key);
        verify(mSp).getInt(item.getName(), ImsConfigImplBase.CONFIG_RESULT_UNKNOWN);
    }

    @Test
    @SmallTest
    public void getValueString_nonStringItem_returnNull() {
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;

        String result = mConfigProxy.getValueString(key);

        verifyNoMoreInteractions(mSp);
        assertNull(result);
    }

    @Test
    @SmallTest
    public void getValueString_failToGetSharedPreference_returnNull() {
        when(mContext.getSharedPreferences(anyString(), anyInt())).thenReturn(null);
        int key = ProvisioningManager.KEY_REGISTRATION_DOMAIN_NAME;

        String result = mConfigProxy.getValueString(key);

        assertNull(result);
    }

    @Test
    @SmallTest
    public void getValueString_returnStoredStringValue() {
        int key = ProvisioningManager.KEY_REGISTRATION_DOMAIN_NAME;

        mConfigProxy.getValueString(key);

        ProvisioningItem item = ConfigUtils.getProvisioningItem(key);
        verify(mSp).getString(item.getName(), null);
    }

    @Test
    @SmallTest
    public void updateCarrierConfigData_storeImsCarrierConfigs() {
        PersistableBundle imsCarrierConfigs = new PersistableBundle();
        imsCarrierConfigs.putInt("Key1", 4);
        imsCarrierConfigs.putInt("Key2", 5);

        mConfigProxy.updateCarrierConfigData(imsCarrierConfigs);

        PersistableBundle updatedConfigs = mConfigProxy.getCarrierConfigData();
        assertTrue(isSameConfig(imsCarrierConfigs, updatedConfigs));
    }

    @Test
    @SmallTest
    public void setValueInt_nonIntegerItem_returnFalse() {
        int key = ProvisioningManager.KEY_REGISTRATION_DOMAIN_NAME;
        int value = 1;

        boolean result = mConfigProxy.setValueInt(key, value);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void setValueInt_invalidValue_returnFalse() {
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;
        int value = 100;

        boolean result = mConfigProxy.setValueInt(key, value);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void setValueInt_failToGetSharedPreference_returnFalse() {
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;
        int value = 1;
        when(mContext.getSharedPreferences(anyString(), anyInt())).thenReturn(null);

        boolean result = mConfigProxy.setValueInt(key, value);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void setValueInt_valueIsNotChanged_returnTrueWithoutUpdate() {
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;
        ProvisioningItem item = ConfigUtils.getProvisioningItem(key);
        int value = 1;
        when(mSp.getInt(item.getName(), ImsConfigImplBase.CONFIG_RESULT_UNKNOWN)).thenReturn(value);

        boolean result = mConfigProxy.setValueInt(key, value);

        assertTrue(result);
        verifyNoMoreInteractions(mEditor);
        verifyNoMoreInteractions(mExecutorService);
    }

    @Test
    @SmallTest
    public void setValueInt_storeIntegerValue() throws Exception {
        replaceInstance("mRequestExecutor", mExecutorService);
        replaceInstance("mData", mDataQueue);
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;
        ProvisioningItem item = ConfigUtils.getProvisioningItem(key);
        int value = 1;
        when(mSp.getInt(item.getName(), ImsConfigImplBase.CONFIG_RESULT_UNKNOWN))
                .thenReturn(ImsConfigImplBase.CONFIG_RESULT_UNKNOWN);

        mConfigProxy.setValueInt(key, value);

        verify(mEditor, atLeastOnce()).putInt(item.getName(), value);
        verify(mExecutorService).execute(any(Runnable.class));
    }

    @Test
    @SmallTest
    public void setValueString_nonStringItem_returnFalse() {
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;
        String value = "test";

        boolean result = mConfigProxy.setValueString(key, value);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void setValueString_failToGetSharedPreference_returnFalse() {
        int key = ProvisioningManager.KEY_REGISTRATION_DOMAIN_NAME;
        String value = "test";
        when(mContext.getSharedPreferences(anyString(), anyInt())).thenReturn(null);

        boolean result = mConfigProxy.setValueString(key, value);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void setValueString_valueIsNotChanged_returnTrueWithoutUpdate() {
        int key = ProvisioningManager.KEY_REGISTRATION_DOMAIN_NAME;
        ProvisioningItem item = ConfigUtils.getProvisioningItem(key);
        String value = "test";
        when(mSp.getString(item.getName(), null)).thenReturn(value);

        boolean result = mConfigProxy.setValueString(key, value);

        assertTrue(result);
        verifyNoMoreInteractions(mEditor);
        verifyNoMoreInteractions(mExecutorService);
    }

    @Test
    @SmallTest
    public void setValueString_storeStringValue() throws Exception {
        replaceInstance("mRequestExecutor", mExecutorService);
        replaceInstance("mData", mDataQueue);
        int key = ProvisioningManager.KEY_REGISTRATION_DOMAIN_NAME;
        ProvisioningItem item = ConfigUtils.getProvisioningItem(key);
        String value = "test";
        when(mSp.getString(item.getName(), null)).thenReturn(null);

        mConfigProxy.setValueString(key, value);

        verify(mEditor, atLeastOnce()).putString(item.getName(), value);
        verify(mExecutorService).execute(any(Runnable.class));
    }

    @Test
    @SmallTest
    public void notifyImsConfigChanged_notNotifiesFramework() {
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;
        int value = 1;
        mConfigProxy.notifyImsConfigChanged(mContextInterface, key, value);

        verify(mImsConfigImplBase).notifyProvisionedValueChanged(key, value);
    }

    @Test
    @SmallTest
    public void notifyConfigItemsChanged_notifiesAddedListener() throws Exception {
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;
        mDataQueue.offer(key);
        replaceInstance("mData", mDataQueue);
        mConfigProxy.addListener(mConfigListener);

        mConfigProxy.notifyConfigItemsChanged();

        verify(mConfigListener).onImsConfigurationChanged(key);
    }

    @Test
    @SmallTest
    public void notifyConfigItemsChanged_doesNotNotifyRemovedListener() throws Exception {
        int key = ProvisioningManager.KEY_ENABLE_SILENT_REDIAL;
        mDataQueue.offer(key);
        replaceInstance("mData", mDataQueue);
        mConfigProxy.addListener(mConfigListener);
        mConfigProxy.removeListener(mConfigListener);

        mConfigProxy.notifyConfigItemsChanged();

        verifyNoMoreInteractions(mConfigListener);
    }

    private boolean isSameConfig(PersistableBundle bundle1, PersistableBundle bundle2) {
        if (!bundle1.keySet().equals(bundle2.keySet())) {
            return false;
        }

        Set<String> keys = bundle1.keySet();
        for (String key : keys) {
            if (bundle1.getInt(key) != bundle2.getInt(key)) {
                return false;
            }
        }
        return true;
    }

    private synchronized void replaceInstance(
            final String instanceName, final Object newValue) throws Exception {
        Field field = ConfigProxy.class.getDeclaredField(instanceName);
        field.setAccessible(true);
        field.set(mConfigProxy, newValue);
    }
}
