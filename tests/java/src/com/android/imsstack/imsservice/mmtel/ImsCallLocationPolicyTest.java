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
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.telephony.ims.ImsCallProfile;
import android.test.mock.MockContentResolver;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

import java.util.concurrent.ConcurrentHashMap;

@RunWith(JUnit4.class)
public class ImsCallLocationPolicyTest extends ImsStackTest {
    /* Indicates that geolocation information is required to make a call */
    private static final int FLAG_LOCATION_REQUIRED = 0x00000001;
    /* Indicates that geolocation information is required for emergency call only */
    private static final int FLAG_EMERGENCY_CALL_ONLY = 0x00000004;
    /* Indicates that geolocation information is required for Wi-Fi call only */
    private static final int FLAG_WIFI_CALL_ONLY = 0x00000008;
    /* Indicates that number list is specified and emergency call needs the location */
    private static final int FLAG_NUMBER_LIST_AND_EMERGENCY_CALL = 0x00000010;

    private static final int LOCATION_FOR_EMERGENCY_ONLY =
            FLAG_LOCATION_REQUIRED | FLAG_EMERGENCY_CALL_ONLY;
    private static final int LOCATION_FOR_WIFI_ONLY = FLAG_LOCATION_REQUIRED | FLAG_WIFI_CALL_ONLY;
    private static final int LOCATION_FOR_EMERGENCY_WIFI_ONLY =
            FLAG_LOCATION_REQUIRED | FLAG_WIFI_CALL_ONLY | FLAG_EMERGENCY_CALL_ONLY;
    private static final int LOCATION_FOR_EMERGENCY_NUMBER_LIST_ONLY = FLAG_LOCATION_REQUIRED
            | FLAG_EMERGENCY_CALL_ONLY | FLAG_NUMBER_LIST_AND_EMERGENCY_CALL;
    private static final int LOCATION_FOR_EMERGENCY_WIFI_NUMBER_LIST_ONLY = FLAG_LOCATION_REQUIRED
            | FLAG_WIFI_CALL_ONLY | FLAG_EMERGENCY_CALL_ONLY | FLAG_NUMBER_LIST_AND_EMERGENCY_CALL;

    private ImsCallContext mMockCallContext;
    private CarrierConfig mMockCarrierConfig;
    private ConfigInterface mMockConfigInterface;
    private ImsCallLocationPolicy mImsCallLocationPolicy;
    private Context mMockContext;

