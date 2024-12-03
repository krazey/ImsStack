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
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class LocationAgentTest {
    @Mock private ConfigInterface mConfigInterface;
    @Mock private CarrierConfig mCarrierConfig;
    @Mock private SubsInfoInterface mSubsInfoInterface;
    @Mock private LocationInterface.Listener mLocationInfoListener;

    private TestAppContext mTestAppContext;
    private LocationAgent mLocationAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();

        AgentFactory.getInstance().setAgent(ConfigInterface.class, mConfigInterface, SLOT0);
        AgentFactory.getInstance().setAgent(SubsInfoInterface.class, mSubsInfoInterface, SLOT0);

        mLocationAgent = new LocationAgent(SLOT0);
        mLocationAgent.init(mTestAppContext.getContext());
        mLocationAgent.addListener(mLocationInfoListener);
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
        when(mConfigInterface.getCarrierConfig()).thenReturn(mCarrierConfig);
        when(mSubsInfoInterface.isTestModeEnabled()).thenReturn(true);
        when(mCarrierConfig.getInt(eq(CarrierConfig.Ims.KEY_LOCATION_POLICY_UPDATE_TYPE_INT)))
                .thenReturn(CarrierConfig.Assets.LOCATION_UPDATE_POLICY_ALWAYS);
        int configuredPolicy = LocationPolicy.POLICY_LOCATION_NOT_ALLOWED_PERIODIC_POLLING
                | LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;
        when(mCarrierConfig.getInt(eq(CarrierConfig.Ims.KEY_LOCATION_ACQUISITION_POLICY_INT)))
                .thenReturn(configuredPolicy);
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfig.Ims.KEY_LOCATION_ALLOW_MOCK_LOCATION_UPDATE_BOOL)))
                .thenReturn(true);
        int addressResolutionTime = 1000;
        when(mCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_LOCATION_ADDRESS_RESOLUTION_TIME_MILLIS_INT)))
                .thenReturn(addressResolutionTime);
        int validityMinutes = 1000;
        when(mCarrierConfig.getInt(eq(CarrierConfig.Ims.KEY_LOCATION_VALIDITY_PERIOD_MIN_INT)))
                .thenReturn(validityMinutes);
        int locationValidityMinutes = 1000;
        when(mCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_LOCATION_ADDRESS_VALIDITY_PERIOD_MIN_INT)))
                .thenReturn(locationValidityMinutes);
        int addressTolerableDistance = 150;
        when(mCarrierConfig.getInt(eq(CarrierConfig.Ims.KEY_LOCATION_TOLERABLE_DISTANCE_INT)))
                .thenReturn(addressTolerableDistance);
        int gpsSearchingDuration = 20;
        when(mCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_LOCATION_GPS_SEARCHING_DURATION_SEC_INT)))
                .thenReturn(gpsSearchingDuration);
        when(mCarrierConfig.getInt(eq(CarrierConfig.Ims.KEY_LOCATION_GEODETIC_SHAPE_INT)))
                .thenReturn(CarrierConfig.Assets.GEODETIC_SHAPE_ELLIPSOID);

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
    public void testNotifyListenersWhenLocationFixed() {
        mLocationAgent.startInstantLocationUpdate();

        mLocationAgent.notifyEventOnLocationFixedForInstantRequest();

        verify(mLocationInfoListener).onInstantRequestedLocationUpdated();
    }
}
