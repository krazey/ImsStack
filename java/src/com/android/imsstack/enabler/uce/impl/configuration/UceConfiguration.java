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

package com.android.imsstack.enabler.uce.impl.configuration;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.util.ImsLog;

public final class UceConfiguration {
    private int mSlotId;
    private CarrierConfig mCarrierConfig;

    public UceConfiguration(int slotId) {
        mSlotId = slotId;
    }

    public void init() {
        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);

        if (config != null) {
            mCarrierConfig = config.getCarrierConfig();

            if (mCarrierConfig == null) {
                ImsLog.w(mSlotId, "mCarrierConfig is null");
            }
        } else {
            ImsLog.w(mSlotId, "config is null");
        }
    }

    public boolean isUseExpiredEtag() {
        if (mCarrierConfig == null) {
            return false;
        }
        return mCarrierConfig.getBoolean(
                CarrierConfig.ImsUce.KEY_USE_EXPIRED_ETAG_BOOL, false);
    }
}
