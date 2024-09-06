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

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.intThat;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Looper;
import android.os.PersistableBundle;
import android.telephony.SubscriptionManager;
import android.util.ArrayMap;

import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ImsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ImsMmTelManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.IpSecManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.LocationManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ProvisioningManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.SensorManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.SmsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;

import java.lang.reflect.Field;
import java.util.Map;

/**
 * An application context for testing using {@link AppContext}, {@link SystemServiceProxy},
 * and so on.
 */
public class TestAppContext {
    /** Slot information. */
    public static final int SLOT0 = 0;
    public static final int SLOT1 = 1;
    /** Subscription information. */
    public static final int SUB_ID_1 = 1; // For SLOT0
    public static final int SUB_ID_2 = 2; // For SLOT1

    private final Context mContext;
    private final BroadcastReceiverProxy mBroadcastReceiverProxy =
            mock(BroadcastReceiverProxy.class);
    private final ContentProviderProxy mContentProviderProxy = mock(ContentProviderProxy.class);
    private final TelephonyManagerProxy mTelephonyManagerProxy = mock(TelephonyManagerProxy.class);
    private final CarrierConfigManagerProxy mCarrierConfigManagerProxy =
            mock(CarrierConfigManagerProxy.class);
    private final SubscriptionManagerProxy mSubscriptionManagerProxy =
            mock(SubscriptionManagerProxy.class);
    private final ConnectivityManagerProxy mConnectivityManagerProxy =
            mock(ConnectivityManagerProxy.class);
    private final IpSecManagerProxy mIpSecManagerProxy = mock(IpSecManagerProxy.class);
    private final LocationManagerProxy mLocationManagerProxy = mock(LocationManagerProxy.class);
    private final SensorManagerProxy mSensorManagerProxy = mock(SensorManagerProxy.class);
    private final SmsManagerProxy mSmsManagerProxy = mock(SmsManagerProxy.class);
    private final ImsManagerProxy mImsManagerProxy = mock(ImsManagerProxy.class);
    private final ImsMmTelManagerProxy mImsMmTelManagerProxy = mock(ImsMmTelManagerProxy.class);
    private final ProvisioningManagerProxy mProvisioningManagerProxy =
            mock(ProvisioningManagerProxy.class);
    private final SystemServiceProxy mSystemServiceProxy = spy(new FakeSystemServiceProxy());
    private AppContext mAppContext;

    public TestAppContext() {
        mContext = mock(Context.class);
    }

    public TestAppContext(Context context) {
        mContext = context;
    }

    public void setUp() {
        AppContext.init(mContext);
        AppContext.getInstance().setBroadcastReceiverProxy(mBroadcastReceiverProxy);
        AppContext.getInstance().setContentProviderProxy(mContentProviderProxy);
        AppContext.getInstance().setSystemServiceProxy(mSystemServiceProxy);
        DeviceConfig.init(AppContext.getInstance());
    }

    public void setUpWithLooper(Looper looper) throws Exception {
        mAppContext = new AppContext(mContext, looper);
        replaceSingletonAppContext(mAppContext);
        AppContext.getInstance().setBroadcastReceiverProxy(mBroadcastReceiverProxy);
        AppContext.getInstance().setContentProviderProxy(mContentProviderProxy);
        AppContext.getInstance().setSystemServiceProxy(mSystemServiceProxy);
        DeviceConfig.init(AppContext.getInstance());
    }

    public void tearDown() {
        if (mAppContext != null) {
            try {
                replaceSingletonAppContext(null);
            } catch (Exception ignored) {
                // Ignore an exception.
            }
            mAppContext = null;
        } else {
            AppContext.deinit();
        }
    }

    public Context getContext() {
        return mContext;
    }

    public BroadcastReceiverProxy getBroadcastReceiverProxy() {
        return mBroadcastReceiverProxy;
    }

    public ContentProviderProxy getContentProviderProxy() {
        return mContentProviderProxy;
    }