    @Before
    public void setUp() throws Exception {
        mContextFixture = new ContextFixture();
        AppContext.init(mContextFixture.getTestDouble());

        mMockContext = Mockito.mock(Context.class);
        mMockCallContext = Mockito.mock(ImsCallContext.class);
        mMockCarrierConfig = Mockito.mock(CarrierConfig.class);
        mMockConfigInterface = Mockito.mock(ConfigInterface.class);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCallContext.getSlotId()).thenReturn(MSimUtils.DEFAULT_SLOT_ID);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface,
                MSimUtils.DEFAULT_SLOT_ID);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
    }

    @After
    public void tearDown() throws Exception {
        mImsCallLocationPolicy = null;
        mContextFixture = null;
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, MSimUtils.DEFAULT_SLOT_ID);
        AppContext.deinit();
        ImsServiceManager ism = ImsServiceManager.getDefault();
        if (ism != null) {
            ism.dispose();
            ImsServiceManager.setDefault(null);
        }
    }

    @Test
    public void testInit() {
        int[] intArray = {150, 155};
        int[] emptyArray = new int[0];
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Ims.KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT))
                .thenReturn(LOCATION_FOR_WIFI_ONLY);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsVoice.KEY_LOCATION_BASED_NUMBER_LIST_INT_ARRAY))
                .thenReturn(intArray);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertTrue(mImsCallLocationPolicy.isPositionInfoRequired());
        assertFalse(mImsCallLocationPolicy.getNumberSet().isEmpty());
        assertTrue(mImsCallLocationPolicy.getNumberSet().contains("150"));

        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsVoice.KEY_LOCATION_BASED_NUMBER_LIST_INT_ARRAY))
                .thenReturn(emptyArray);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertTrue(mImsCallLocationPolicy.getNumberSet().isEmpty());
    }

    @Test
    public void testPublicIsLocationRequired() {
        assertFalse(mImsCallLocationPolicy.isLocationRequired(mContext, MSimUtils.DEFAULT_SLOT_ID));

        when(mMockCarrierConfig.getInt(
                CarrierConfig.Ims.KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT))
                .thenReturn(FLAG_LOCATION_REQUIRED);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertTrue(mImsCallLocationPolicy.isLocationRequired(mContext, MSimUtils.DEFAULT_SLOT_ID));
    }

    @Test
    public void testIsPositionInfoRequired() {
        assertTrue(mImsCallLocationPolicy.isPositionInfoRequired());
    }

    @Test
    public void testGetValidityPeriod() {
        assertEquals(24 * 60 * 60 * 1000 * 1000000L, mImsCallLocationPolicy.getValidityPeriod());
    }

    @Test
    public void testGetWaitingTimeForLocationFix() {
        assertEquals(5 * 1000L, mImsCallLocationPolicy.getWaitingTimeForLocationFix());
    }

    @Test
    public void testIsLocationRequired() {
        assertFalse(mImsCallLocationPolicy.isLocationRequired("150", null));

        /*verify isLocationRequiredFromCallInfo()
         *-isEmergencyCall() -> false , isWifiCall() -> true */
        ImsCallProfile callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Ims.KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT))
                .thenReturn(LOCATION_FOR_EMERGENCY_WIFI_ONLY);
        mockForIsWiFi(true);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertFalse(mImsCallLocationPolicy.isLocationRequired("910", callProfile));

        /*verify isLocationRequiredFromCallInfo()
         *-isEmergencyCall() -> true , isWifiCall() -> false */
        callProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_EMERGENCY,
                ImsCallProfile.CALL_TYPE_VOICE);
        mockForIsWiFi(false);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertFalse(mImsCallLocationPolicy.isLocationRequired("150", callProfile));

        /*verify isLocationRequiredFromCallInfo()
         *-isEmergencyCall() -> true , isWifiCall() -> true */
        mockForIsWiFi(true);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertTrue(mImsCallLocationPolicy.isLocationRequired("910", callProfile));

        //verify !isLocationRequiredForNumberListAndECall()
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Ims.KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT))
                .thenReturn(LOCATION_FOR_EMERGENCY_ONLY);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertTrue(mImsCallLocationPolicy.isLocationRequired("910", callProfile));

        //verify isLocationRequiredForWifiCallOnly()
        mockForIsWiFi(true);
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Ims.KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT))
                .thenReturn(LOCATION_FOR_WIFI_ONLY);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertTrue(mImsCallLocationPolicy.isLocationRequired("910", callProfile));

        //verify mNumberSet.contains -> true , it will return form isWifiCall() true
        int[] intArray = new int[]{-1, 150, 155};
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsVoice.KEY_LOCATION_BASED_NUMBER_LIST_INT_ARRAY))
                .thenReturn(intArray);
        mockForIsWiFi(true);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertTrue(mImsCallLocationPolicy.isLocationRequired("150", callProfile));

        //verify mNumberSet.contains -> false and isLocationRequiredForNumberListAndECall -> true
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Ims.KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT))
                .thenReturn(LOCATION_FOR_EMERGENCY_WIFI_NUMBER_LIST_ONLY);
        mockForIsWiFi(true);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertTrue(mImsCallLocationPolicy.isLocationRequired("910", callProfile));

        when(mMockCarrierConfig.getInt(
                CarrierConfig.Ims.KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT))
                .thenReturn(LOCATION_FOR_EMERGENCY_NUMBER_LIST_ONLY);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertTrue(mImsCallLocationPolicy.isLocationRequired("910", callProfile));

        when(mMockCarrierConfig.getInt(
                CarrierConfig.Ims.KEY_GEOLOCATION_POLICY_FOR_LOCATION_BASED_CALL_INT))
                .thenReturn(FLAG_LOCATION_REQUIRED);
        mImsCallLocationPolicy = new ImsCallLocationPolicy(mMockCallContext);
        assertFalse(mImsCallLocationPolicy.isLocationRequired("910", callProfile));

        ImsServiceManager.getDefault().dispose();
        ImsServiceManager.setDefault(null);
    }

    private void mockForIsWiFi(boolean isRegistered) {
        MessageExecutor executor = new MessageExecutor(ImsServiceManager.class.getSimpleName());
        ImsServiceRecord mockServiceRecord = Mockito.mock(ImsServiceRecord.class);
        ImsRegistrationTracker mockImsRegTracker = Mockito.mock(ImsRegistrationTracker.class);
        Context mockContext = Mockito.mock(Context.class);
        MockContentResolver mockContentResolver = new MockContentResolver();
        when(mMockCallContext.getSlotId()).thenReturn(MSimUtils.DEFAULT_SLOT_ID);
        when(mockServiceRecord.getRegistrationTracker()).thenReturn(mockImsRegTracker);
        when(mockImsRegTracker.isCallRegistered()).thenReturn(isRegistered);
        when(mockImsRegTracker.getRegisteredNetworkType()).thenReturn(
                IAosRegistrationListener.NetworkType.IWLAN);
        when(mockContext.getContentResolver()).thenReturn(mockContentResolver);

        ImsServiceManager serviceManager = ImsServiceManager.getDefault();
        if (serviceManager != null) {
            serviceManager.dispose();
        }
        serviceManager = new ImsServiceManager(mockContext, executor);
        ImsServiceManager.setDefault(serviceManager);
        ConcurrentHashMap<Integer, ImsServiceRecord> serviceMap =
                serviceManager.getServiceRecordMap();
        serviceMap.put(MSimUtils.DEFAULT_SLOT_ID, mockServiceRecord);
    }
}

