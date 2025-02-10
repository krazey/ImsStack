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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.hardware.TriggerEventListener;
import android.location.LastLocationRequest;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.IpSecManager;
import android.net.IpSecManager.ResourceUnavailableException;
import android.net.IpSecManager.SpiUnavailableException;
import android.net.IpSecTransform;
import android.net.Network;
import android.net.NetworkRequest;
import android.net.QosCallback;
import android.net.QosSocketInfo;
import android.net.Uri;
import android.os.Handler;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.CarrierConfigManager.CarrierConfigChangeListener;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.telephony.ims.ImsException;
import android.telephony.ims.ImsManager;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.ProvisioningManager.FeatureProvisioningCallback;

import androidx.test.filters.SmallTest;

import com.android.dx.mockito.inline.extended.ExtendedMockito;
import com.android.imsstack.ContextFixture;
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

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.MockitoAnnotations;
import org.mockito.MockitoSession;

import java.io.FileDescriptor;
import java.io.IOException;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class SystemServiceProxyImplTest {
    private static class UnknownProxy {
        UnknownProxy() {}
    }

    private ContextFixture mContextFixture;
    private Context mContext;
    private SystemServiceProxyImpl mSystemServiceProxy;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();

        mSystemServiceProxy = new SystemServiceProxyImpl(mContext);
    }

    @After
    public void tearDown() throws Exception {
        mSystemServiceProxy = null;
        mContext = null;
        mContextFixture = null;
    }

    @Test
    @SmallTest
    public void testGetSystemService() {
        TelephonyManagerProxy telephonyProxy =
                mSystemServiceProxy.getSystemService(TelephonyManagerProxy.class);
        assertNotNull(telephonyProxy);

        CarrierConfigManagerProxy carrierConfigProxy =
                mSystemServiceProxy.getSystemService(CarrierConfigManagerProxy.class);
        assertNotNull(carrierConfigProxy);

        SubscriptionManagerProxy subscriptionProxy =
                mSystemServiceProxy.getSystemService(SubscriptionManagerProxy.class);
        assertNotNull(subscriptionProxy);

        ConnectivityManagerProxy connectivityProxy =
                mSystemServiceProxy.getSystemService(ConnectivityManagerProxy.class);
        assertNotNull(connectivityProxy);

        IpSecManagerProxy ipSecProxy =
                mSystemServiceProxy.getSystemService(IpSecManagerProxy.class);
        assertNotNull(ipSecProxy);

        LocationManagerProxy locationProxy =
                mSystemServiceProxy.getSystemService(LocationManagerProxy.class);
        assertNotNull(locationProxy);

        SensorManagerProxy sensorProxy =
                mSystemServiceProxy.getSystemService(SensorManagerProxy.class);
        assertNotNull(sensorProxy);

        SmsManagerProxy smsProxy = mSystemServiceProxy.getSystemService(SmsManagerProxy.class);
        assertNotNull(smsProxy);

        ImsManagerProxy imsProxy = mSystemServiceProxy.getSystemService(ImsManagerProxy.class);
        assertNotNull(imsProxy);

        assertThrows(IllegalArgumentException.class, () -> {
            mSystemServiceProxy.getSystemService(UnknownProxy.class);
        });
    }

    @Test
    @SmallTest
    public void testCarrierConfigManagerProxy_accessors() {
        CarrierConfigManager carrierConfigManager =
                mContext.getSystemService(CarrierConfigManager.class);
        CarrierConfigManagerProxy ccmp =
                mSystemServiceProxy.getSystemService(CarrierConfigManagerProxy.class);

        PersistableBundle b = new PersistableBundle();
        assertFalse(ccmp.isConfigForIdentifiedCarrier(b));
        assertFalse(ccmp.isConfigForIdentifiedCarrier(null));

        b.putBoolean(CarrierConfigManager.KEY_CARRIER_CONFIG_APPLIED_BOOL, true);
        assertTrue(ccmp.isConfigForIdentifiedCarrier(b));

        assertNotNull(ccmp.getDefaultConfig());

        final String[] keys = {
            CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL,
            CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL,
            CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL
        };
        ccmp.getConfigForSubId(TestAppContext.SUB_ID_1, keys);
        verify(carrierConfigManager).getConfigForSubId(eq(TestAppContext.SUB_ID_1), any());

        // Expected that the CarrierConfigManager API throws an exception.
        doThrow(new IllegalStateException("getConfigForSubId exception."))
                .when(carrierConfigManager).getConfigForSubId(anyInt(), any());
        PersistableBundle config = ccmp.getConfigForSubId(TestAppContext.SUB_ID_1, keys);
        assertTrue(config.isEmpty());

        // Expected that the CarrierConfigManager is null.
        mContextFixture.setSystemService(Context.CARRIER_CONFIG_SERVICE, null);

        config = ccmp.getConfigForSubId(TestAppContext.SUB_ID_1, keys);
        assertNotNull(config);
        assertTrue(config.isEmpty());
    }

    @Test
    @SmallTest
    public void testCarrierConfigManagerProxy_registerAndUnregisterCarrierConfigChangeListener() {
        CarrierConfigManager carrierConfigManager =
                mContext.getSystemService(CarrierConfigManager.class);
        CarrierConfigManagerProxy ccmp =
                mSystemServiceProxy.getSystemService(CarrierConfigManagerProxy.class);
        Executor executor = mock(Executor.class);
        CarrierConfigChangeListener listener = mock(CarrierConfigChangeListener.class);

        ccmp.registerCarrierConfigChangeListener(executor, listener);
        verify(carrierConfigManager)
                .registerCarrierConfigChangeListener(eq(executor), eq(listener));

        ccmp.unregisterCarrierConfigChangeListener(listener);
        verify(carrierConfigManager).unregisterCarrierConfigChangeListener(eq(listener));

        // Expected that the CarrierConfigManager is null.
        mContextFixture.setSystemService(Context.CARRIER_CONFIG_SERVICE, null);
        ccmp.registerCarrierConfigChangeListener(executor, listener);
        ccmp.unregisterCarrierConfigChangeListener(listener);
        verifyNoMoreInteractions(carrierConfigManager);
    }

    @Test
    @SmallTest
    public void testSubscriptionManagerProxy_accessors() {
        SubscriptionManager subscriptionManager =
                mContext.getSystemService(SubscriptionManager.class);
        SubscriptionManagerProxy smp =
                mSystemServiceProxy.getSystemService(SubscriptionManagerProxy.class);

        assertTrue(smp.isUsableSubscriptionId(TestAppContext.SUB_ID_1));
        assertFalse(smp.isUsableSubscriptionId(MSimUtils.DEFAULT_SUB_ID));
        assertFalse(smp.isUsableSubscriptionId(MSimUtils.INVALID_SUB_ID));

        assertTrue(smp.isValidSubscriptionId(TestAppContext.SUB_ID_1));
        assertTrue(smp.isValidSubscriptionId(MSimUtils.DEFAULT_SUB_ID));
        assertFalse(smp.isValidSubscriptionId(MSimUtils.INVALID_SUB_ID));

        smp.getPhoneNumber(TestAppContext.SUB_ID_1, SubscriptionManager.PHONE_NUMBER_SOURCE_UICC);
        verify(subscriptionManager).getPhoneNumber(eq(TestAppContext.SUB_ID_1),
                eq(SubscriptionManager.PHONE_NUMBER_SOURCE_UICC));

        // Expected that the SubscriptionManager is null.
        mContextFixture.setSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE, null);
        assertThrows(IllegalStateException.class, () -> {
            smp.getPhoneNumber(TestAppContext.SUB_ID_1,
                    SubscriptionManager.PHONE_NUMBER_SOURCE_UICC);
        });

        // Static methods
        MockitoSession mockitoSession =
                ExtendedMockito.mockitoSession()
                        .mockStatic(SubscriptionManager.class)
                        .startMocking();

        try {
            smp.getDefaultDataSubscriptionId();
            ExtendedMockito.verify(() -> SubscriptionManager.getDefaultDataSubscriptionId());

            smp.getSlotIndex(TestAppContext.SUB_ID_1);
            ExtendedMockito.verify(
                    () -> SubscriptionManager.getSlotIndex(eq(TestAppContext.SUB_ID_1)));

            smp.getSubscriptionId(TestAppContext.SLOT0);
            ExtendedMockito.verify(
                    () -> SubscriptionManager.getSubscriptionId(eq(TestAppContext.SLOT0)));
        } finally {
            mockitoSession.finishMocking();
        }
    }

    @Test
    @SmallTest
    public void testConnectivityManagerProxy_registerAndUnregisterQosCallback()
            throws SocketException, IOException {
        ConnectivityManager connectivityManager =
                mContext.getSystemService(ConnectivityManager.class);
        ConnectivityManagerProxy cmp =
                mSystemServiceProxy.getSystemService(ConnectivityManagerProxy.class);
        Executor executor = mock(Executor.class);
        QosCallback callback = mock(QosCallback.class);
        QosSocketInfo socketInfo = mock(QosSocketInfo.class);

        cmp.registerQosCallback(socketInfo, executor, callback);
        verify(connectivityManager).registerQosCallback(eq(socketInfo), eq(executor), eq(callback));

        cmp.unregisterQosCallback(callback);
        verify(connectivityManager).unregisterQosCallback(eq(callback));

        // Expected that the ConnectivityManager is null.
        mContextFixture.setSystemService(Context.CONNECTIVITY_SERVICE, null);
        assertThrows(IllegalStateException.class, () -> {
            cmp.registerQosCallback(socketInfo, executor, callback);
        });

        cmp.unregisterQosCallback(callback);
        verifyNoMoreInteractions(connectivityManager);
    }

    @Test
    @SmallTest
    public void testConnectivityManagerProxy_registerAndUnregisterNetworkCallback() {
        ConnectivityManager connectivityManager =
                mContext.getSystemService(ConnectivityManager.class);
        ConnectivityManagerProxy cmp =
                mSystemServiceProxy.getSystemService(ConnectivityManagerProxy.class);
        NetworkRequest request = mock(NetworkRequest.class);
        NetworkCallback callback = mock(NetworkCallback.class);
        Handler handler = mock(Handler.class);

        cmp.registerNetworkCallback(request, callback, handler);
        verify(connectivityManager).registerNetworkCallback(eq(request), eq(callback), eq(handler));

        cmp.unregisterNetworkCallback(callback);
        verify(connectivityManager).unregisterNetworkCallback(eq(callback));

        // Expected that the ConnectivityManager is null.
        mContextFixture.setSystemService(Context.CONNECTIVITY_SERVICE, null);
        assertThrows(IllegalStateException.class, () -> {
            cmp.registerNetworkCallback(request, callback, handler);
        });

        cmp.unregisterNetworkCallback(callback);
        verifyNoMoreInteractions(connectivityManager);
    }

    @Test
    @SmallTest
    public void testConnectivityManagerProxy_getLinkProperties() {
        ConnectivityManager connectivityManager =
                mContext.getSystemService(ConnectivityManager.class);
        ConnectivityManagerProxy cmp =
                mSystemServiceProxy.getSystemService(ConnectivityManagerProxy.class);
        Network network = mock(Network.class);

        cmp.getLinkProperties(network);
        verify(connectivityManager).getLinkProperties(eq(network));

        // Expected that the ConnectivityManager is null.
        mContextFixture.setSystemService(Context.CONNECTIVITY_SERVICE, null);
        assertNull(cmp.getLinkProperties(network));
    }

    @Test
    @SmallTest
    public void testConnectivityManagerProxy_requestNetwork() {
        ConnectivityManager connectivityManager =
                mContext.getSystemService(ConnectivityManager.class);
        ConnectivityManagerProxy cmp =
                mSystemServiceProxy.getSystemService(ConnectivityManagerProxy.class);
        NetworkRequest request = mock(NetworkRequest.class);
        NetworkCallback callback = mock(NetworkCallback.class);
        Handler handler = mock(Handler.class);

        cmp.requestNetwork(request, callback, handler);
        verify(connectivityManager).requestNetwork(eq(request), eq(callback), eq(handler));

        // Expected that the ConnectivityManager is null.
        mContextFixture.setSystemService(Context.CONNECTIVITY_SERVICE, null);
        assertThrows(IllegalStateException.class, () -> {
            cmp.requestNetwork(request, callback, handler);
        });
    }

    @Test
    @SmallTest
    public void testConnectivityManagerProxy_registerSystemDefaultNetworkCallback() {
        ConnectivityManager connectivityManager =
                mContext.getSystemService(ConnectivityManager.class);
        ConnectivityManagerProxy cmp =
                mSystemServiceProxy.getSystemService(ConnectivityManagerProxy.class);
        NetworkCallback callback = mock(NetworkCallback.class);
        Handler handler = mock(Handler.class);

        cmp.registerSystemDefaultNetworkCallback(callback, handler);
        verify(connectivityManager).registerSystemDefaultNetworkCallback(eq(callback), eq(handler));

        // Expected that the ConnectivityManager is null.
        mContextFixture.setSystemService(Context.CONNECTIVITY_SERVICE, null);
        assertThrows(
                IllegalStateException.class,
                () -> {
                    cmp.registerSystemDefaultNetworkCallback(callback, handler);
                });
    }

    @Test
    @SmallTest
    public void testIpSecManagerProxy_allocateSecurityParameterIndex()
            throws SpiUnavailableException, ResourceUnavailableException {
        IpSecManager ipSecManager = mContext.getSystemService(IpSecManager.class);
        IpSecManagerProxy ismp = mSystemServiceProxy.getSystemService(IpSecManagerProxy.class);
        InetAddress dstAddr = mock(InetAddress.class);
        int spi = 1234;

        ismp.allocateSecurityParameterIndex(dstAddr, spi);
        verify(ipSecManager).allocateSecurityParameterIndex(eq(dstAddr), eq(spi));

        // Expected that the IpSecManager is null.
        mContextFixture.setSystemService(Context.IPSEC_SERVICE, null);
        assertThrows(IllegalStateException.class, () -> {
            ismp.allocateSecurityParameterIndex(dstAddr, spi);
        });
    }

    @Test
    @SmallTest
    public void testIpSecManagerProxy_applyTransportModeTransform() throws IOException {
        IpSecManager ipSecManager = mContext.getSystemService(IpSecManager.class);
        IpSecManagerProxy ismp = mSystemServiceProxy.getSystemService(IpSecManagerProxy.class);
        FileDescriptor socket = mock(FileDescriptor.class);
        IpSecTransform transform = mock(IpSecTransform.class);

        ismp.applyTransportModeTransform(socket, IpSecManager.DIRECTION_OUT, transform);
        verify(ipSecManager).applyTransportModeTransform(
                eq(socket), eq(IpSecManager.DIRECTION_OUT), eq(transform));

        // Expected that the IpSecManager is null.
        mContextFixture.setSystemService(Context.IPSEC_SERVICE, null);
        assertThrows(IllegalStateException.class, () -> {
            ismp.applyTransportModeTransform(socket, IpSecManager.DIRECTION_OUT, transform);
        });
    }

    @Test
    @SmallTest
    public void testIpSecManagerProxy_removeTransportModeTransforms() throws IOException {
        IpSecManager ipSecManager = mContext.getSystemService(IpSecManager.class);
        IpSecManagerProxy ismp = mSystemServiceProxy.getSystemService(IpSecManagerProxy.class);
        FileDescriptor socket = mock(FileDescriptor.class);

        ismp.removeTransportModeTransforms(socket);
        verify(ipSecManager).removeTransportModeTransforms(eq(socket));

        // Expected that the IpSecManager is null.
        mContextFixture.setSystemService(Context.IPSEC_SERVICE, null);
        ismp.removeTransportModeTransforms(socket);
        verifyNoMoreInteractions(ipSecManager);
    }

    @Test
    @SmallTest
    public void testLocationManagerProxy_isProviderEnabled() {
        LocationManager locationManager = mContext.getSystemService(LocationManager.class);
        LocationManagerProxy lmp = mSystemServiceProxy.getSystemService(LocationManagerProxy.class);

        lmp.isProviderEnabled(LocationManager.FUSED_PROVIDER);
        verify(locationManager).isProviderEnabled(eq(LocationManager.FUSED_PROVIDER));

        // Expected that the LocationManager is null.
        mContextFixture.setSystemService(Context.LOCATION_SERVICE, null);
        assertFalse(lmp.isProviderEnabled(LocationManager.FUSED_PROVIDER));
    }

    @Test
    @SmallTest
    public void testLocationManagerProxy_getLastKnownLocation() {
        LocationManager locationManager = mContext.getSystemService(LocationManager.class);
        LocationManagerProxy lmp = mSystemServiceProxy.getSystemService(LocationManagerProxy.class);
        LastLocationRequest request = mock(LastLocationRequest.class);

        lmp.getLastKnownLocation(LocationManager.FUSED_PROVIDER, request);
        verify(locationManager).getLastKnownLocation(
                eq(LocationManager.FUSED_PROVIDER), eq(request));

        // Expected that the LocationManager is null.
        mContextFixture.setSystemService(Context.LOCATION_SERVICE, null);
        assertNull(lmp.getLastKnownLocation(LocationManager.FUSED_PROVIDER, request));
    }

    @Test
    @SmallTest
    public void testLocationManagerProxy_requestLocationUpdates() {
        LocationManager locationManager = mContext.getSystemService(LocationManager.class);
        LocationManagerProxy lmp = mSystemServiceProxy.getSystemService(LocationManagerProxy.class);
        Executor executor = mock(Executor.class);
        LocationRequest request = mock(LocationRequest.class);
        LocationListener listener = mock(LocationListener.class);

        lmp.requestLocationUpdates(LocationManager.FUSED_PROVIDER, request, executor, listener);
        verify(locationManager).requestLocationUpdates(
                eq(LocationManager.FUSED_PROVIDER), eq(request), eq(executor), eq(listener));

        // Expected that the LocationManager is null.
        mContextFixture.setSystemService(Context.LOCATION_SERVICE, null);
        lmp.requestLocationUpdates(LocationManager.FUSED_PROVIDER, request, executor, listener);
        verifyNoMoreInteractions(locationManager);
    }

    @Test
    @SmallTest
    public void testLocationManagerProxy_removeUpdates() {
        LocationManager locationManager = mContext.getSystemService(LocationManager.class);
        LocationManagerProxy lmp = mSystemServiceProxy.getSystemService(LocationManagerProxy.class);
        LocationListener listener = mock(LocationListener.class);

        lmp.removeUpdates(listener);
        verify(locationManager).removeUpdates(eq(listener));

        // Expected that the LocationManager is null.
        mContextFixture.setSystemService(Context.LOCATION_SERVICE, null);
        lmp.removeUpdates(listener);
        verifyNoMoreInteractions(locationManager);
    }

    @Test
    @SmallTest
    public void testSensorManagerProxy_getDefaultSensor() {
        SensorManager sensorManager = mContext.getSystemService(SensorManager.class);
        SensorManagerProxy smp = mSystemServiceProxy.getSystemService(SensorManagerProxy.class);

        smp.getDefaultSensor(Sensor.TYPE_SIGNIFICANT_MOTION);
        verify(sensorManager).getDefaultSensor(eq(Sensor.TYPE_SIGNIFICANT_MOTION));

        // Expected that the SensorManager is null.
        mContextFixture.setSystemService(Context.SENSOR_SERVICE, null);
        assertNull(smp.getDefaultSensor(Sensor.TYPE_SIGNIFICANT_MOTION));
    }

    @Test
    @SmallTest
    public void testSensorManagerProxy_requestTriggerSensor() {
        SensorManager sensorManager = mContext.getSystemService(SensorManager.class);
        SensorManagerProxy smp = mSystemServiceProxy.getSystemService(SensorManagerProxy.class);
        TriggerEventListener listener = mock(TriggerEventListener.class);
        Sensor sensor = mock(Sensor.class);

        smp.requestTriggerSensor(listener, sensor);
        verify(sensorManager).requestTriggerSensor(eq(listener), eq(sensor));

        // Expected that the SensorManager is null.
        mContextFixture.setSystemService(Context.SENSOR_SERVICE, null);
        assertFalse(smp.requestTriggerSensor(listener, sensor));
    }

    @Test
    @SmallTest
    public void testSensorManagerProxy_cancelTriggerSensor() {
        SensorManager sensorManager = mContext.getSystemService(SensorManager.class);
        SensorManagerProxy smp = mSystemServiceProxy.getSystemService(SensorManagerProxy.class);
        TriggerEventListener listener = mock(TriggerEventListener.class);
        Sensor sensor = mock(Sensor.class);

        smp.cancelTriggerSensor(listener, sensor);
        verify(sensorManager).cancelTriggerSensor(eq(listener), eq(sensor));

        // Expected that the SensorManager is null.
        mContextFixture.setSystemService(Context.SENSOR_SERVICE, null);
        assertFalse(smp.cancelTriggerSensor(listener, sensor));
    }

    @Test
    @SmallTest
    public void testSmsManagerProxy_createForSubscriptionId() {
        SmsManager smsManager = mContext.getSystemService(SmsManager.class);
        SmsManagerProxy smp = mSystemServiceProxy.getSystemService(SmsManagerProxy.class);

        assertNotNull(smp.createForSubscriptionId(TestAppContext.SUB_ID_1));
        verify(smsManager).createForSubscriptionId(eq(TestAppContext.SUB_ID_1));

        // Expected that the SmsManager is null.
        mContextFixture.setSystemService(Context.SMS_SERVICE, null);
        assertThrows(IllegalStateException.class, () -> {
            smp.createForSubscriptionId(TestAppContext.SUB_ID_1);
        });
    }

    @Test
    @SmallTest
    public void testSmsManagerProxy_getSmscAddress() {
        SmsManager smsManager = mContext.getSystemService(SmsManager.class);
        SmsManagerProxy smp = mSystemServiceProxy.getSystemService(SmsManagerProxy.class);

        smp.getSmscAddress();
        verify(smsManager).getSmscAddress();

        // Expected that the SmsManager is null.
        mContextFixture.setSystemService(Context.SMS_SERVICE, null);
        mSystemServiceProxy = new SystemServiceProxyImpl(mContext);
        smp = mSystemServiceProxy.getSystemService(SmsManagerProxy.class);
        assertNull(smp.getSmscAddress());
    }

    @Test
    @SmallTest
    public void testSmsManagerProxy_getSmscIdentity() {
        SmsManager smsManager = mContext.getSystemService(SmsManager.class);
        SmsManagerProxy smp = mSystemServiceProxy.getSystemService(SmsManagerProxy.class);

        smp.getSmscIdentity();
        verify(smsManager).getSmscIdentity();

        // Expected that the SmsManager is null.
        mContextFixture.setSystemService(Context.SMS_SERVICE, null);
        mSystemServiceProxy = new SystemServiceProxyImpl(mContext);
        smp = mSystemServiceProxy.getSystemService(SmsManagerProxy.class);
        assertEquals(Uri.EMPTY, smp.getSmscIdentity());
    }

    @Test
    @SmallTest
    public void testImsManagerProxy_getImsMmTelManagerProxy() {
        ImsManager imsManager = mContext.getSystemService(ImsManager.class);
        ImsManagerProxy imp = mSystemServiceProxy.getSystemService(ImsManagerProxy.class);

        assertNotNull(imp.getImsMmTelManagerProxy(TestAppContext.SUB_ID_1));
        verify(imsManager).getImsMmTelManager(eq(TestAppContext.SUB_ID_1));

        doThrow(new IllegalArgumentException("Invalid subscription id."))
                .when(imsManager).getImsMmTelManager(eq(MSimUtils.INVALID_SUB_ID));
        assertNull(imp.getImsMmTelManagerProxy(MSimUtils.INVALID_SUB_ID));

        // Expected that the ImsManager is null.
        mContextFixture.setSystemService(Context.TELEPHONY_IMS_SERVICE, null);
        assertNull(imp.getImsMmTelManagerProxy(TestAppContext.SUB_ID_1));
    }

    @Test
    @SmallTest
    public void testImsManagerProxy_getProvisioningManagerProxy() {
        ImsManager imsManager = mContext.getSystemService(ImsManager.class);
        ImsManagerProxy imp = mSystemServiceProxy.getSystemService(ImsManagerProxy.class);

        assertNotNull(imp.getProvisioningManagerProxy(TestAppContext.SUB_ID_1));
        verify(imsManager).getProvisioningManager(eq(TestAppContext.SUB_ID_1));

        doThrow(new IllegalArgumentException("Invalid subscription id."))
                .when(imsManager).getProvisioningManager(eq(MSimUtils.INVALID_SUB_ID));
        assertNull(imp.getProvisioningManagerProxy(MSimUtils.INVALID_SUB_ID));

        // Expected that the ImsManager is null.
        mContextFixture.setSystemService(Context.TELEPHONY_IMS_SERVICE, null);
        assertNull(imp.getProvisioningManagerProxy(TestAppContext.SUB_ID_1));
    }

    @Test
    @SmallTest
    public void testImsMmTelManagerProxy_accessors() throws ImsException {
        ImsManager imsManager = mContext.getSystemService(ImsManager.class);
        ImsMmTelManager imsMmTelManager = mock(ImsMmTelManager.class);
        when(imsManager.getImsMmTelManager(anyInt())).thenReturn(imsMmTelManager);

        ImsManagerProxy imp = mSystemServiceProxy.getSystemService(ImsManagerProxy.class);
        ImsMmTelManagerProxy imtmp = imp.getImsMmTelManagerProxy(TestAppContext.SUB_ID_1);

        imtmp.isAdvancedCallingSettingEnabled();
        verify(imsMmTelManager).isAdvancedCallingSettingEnabled();

        imtmp.isVtSettingEnabled();
        verify(imsMmTelManager).isVtSettingEnabled();

        imtmp.isVoWiFiSettingEnabled();
        verify(imsMmTelManager).isVoWiFiSettingEnabled();

        imtmp.getVoWiFiModeSetting();
        verify(imsMmTelManager).getVoWiFiModeSetting();

        imtmp.isVoWiFiRoamingSettingEnabled();
        verify(imsMmTelManager).isVoWiFiRoamingSettingEnabled();

        imtmp.getVoWiFiRoamingModeSetting();
        verify(imsMmTelManager).getVoWiFiRoamingModeSetting();

        imtmp.isCrossSimCallingEnabled();
        verify(imsMmTelManager).isCrossSimCallingEnabled();

        doThrow(new ImsException("isCrossSimCallingEnabled failed."))
                .when(imsMmTelManager).isCrossSimCallingEnabled();
        assertFalse(imtmp.isCrossSimCallingEnabled());
    }

    @Test
    @SmallTest
    public void testImsMmTelManagerProxy_setCrossSimCallingEnabled() throws ImsException {
        ImsManager imsManager = mContext.getSystemService(ImsManager.class);
        ImsMmTelManager imsMmTelManager = mock(ImsMmTelManager.class);
        when(imsManager.getImsMmTelManager(anyInt())).thenReturn(imsMmTelManager);

        ImsManagerProxy imp = mSystemServiceProxy.getSystemService(ImsManagerProxy.class);
        ImsMmTelManagerProxy imtmp = imp.getImsMmTelManagerProxy(TestAppContext.SUB_ID_1);

        imtmp.setCrossSimCallingEnabled(true);
        verify(imsMmTelManager).setCrossSimCallingEnabled(eq(true));

        doThrow(new ImsException("setCrossSimCallingEnabled failed."))
                .when(imsMmTelManager).setCrossSimCallingEnabled(anyBoolean());
        // Expected: no exception thrown.
        imtmp.setCrossSimCallingEnabled(true);
    }

    @Test
    @SmallTest
    public void testProvisioningManagerProxy_registerFeatureProvisioningChangedCallback()
            throws ImsException {
        ImsManager imsManager = mContext.getSystemService(ImsManager.class);
        ProvisioningManager provisioningManager = mock(ProvisioningManager.class);
        when(imsManager.getProvisioningManager(anyInt())).thenReturn(provisioningManager);

        ImsManagerProxy imp = mSystemServiceProxy.getSystemService(ImsManagerProxy.class);
        ProvisioningManagerProxy pmp = imp.getProvisioningManagerProxy(TestAppContext.SUB_ID_1);

        Executor executor = mock(Executor.class);
        FeatureProvisioningCallback callback = mock(FeatureProvisioningCallback.class);

        pmp.registerFeatureProvisioningChangedCallback(executor, callback);
        verify(provisioningManager)
                .registerFeatureProvisioningChangedCallback(eq(executor), eq(callback));

        pmp.unregisterFeatureProvisioningChangedCallback(callback);
        verify(provisioningManager).unregisterFeatureProvisioningChangedCallback(eq(callback));
    }

    @Test
    @SmallTest
    public void testProvisioningManagerProxy_setProvisioningIntValue() {
        ImsManager imsManager = mContext.getSystemService(ImsManager.class);
        ProvisioningManager provisioningManager = mock(ProvisioningManager.class);
        when(imsManager.getProvisioningManager(anyInt())).thenReturn(provisioningManager);

        ImsManagerProxy imp = mSystemServiceProxy.getSystemService(ImsManagerProxy.class);
        ProvisioningManagerProxy pmp = imp.getProvisioningManagerProxy(TestAppContext.SUB_ID_1);

        final int key = 10;
        final int value = 1;
        pmp.setProvisioningIntValue(key, value);
        verify(provisioningManager).setProvisioningIntValue(eq(key), eq(value));
    }

    @Test
    @SmallTest
    public void testProvisioningManagerProxy_getProvisioningStatusForCapability() {
        ImsManager imsManager = mContext.getSystemService(ImsManager.class);
        ProvisioningManager provisioningManager = mock(ProvisioningManager.class);
        when(imsManager.getProvisioningManager(anyInt())).thenReturn(provisioningManager);

        ImsManagerProxy imp = mSystemServiceProxy.getSystemService(ImsManagerProxy.class);
        ProvisioningManagerProxy pmp = imp.getProvisioningManagerProxy(TestAppContext.SUB_ID_1);

        final int voiceCapability = 1 << 0;
        final int lteTech = 0;
        pmp.getProvisioningStatusForCapability(voiceCapability, lteTech);
        verify(provisioningManager)
                .getProvisioningStatusForCapability(eq(voiceCapability), eq(lteTech));
    }
}
