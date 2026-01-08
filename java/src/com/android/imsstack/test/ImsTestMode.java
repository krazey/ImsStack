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
package com.android.imsstack.test;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.util.ImsLog;

import java.util.HashMap;
import java.util.Map;

/**
 * IMS Test Mode
 */
public final class ImsTestMode {
    public static final int CONFIG_NONE = 0;
    /**
     * Bitmask: Framework level test cases (0x01 ~ 0x80)
     */
    public static final int CONFIG_IMS_OFF = 0x00000001;
    public static final int CONFIG_DEBUG = 0x00000002;
    public static final int CONFIG_GENERIC_TEST_MODE = 0x00000004;
    /**
     * Bitmask: Call level test cases (0x0100 ~ 0x8000)
     */
    public static final int CONFIG_LOCAL_HOLD_TONE = 0x00000100;

    /**
     * Bitmask: for getExtraTestmask()
     */
    public static final int TEST_MASK_ROAMING_CONDITION = 0x0002;
    public static final int TEST_MASK_IMS_STATUS_TO_UICC_OFF = 0x0008;

    private static ImsTestMode sImsTestMode = new ImsTestMode();

    private static Map<Integer, IImsTestMode> sTestModes =
            new HashMap<Integer, IImsTestMode>(DeviceConfig.getSupportedSimCount());


    public static ImsTestMode getInstance() {
        return sImsTestMode;
    }

    private ImsTestMode() {
    }

    public synchronized void init(int slotId) {
        ImsLog.d("SLOT = " + slotId);

        IImsTestMode itm = sTestModes.get(slotId);
        if (itm != null) {
            return;
        }

        TestMode tm = new TestMode(slotId);
        tm.init();
        sTestModes.put(slotId, tm);
    }

    public synchronized void cleanUp(int slotId) {
        ImsLog.d("SLOT = " + slotId);

        TestMode tm = (TestMode) sTestModes.get(slotId);
        if (tm == null) {
            return;
        }

        tm.cleanup();
        sTestModes.remove(slotId);
    }

    public synchronized IImsTestMode getTestMode(int slotId) {
        IImsTestMode itm = sTestModes.get(slotId);
        if (itm != null) {
            return itm;
        }

        TestMode tm = new TestMode(slotId);
        tm.init();
        sTestModes.put(slotId, tm);
        return tm;
    }

    private static class TestMode implements IImsTestMode {
        private final int mSlotId;
        private int mTestmodes = CONFIG_NONE;
        private int mExtraTestmask = 0;

        TestMode(int slotId) {
            mSlotId = slotId;
        }

        public void init() {
            initFromConfig();

            if (isGenericTestMode() && isDebugEnabled()) {
                enableTestmode(CONFIG_LOCAL_HOLD_TONE);
            }

            ImsLog.i("[ ImsTestMode("+ mSlotId + ") : testmodes="
                    + String.format("%08x", mTestmodes)
                    + ", extraTestmask=" + String.format("%08x", mExtraTestmask) + " ]");
        }

        public void cleanup() {
        }

        @Override
        public int getExtraTestmask() {
            return mExtraTestmask;
        }

        @Override
        public boolean isDebugEnabled() {
            return isConfigEnabled(CONFIG_DEBUG);
        }

        @Override
        public boolean isDebuggable() {
            return isDebugEnabled() || isGenericTestMode();
        }

        @Override
        public boolean isImsOff() {
            return isConfigEnabled(CONFIG_IMS_OFF);
        }

        @Override
        public boolean isLocalHoldToneEnabled() {
            return isConfigEnabled(CONFIG_LOCAL_HOLD_TONE);
        }

        @Override
        public boolean isGenericTestMode() {
            return isConfigEnabled(CONFIG_GENERIC_TEST_MODE);
        }

        private void initFromConfig() {
            ImsLog.i("ImsTestMode(" + mSlotId + ") : init");

            if (ImsPrivateProperties.Persistent.getBoolean(
                    ImsPrivateProperties.Persistent.KEY_TEST_IMS_DISABLED, mSlotId)) {
                enableTestmode(CONFIG_IMS_OFF);
            }

            if (ImsPrivateProperties.Persistent.getBoolean(
                    ImsPrivateProperties.Persistent.KEY_TEST_TESTMODE_ENABLED, mSlotId)) {
                enableTestmode(CONFIG_GENERIC_TEST_MODE);
            }

            if (ImsPrivateProperties.Persistent.getBoolean(
                    ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_ENABLED, mSlotId)) {
                enableTestmode(CONFIG_DEBUG);
            }
        }

        private void enableTestmode(int enabledTestmodes) {
            mTestmodes |= enabledTestmodes;
        }

        private boolean isConfigEnabled(int config) {
            return (mTestmodes & config) == config;
        }
    }
}
