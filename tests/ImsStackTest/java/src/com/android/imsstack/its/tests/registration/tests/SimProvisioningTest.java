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
package com.android.imsstack.its.tests.registration.tests;

import static com.android.imsstack.its.base.TestConstants.SLOT0;
import static com.android.imsstack.its.tests.registration.RegistrationInfo.SimSupportMode;

import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.its.base.SystemProxyResolver;
import com.android.imsstack.its.tests.registration.RegistrationHelper;
import com.android.imsstack.its.tests.registration.RegistrationInfo;
import com.android.imsstack.its.tests.registration.util.TestRegistration;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class SimProvisioningTest extends RegistrationTestBase {

    public static final String IMS_IDENTITY_PRIORITY =
            CarrierConfig.Ims.KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY;
    private static final int[] SUPPORT_BOTH_ISIM_USIM = {0, 1};
    private static final int[] SUPPORT_ISIM_ISIMIMSI = {0, 2};
    private static final int[] SUPPORT_ONLY_ISIM = {0};
    private static final int[] SUPPORT_ONLY_USIM = {1};

    @Before
    public void setUp() throws Exception {

        setRegistrationBaseConfig();

        mTelephony = SystemProxyResolver.getTelephonyManagerProxy(getSubId(SLOT0));
        mTelephony.setHalVersion(-2, -2);

        setUpBase(SLOT0);

        mRegistration = new TestRegistration(mImsServiceConnector.getRegistration());
        mRegistrationHelper = new RegistrationHelper();

        mInfoBuilder = new RegistrationInfo.Builder().setConfig(mConfig);
    }

    @After
    public void tearDown() throws Exception {
        tearDownBase(SLOT0);
    }

    /**
     * Mode : BOTH_ISIM_USIM
     * UE supports ISIM application and USIM application.
     */
    @Test
    public void testTriggerRegistration_modeIsimUsim_priorityIsimUsim() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_BOTH_ISIM_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.BOTH_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    @Test
    public void testTriggerRegistration_modeIsimUsim_priorityIsimAndIsimImsi() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ISIM_ISIMIMSI);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.BOTH_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    @Test
    public void testTriggerRegistration_modeIsimUsim_priorityIsim() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_ISIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.BOTH_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    @Test
    public void testTriggerRegistration_modeIsimUsim_priorityUsim() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.BOTH_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    /**
     * Mode : INCOMP_ISIM_USIM
     * UE supports incomplete ISIM application and USIM application.
     * Incomplete ISIM : Simulates a scenario where IMPU, IMPI, and Home network domain
     *                   cannot be obtained from the ISIM.
     */
    @Test
    public void testTriggerRegistration_modeIncompIsimUsim_priorityIsimUsim() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_BOTH_ISIM_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.INCOMP_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    @Test
    public void testTriggerRegistration_modeIncompIsimUsim_priorityIsimAndIsimImsi()
            throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ISIM_ISIMIMSI);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.INCOMP_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    @Test
    public void testTriggerRegistration_modeIncompIsimUsim_priorityIsim() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_ISIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.INCOMP_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expectNot(5000).registered();
    }

    @Test
    public void testTriggerRegistration_modeIncompIsimUsim_priorityUsim() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.INCOMP_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    /**
     * Mode : ONLY_USIM
     * UE only supports a USIM application.
     */
    @Test
    public void testTriggerRegistration_modeUsim_priorityIsimUsim() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_BOTH_ISIM_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.ONLY_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    @Test
    public void testTriggerRegistration_modeUsim_priorityIsimAndIsimImsi() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ISIM_ISIMIMSI);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.ONLY_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    @Test
    public void testTriggerRegistration_modeUsim_priorityIsim() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_ISIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.ONLY_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expectNot(5000).registered();
    }

    @Test
    public void testTriggerRegistration_modeUsim_priorityUsim() throws Exception {

        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.ONLY_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }
}
