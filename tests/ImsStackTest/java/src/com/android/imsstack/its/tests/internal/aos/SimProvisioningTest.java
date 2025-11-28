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
package com.android.imsstack.its.tests.internal.aos;

import static com.android.imsstack.its.base.TestConstants.SLOT0;
import static com.android.imsstack.its.tests.registration.RegistrationInfo.SimSupportMode;

import android.os.PersistableBundle;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.its.tests.registration.RegistrationHelper;
import com.android.imsstack.its.tests.registration.RegistrationInfo;
import com.android.imsstack.its.tests.registration.tests.RegistrationTestBase;
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
        setUpBase(SLOT0);

        mRegistration = new TestRegistration(mImsServiceConnector.getRegistration());
        createControlConnection(mRegistration);

        mRegistrationHelper = new RegistrationHelper();
        mConfig = new PersistableBundle();
        mInfoBuilder = new RegistrationInfo.Builder();

        setDefaultRegistrationScenario();
    }

    @After
    public void tearDown() throws Exception {
        mServerControlConnection.disconnect();
        tearDownBase(SLOT0);
    }

    /**
     * Mode : BOTH_ISIM_USIM
     * UE supports ISIM application and USIM application.
     */

    // 1. Configure the IMS identity priority to support ISIM and then USIM as a fallback.
    // 2. Set the SIM support mode to BOTH_ISIM_USIM, simulating a device where
    //    both ISIM and USIM are fully supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device successfully registers using the prioritized ISIM.
    @Test
    public void provisioning_supportIsimAndUsim_priorityIsimUsim_succeeds()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_BOTH_ISIM_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.BOTH_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    // 1. Configure the IMS identity priority to support ISIM and then IMSI from ISIM.
    // 2. Set the SIM support mode to BOTH_ISIM_USIM, simulating a device where
    //    both ISIM and USIM are fully supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device successfully registers using the prioritized ISIM.
    @Test
    public void provisioning_supportIsimAndUsim_priorityIsimImsi_succeeds()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ISIM_ISIMIMSI);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.BOTH_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    // 1. Configure the IMS identity priority to support only ISIM.
    // 2. Set the SIM support mode to BOTH_ISIM_USIM, simulating a device where
    //    both ISIM and USIM are fully supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device successfully registers using the ISIM.
    @Test
    public void provisioning_supportIsimAndUsim_priorityIsim_succeeds()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_ISIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.BOTH_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    // 1. Configure the IMS identity priority to support only USIM.
    // 2. Set the SIM support mode to BOTH_ISIM_USIM, simulating a device where
    //    both ISIM and USIM are fully supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device successfully registers using the USIM.
    @Test
    public void provisioning_supportIsimAndUsim_priorityUsim_succeeds()
            throws Exception {
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

    // 1. Configure the IMS identity priority to support ISIM and then USIM as a fallback.
    // 2. Set the SIM support mode to INCOMP_ISIM_USIM, where ISIM is incomplete but USIM is
    //    supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device successfully registers by falling back to the valid USIM
    //    application.
    @Test
    public void provisioning_supportIncompIsimAndUsim_priorityIsimUsim_succeeds()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_BOTH_ISIM_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.INCOMP_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    // 1. Configure the IMS identity priority to support ISIM and then IMSI from ISIM.
    // 2. Set the SIM support mode to INCOMP_ISIM_USIM, where ISIM is incomplete but USIM is
    //    supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device successfully registers, likely using IMSI from the incomplete ISIM.
    @Test
    public void provisioning_supportIncompIsimAndUsim_priorityIsimImsi_succeeds()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ISIM_ISIMIMSI);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.INCOMP_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    // 1. Configure the IMS identity priority to support only ISIM.
    // 2. Set the SIM support mode to INCOMP_ISIM_USIM, where ISIM is incomplete but USIM is
    //    supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device does not register, as the configuration prioritizes only
    //    the incomplete ISIM and does not allow fallback.
    @Test
    public void provisioning_supportIncompIsimAndUsim_priorityIsim_fails()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_ISIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.INCOMP_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expectNot(5000).registered();
    }

    // 1. Configure the IMS identity priority to support only USIM.
    // 2. Set the SIM support mode to INCOMP_ISIM_USIM, where ISIM is incomplete but USIM is
    //    supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device successfully registers using the valid USIM.
    @Test
    public void provisioning_supportIncompIsimAndUsim_priorityUsim_succeeds()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.INCOMP_ISIM_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    /**
     * Mode : ONLY_USIM
     * UE only supports a USIM application.
     */

    // 1. Configure the IMS identity priority to support ISIM and then USIM as a fallback.
    // 2. Set the SIM support mode to ONLY_USIM, where only USIM is supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device successfully registers using the available USIM.
    @Test
    public void provisioning_supportUsim_priorityIsimUsim_succeeds()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_BOTH_ISIM_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.ONLY_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    // 1. Configure the IMS identity priority to support ISIM and then IMSI from ISIM.
    // 2. Set the SIM support mode to ONLY_USIM, where only USIM is supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device successfully registers using the available USIM as a fallback.
    @Test
    public void provisioning_supportUsim_priorityIsimImsi_succeeds()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ISIM_ISIMIMSI);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.ONLY_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }

    // 1. Configure the IMS identity priority to support only ISIM.
    // 2. Set the SIM support mode to ONLY_USIM, where only USIM is supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device does not register, as the required ISIM application is not
    //    present.
    @Test
    public void provisioning_supportUsim_priorityIsim_fails()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_ISIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.ONLY_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expectNot(5000).registered();
    }

    // 1. Configure the IMS identity priority to support only USIM.
    // 2. Set the SIM support mode to ONLY_USIM, where only USIM is supported.
    // 3. Trigger IMS registration.
    // 4. Verify that the device successfully registers using the USIM.
    @Test
    public void provisioning_supportUsim_priorityUsim_succeeds()
            throws Exception {
        mConfig.putIntArray(IMS_IDENTITY_PRIORITY, SUPPORT_ONLY_USIM);
        mInfoBuilder.addConfig(mConfig).setSimSupportMode(SimSupportMode.ONLY_USIM);

        mRegistrationHelper.triggerRegistration(this, mInfoBuilder.build());
        mRegistration.expect(30000).registered();
    }
}
