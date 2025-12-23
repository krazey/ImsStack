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
package com.android.imsstack.core.agents;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.location.Location;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.os.CancellationSignal;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.SystemServiceProxy;
import com.android.imsstack.base.SystemServiceProxy.LocationManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.Executor;
import java.util.function.Consumer;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class LocationAgentTest {
    @Mock private ConfigInterface mConfigInterface;
    @Mock private CarrierConfig mCarrierConfig;
    @Mock private SubsInfoInterface mSubsInfoInterface;
    @Mock private LocationInterface.Listener mLocationInfoListener;
    @Mock private Location mLocation;

    @Captor private ArgumentCaptor<Consumer<Location>> mLocationConsumerCaptor;
    @Captor private ArgumentCaptor<LocationRequest> mLocationRequestCaptor;

    private TestAppContext mTestAppContext;
    private LocationAgent mLocationAgent;

    private static final double TEST_LATITUDE = 37.7749;
    private static final double TEST_LONGITUDE = -122.4194;
    private static final float TEST_ACCURACY = 10.0f;
    private static final double TEST_ALTITUDE = 50.0;
    private static final long TEST_TIME = System.currentTimeMillis();
    private static final float TEST_VERTICAL_ACCURACY = 5.0f;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();

        AgentFactory.getInstance().setAgent(ConfigInterface.class, mConfigInterface, SLOT0);
        AgentFactory.getInstance().setAgent(SubsInfoInterface.class, mSubsInfoInterface, SLOT0);
        when(mConfigInterface.getCarrierConfig()).thenReturn(mCarrierConfig);

        mLocationAgent = new LocationAgent(SLOT0);
        mLocationAgent.init(mTestAppContext.getContext());
        mLocationAgent.addListener(mLocationInfoListener);

        when(mLocation.getLatitude()).thenReturn(TEST_LATITUDE);
        when(mLocation.getLongitude()).thenReturn(TEST_LONGITUDE);
        when(mLocation.getAccuracy()).thenReturn(TEST_ACCURACY);
        when(mLocation.getAltitude()).thenReturn(TEST_ALTITUDE);
        when(mLocation.getTime()).thenReturn(TEST_TIME);
        when(mLocation.getProvider()).thenReturn(LocationManager.FUSED_PROVIDER);
        when(mLocation.getVerticalAccuracyMeters()).thenReturn(TEST_VERTICAL_ACCURACY);
        when(mLocation.isMock()).thenReturn(false);
        when(mLocation.hasAltitude()).thenReturn(true);
    }

    @After
    public void tearDown() throws Exception {
        if (mLocationAgent != null) {
            mLocationAgent.removeListener(mLocationInfoListener);
            mLocationAgent.cleanup();
            mLocationAgent = null;
        }

        AgentFactory.getInstance().setAgent(SubsInfoInterface.class, null, SLOT0);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);

        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void testUpdateLocationPolicyWhenCarrierConfigChanged() {
        when(mSubsInfoInterface.isTestModeEnabled()).thenReturn(true);
        when(mCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_LOCATION_POLICY_UPDATE_TYPE_INT), anyInt()))
                .thenReturn(CarrierConfig.Ims.LOCATION_UPDATE_POLICY_ALWAYS);
        int configuredPolicy = LocationPolicy.POLICY_LOCATION_NOT_ALLOWED_PERIODIC_POLLING
                | LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;
        when(mCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_LOCATION_ACQUISITION_POLICY_INT), anyInt()))
                .thenReturn(configuredPolicy);
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfig.Ims.KEY_LOCATION_ALLOW_MOCK_LOCATION_UPDATE_BOOL)))
                .thenReturn(true);
        int addressResolutionTime = 1000;
        when(mCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_LOCATION_ADDRESS_RESOLUTION_TIME_MILLIS_INT), anyInt()))
                .thenReturn(addressResolutionTime);
        int validityMinutes = 1000;
        when(mCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_LOCATION_VALIDITY_PERIOD_MIN_INT), anyInt()))
                .thenReturn(validityMinutes);
        int locationValidityMinutes = 1000;
        when(mCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_LOCATION_ADDRESS_VALIDITY_PERIOD_MIN_INT), anyInt()))
                .thenReturn(locationValidityMinutes);
        int addressTolerableDistance = 150;
        when(mCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_LOCATION_TOLERABLE_DISTANCE_INT), anyInt()))
                .thenReturn(addressTolerableDistance);
        int gpsSearchingDuration = 20;
        when(mCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_LOCATION_GPS_SEARCHING_DURATION_SEC_INT), anyInt()))
                .thenReturn(gpsSearchingDuration);
        when(mCarrierConfig.getInt(eq(CarrierConfig.Ims.KEY_LOCATION_GEODETIC_SHAPE_INT)))
                .thenReturn(CarrierConfig.Ims.GEODETIC_SHAPE_ELLIPSOID);

        ArgumentCaptor<ConfigInterface.Listener> configListenerCaptor =
                ArgumentCaptor.forClass(ConfigInterface.Listener.class);
        verify(mConfigInterface).addListener(configListenerCaptor.capture());
        ConfigInterface.Listener listener = configListenerCaptor.getValue();
        listener.onCarrierConfigChanged(SLOT0, SUB_ID_1);

        LocationPolicy lp = mLocationAgent.getLocationPolicy();
        int expectedPolicy = configuredPolicy | LocationPolicy.POLICY_ENABLE_CACHED_LOCATION
                | LocationPolicy.POLICY_USE_CACHED_LOCATION
                | LocationPolicy.POLICY_ALLOW_MOCK_LOCATION_UPDATE;
        assertEquals(expectedPolicy, lp.getPolicy());
        assertEquals(addressResolutionTime, lp.getDefaultAddressResolutionTime());
        assertEquals(validityMinutes * 60L * 1000L * 1000000L, lp.getValidityPeriod());
        assertEquals(locationValidityMinutes * 60L * 1000L * 1000000L,
                lp.getAddressValidityPeriod());
        assertEquals(addressTolerableDistance, lp.getAddressTolerableDistance());
        assertEquals(gpsSearchingDuration, lp.getSearchDurationForGps());
        assertEquals(LocationPolicy.SHAPE_ELLIPSOID, lp.getShape());
    }

    @Test
    @SmallTest
    public void testNotifyListenersWhenLastKnownCountryChanged() {
        LocationPolicy lp = mLocationAgent.getLocationPolicy();
        lp.setPolicy(LocationPolicy.POLICY_NOTIFY_COUNTRY_CHANGED_EVENT);
        mLocationAgent.setLocationPolicy(lp);

        mLocationAgent.notifyEventOnLastKnownCountryChanged("KR", "US");

        verify(mLocationInfoListener).onLastKnownCountryUpdated();
    }

    @Test
    @SmallTest
    public void testRequestLocationUpdate() {
        LocationManagerProxy lmp = mTestAppContext.getSystemServiceProxy(
                SystemServiceProxy.LocationManagerProxy.class);
        when(lmp.isProviderEnabled(any(String.class))).thenReturn(true);

        int waitTimeMs = 2000; // 2s
        int requestId = mLocationAgent.requestLocationUpdate(waitTimeMs);

        ArgumentCaptor<CancellationSignal> cancellationSignalCaptor =
                ArgumentCaptor.forClass(CancellationSignal.class);
        verify(lmp).getCurrentLocation(any(String.class), any(LocationRequest.class),
                cancellationSignalCaptor.capture(), any(Executor.class), any(Consumer.class));
        assertFalse(cancellationSignalCaptor.getValue().isCanceled());
        assertNotEquals(0, requestId);

        mLocationAgent.cancelLocationUpdate(requestId);

        assertTrue(cancellationSignalCaptor.getValue().isCanceled());
    }

    @Test
    @SmallTest
    public void testRequestLocationUpdateWhenAllProvidersDisabled() {
        LocationManagerProxy lmp = mTestAppContext.getSystemServiceProxy(
                SystemServiceProxy.LocationManagerProxy.class);
        when(lmp.isProviderEnabled(any(String.class))).thenReturn(false);

        int waitTimeMs = 2000; // 2s
        int requestId = mLocationAgent.requestLocationUpdate(waitTimeMs);

        ArgumentCaptor<CancellationSignal> cancellationSignalCaptor =
                ArgumentCaptor.forClass(CancellationSignal.class);
        verify(lmp).getCurrentLocation(any(String.class), any(LocationRequest.class),
                cancellationSignalCaptor.capture(), any(Executor.class), any(Consumer.class));
        assertFalse(cancellationSignalCaptor.getValue().isCanceled());
        assertNotEquals(0, requestId);

        mLocationAgent.cancelLocationUpdate(requestId);

        assertTrue(cancellationSignalCaptor.getValue().isCanceled());
    }

    @Test
    @SmallTest
    public void testParseLocationInfoEllipsoidConfigAltitudeAvailable() {
        setLocationPolicyShape(LocationPolicy.SHAPE_ELLIPSOID);
        when(mLocation.hasAltitude()).thenReturn(true);

        deliverMockLocation();
        String[] locationInfo =
                mLocationAgent.getLastKnownLocation(LocationInterface.LOCATION_CATEGORY_ALL);

        assertNotNull(locationInfo);
        assertEquals(LocationPolicy.SHAPE_ELLIPSOID, locationInfo[3]);
        assertEquals(Double.toString(TEST_ALTITUDE), locationInfo[11]);
    }

    @Test
    @SmallTest
    public void testParseLocationInfoEllipsoidConfigAltitudeNotAvailable() {
        setLocationPolicyShape(LocationPolicy.SHAPE_ELLIPSOID);
        when(mLocation.hasAltitude()).thenReturn(false);

        deliverMockLocation();
        String[] locationInfo =
                mLocationAgent.getLastKnownLocation(LocationInterface.LOCATION_CATEGORY_ALL);

        assertNotNull(locationInfo);
        assertEquals(LocationPolicy.SHAPE_CIRCLE, locationInfo[3]);
    }

    @Test
    @SmallTest
    public void testParseLocationInfoCircleConfigAltitudeAvailable() {
        setLocationPolicyShape(LocationPolicy.SHAPE_CIRCLE);
        when(mLocation.hasAltitude()).thenReturn(true);

        deliverMockLocation();
        String[] locationInfo =
                mLocationAgent.getLastKnownLocation(LocationInterface.LOCATION_CATEGORY_ALL);

        assertNotNull(locationInfo);
        assertEquals(LocationPolicy.SHAPE_CIRCLE, locationInfo[3]);
    }

    @Test
    @SmallTest
    public void testParseLocationInfoCircleConfigAltitudeNotAvailable() {
        setLocationPolicyShape(LocationPolicy.SHAPE_CIRCLE);
        when(mLocation.hasAltitude()).thenReturn(false);

        deliverMockLocation();
        String[] locationInfo =
                mLocationAgent.getLastKnownLocation(LocationInterface.LOCATION_CATEGORY_ALL);

        assertNotNull(locationInfo);
        assertEquals(LocationPolicy.SHAPE_CIRCLE, locationInfo[3]);
    }

    @Test
    @SmallTest
    public void testRequestLocationUpdateGpsProviderQuality() {
        LocationManagerProxy lmp = mTestAppContext.getSystemServiceProxy(
                SystemServiceProxy.LocationManagerProxy.class);
        when(lmp.isProviderEnabled(LocationManager.GPS_PROVIDER)).thenReturn(true);
        when(lmp.isProviderEnabled(LocationManager.NETWORK_PROVIDER)).thenReturn(false);
        when(lmp.isProviderEnabled(LocationManager.FUSED_PROVIDER)).thenReturn(false);

        mLocationAgent.requestLocationUpdate(1000);
        verifyLocationRequest(LocationManager.GPS_PROVIDER,
                LocationRequest.QUALITY_HIGH_ACCURACY);
    }

    @Test
    @SmallTest
    public void testRequestLocationUpdateFusedProviderQuality() {
        LocationManagerProxy lmp = mTestAppContext.getSystemServiceProxy(
                SystemServiceProxy.LocationManagerProxy.class);
        when(lmp.isProviderEnabled(LocationManager.GPS_PROVIDER)).thenReturn(false);
        when(lmp.isProviderEnabled(LocationManager.NETWORK_PROVIDER)).thenReturn(false);
        when(lmp.isProviderEnabled(LocationManager.FUSED_PROVIDER)).thenReturn(true);

        mLocationAgent.requestLocationUpdate(1000);
        verifyLocationRequest(LocationManager.FUSED_PROVIDER,
                LocationRequest.QUALITY_HIGH_ACCURACY);
    }

    @Test
    @SmallTest
    public void testRequestLocationUpdateNetworkProviderQuality() {
        LocationManagerProxy lmp = mTestAppContext.getSystemServiceProxy(
                SystemServiceProxy.LocationManagerProxy.class);
        when(lmp.isProviderEnabled(LocationManager.GPS_PROVIDER)).thenReturn(false);
        when(lmp.isProviderEnabled(LocationManager.NETWORK_PROVIDER)).thenReturn(true);
        when(lmp.isProviderEnabled(LocationManager.FUSED_PROVIDER)).thenReturn(false);

        mLocationAgent.requestLocationUpdate(1000);
        verifyLocationRequest(LocationManager.NETWORK_PROVIDER,
                LocationRequest.QUALITY_BALANCED_POWER_ACCURACY);
    }

    private void setLocationPolicyShape(String shape) {
        LocationPolicy lp = mLocationAgent.getLocationPolicy();
        lp.setShape(shape);
        mLocationAgent.setLocationPolicy(lp);
    }

    private void deliverMockLocation() {
        int waitTimeMs = 2000; // 2s
        mLocationAgent.requestLocationUpdate(waitTimeMs);

        LocationManagerProxy lmp = mTestAppContext.getSystemServiceProxy(
                SystemServiceProxy.LocationManagerProxy.class);
        verify(lmp).getCurrentLocation(
                any(String.class),
                any(LocationRequest.class),
                any(CancellationSignal.class),
                any(Executor.class),
                mLocationConsumerCaptor.capture());

        mLocationConsumerCaptor.getValue().accept(mLocation);
    }

    private void verifyLocationRequest(String expectedProvider, int expectedQuality) {
        LocationManagerProxy lmp = mTestAppContext.getSystemServiceProxy(
                SystemServiceProxy.LocationManagerProxy.class);
        verify(lmp).getCurrentLocation(eq(expectedProvider), mLocationRequestCaptor.capture(),
                any(CancellationSignal.class), any(Executor.class), any(Consumer.class));

        LocationRequest request = mLocationRequestCaptor.getValue();
        assertEquals(expectedQuality, request.getQuality());
        assertTrue(request.isLocationSettingsIgnored());
    }
}