    public SystemServiceProxy getSystemServiceProxy() {
        return mSystemServiceProxy;
    }

    public <T> T getSystemServiceProxy(Class<T> clazz) {
        return mSystemServiceProxy.getSystemService(clazz);
    }

    private class FakeSystemServiceProxy implements SystemServiceProxy {
        private final PersistableBundle mCarrierConfig = new PersistableBundle();
        private final Map<Class<?>, Object> mManagerProxies = new ArrayMap<>();

        FakeSystemServiceProxy() {
            mManagerProxies.put(TelephonyManagerProxy.class, mTelephonyManagerProxy);
            mManagerProxies.put(CarrierConfigManagerProxy.class, mCarrierConfigManagerProxy);
            mManagerProxies.put(SubscriptionManagerProxy.class, mSubscriptionManagerProxy);
            mManagerProxies.put(ConnectivityManagerProxy.class, mConnectivityManagerProxy);
            mManagerProxies.put(IpSecManagerProxy.class, mIpSecManagerProxy);
            mManagerProxies.put(LocationManagerProxy.class, mLocationManagerProxy);
            mManagerProxies.put(SensorManagerProxy.class, mSensorManagerProxy);
            mManagerProxies.put(SmsManagerProxy.class, mSmsManagerProxy);
            mManagerProxies.put(ImsManagerProxy.class, mImsManagerProxy);

            when(mTelephonyManagerProxy.createForSubscriptionId(anyInt()))
                    .thenReturn(mTelephonyManagerProxy);
            when(mTelephonyManagerProxy.getActiveModemCount()).thenReturn(1);
            when(mTelephonyManagerProxy.getSupportedModemCount()).thenReturn(1);
            when(mSubscriptionManagerProxy.isUsableSubscriptionId(
                    intThat(subId -> SubscriptionManager.isUsableSubscriptionId(subId))))
                    .thenReturn(true);
            when(mSubscriptionManagerProxy.isValidSubscriptionId(
                    intThat(subId -> SubscriptionManager.isValidSubscriptionId(subId))))
                    .thenReturn(true);
            when(mSubscriptionManagerProxy.getSlotIndex(SUB_ID_1)).thenReturn(SLOT0);
            when(mSubscriptionManagerProxy.getSlotIndex(SUB_ID_2)).thenReturn(SLOT1);
            when(mSubscriptionManagerProxy.getSubscriptionId(SLOT0)).thenReturn(SUB_ID_1);
            when(mSubscriptionManagerProxy.getSubscriptionId(SLOT1)).thenReturn(SUB_ID_2);
            when(mSubscriptionManagerProxy.getDefaultDataSubscriptionId())
                    .thenReturn(MSimUtils.DEFAULT_SUB_ID);
            when(mCarrierConfigManagerProxy.getConfigForSubId(anyInt(), any()))
                    .thenReturn(mCarrierConfig);
            when(mSmsManagerProxy.createForSubscriptionId(anyInt())).thenReturn(mSmsManagerProxy);
            when(mImsManagerProxy.getImsMmTelManagerProxy(anyInt()))
                    .thenReturn(mImsMmTelManagerProxy);
            when(mImsManagerProxy.getProvisioningManagerProxy(anyInt()))
                    .thenReturn(mProvisioningManagerProxy);
        }

        @Override
        public <T> T getSystemService(Class<T> clazz) {
            Object proxy = mManagerProxies.get(clazz);
            if (proxy == null) {
                throw new IllegalArgumentException("Unknown system proxy: " + clazz);
            }
            return (T) proxy;
        }
    }

    public <T> T getSystemService(Class<T> clazz) {
        return mContext.getSystemService(clazz);
    }

    private void replaceSingletonAppContext(Object newValue) throws Exception {
        Field field = AppContext.class.getDeclaredField("sAppContext");
        field.setAccessible(true);
        field.set(null, newValue);
    }
}
