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
package com.android.imsstack.its.tests.registration;

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;

import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.imsservice.reg.ImsRegistrationWrapper;
import com.android.imsstack.its.tests.ImsStackTestBase;

public class RegistrationTestBase extends ImsStackTestBase {
    protected TelephonyManagerProxyImpl mTelephony;
    protected ImsRegistrationWrapper mImsRegistration;
    protected RegistrationHelper mRegistrationHelper;
    protected PersistableBundle mConfig = null;

    protected void setRegistrationBaseConfig(int slotId) {
        mConfig = new PersistableBundle();
        mConfig.putBoolean(CarrierConfigManager.Ims.KEY_SIP_OVER_IPSEC_ENABLED_BOOL, false);
    }
}
