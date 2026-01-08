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

package com.android.imsstack.core.agents;

import android.content.Context;

import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.core.agents.SubsInfoInterface;

public class SubsInfoAgent implements SubsInfoInterface {
    private final int mSlotId;

    public SubsInfoAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        // no-op
    }

    @Override
    public void cleanup() {
        // no-op
    }

    @Override
    public String getPrimaryImpu() {
        return ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_PRIMARY_IMPU, mSlotId);
    }

    @Override
    public boolean isImsEnabled() {
        return !ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_IMS_DISABLED, mSlotId);
    }

    @Override
    public boolean isIsimEnabled() {
        return ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_ISIM_ENABLED, mSlotId);
    }

    @Override
    public boolean isUsimEnabled() {
        return ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_USIM_ENABLED, mSlotId);
    }

    @Override
    public boolean isDebugEnabled() {
        return ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_ENABLED, mSlotId);
    }

    @Override
    public boolean isTestModeEnabled() {
        return ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_TESTMODE_ENABLED, mSlotId);
    }
}
