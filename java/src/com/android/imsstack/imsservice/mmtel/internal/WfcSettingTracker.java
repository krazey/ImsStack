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
package com.android.imsstack.imsservice.mmtel.internal;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.os.Handler;
import android.telephony.SubscriptionManager;

import com.android.ims.ImsConfig;
import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.SettingsUtils;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.util.ImsLog;

public class WfcSettingTracker {
    /** 0 : Wi-Fi only, 1 : Cellular preferred, 2 : Wi-Fi preferred */
    public static final int MODE_NONE = (-1);
    public static final int MODE_WIFI_ONLY
            = ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY;
    public static final int MODE_CELLULAR_PREFERRED
            = ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED;
    public static final int MODE_WIFI_PREFERRED
            = ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED;
    public static final int MODE_IMS_PREFERRED
            = 3; // ImsConfig.WfcModeFeatureValueConstants.IMS_PREFERRED;

    private final IBaseContext mContext;
    private SettingObserver mSettingObserver;
    private ModeObserver mModeObserver;
    private boolean mWfcEnabled;
    private boolean mWfcSettingOn;
    private int mWfcMode;

    public WfcSettingTracker(IBaseContext context) {
        mContext = context;

        init();
    }

    public void dispose() {
        clear();
    }

    public void init() {
        Context context = mContext.getContext();
        int slotId = mContext.getSlotId();

        mWfcEnabled = ImsGlobal.isWfcEnabled(context, slotId);

        if (isWfcEnabled()) {
            mSettingObserver = new SettingObserver(mContext.getDefaultHandler());
            mModeObserver = new ModeObserver(mContext.getDefaultHandler());

            ContentResolver cr = context.getContentResolver();

            mWfcSettingOn = SettingsUtils.isWFCImsEnabled(context, slotId);
            mWfcMode = SettingsUtils.getWFCImsMode(context, slotId);

            SettingsUtils.registerObserverForGlobal(cr,
                    SubscriptionManager.WFC_IMS_ENABLED, mSettingObserver);
            SettingsUtils.registerObserverForGlobal(cr,
                    SubscriptionManager.WFC_IMS_MODE, mModeObserver);
        } else {
            mSettingObserver = null;
            mModeObserver = null;
        }
    }

    public void clear() {
        ContentResolver cr = mContext.getContext().getContentResolver();

        if (mSettingObserver != null) {
            SettingsUtils.unregisterObserver(cr, mSettingObserver);
            mSettingObserver = null;
        }

        if (mModeObserver != null) {
            SettingsUtils.unregisterObserver(cr, mModeObserver);
            mModeObserver = null;
        }

        mWfcEnabled = false;
        mWfcSettingOn = false;
        mWfcMode = MODE_NONE;
    }

    public boolean isWfcAvailable() {
        if (isWfcSettingEditable()) {
            return mWfcSettingOn
                    && ((mWfcMode == MODE_WIFI_ONLY)
                        || (mWfcMode == MODE_WIFI_PREFERRED)
                        || (mWfcMode == MODE_CELLULAR_PREFERRED)
                        || (mWfcMode == MODE_IMS_PREFERRED));
        }

        return isWfcEnabled();
    }

    public boolean isWfcEnabled() {
        return mWfcEnabled;
    }

    public boolean isWfcSettingEditable() {
        return mWfcEnabled;
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    private class SettingObserver extends ContentObserver {
        public SettingObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);

            final boolean wfcSettingOn = SettingsUtils.isWFCImsEnabled(
                    mContext.getContext(), mContext.getSlotId());

            if (mWfcSettingOn != wfcSettingOn) {
                logi("WFC-Setting :: " + mWfcSettingOn + " >> " + wfcSettingOn
                        + "; phoneId=" + mContext.getPhoneId());
                mWfcSettingOn = wfcSettingOn;
            }
        }
    }

    private class ModeObserver extends ContentObserver {
        public ModeObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);

            final int wfcMode = SettingsUtils.getWFCImsMode(
                    mContext.getContext(), mContext.getSlotId());

            if (mWfcMode != wfcMode) {
                logi("WFC-Mode :: " + mWfcMode + " >> " + wfcMode
                        + "; phoneId=" + mContext.getPhoneId());
                mWfcMode = wfcMode;
            }
        }
    }
}
