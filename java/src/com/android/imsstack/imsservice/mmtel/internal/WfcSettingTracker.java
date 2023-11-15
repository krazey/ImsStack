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

import android.telephony.ims.ImsMmTelManager;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.ImsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ImsMmTelManagerProxy;
import com.android.imsstack.core.config.ServiceCaps;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.util.ImsLog;

public class WfcSettingTracker {
    private final IBaseContext mContext;

    public WfcSettingTracker(IBaseContext context) {
        mContext = context;

        init();
    }

    public void dispose() {
        clear();
    }

    public void init() {
    }

    public void clear() {
    }

    public boolean isWfcAvailable() {
        if (isWfcSettingEditable()) {
            if (isVoWiFiSettingEnabled()) {
                int wfcMode = getVoWiFiModeSetting();
                return wfcMode == ImsMmTelManager.WIFI_MODE_WIFI_ONLY
                        || wfcMode == ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED
                        || wfcMode == ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED;
            } else {
                return false;
            }
        }

        return isWfcEnabled();
    }

    public boolean isWfcEnabled() {
        return ServiceCaps.isWfcEnabledByPlatform(mContext.getSlotId());
    }

    public boolean isWfcSettingEditable() {
        return isWfcEnabled();
    }

    private ImsMmTelManagerProxy getImsMmTelManagerProxy() {
        int subId = MSimUtils.getSubId(mContext.getSlotId());
        ImsManagerProxy imp = AppContext.getInstance().getSystemServiceProxy(ImsManagerProxy.class);
        return imp.getImsMmTelManagerProxy(subId);
    }

    private boolean isVoWiFiSettingEnabled() {
        ImsMmTelManagerProxy imtmp = getImsMmTelManagerProxy();

        try {
            return imtmp != null ? imtmp.isVoWiFiSettingEnabled() : false;
        } catch (Exception e) {
            ImsLog.e(mContext.getSlotId(), "isVoWiFiSettingEnabled: " + e.toString());
            return false;
        }
    }

    private int getVoWiFiModeSetting() {
        ImsMmTelManagerProxy imtmp = getImsMmTelManagerProxy();

        try {
            return imtmp != null
                    ? imtmp.getVoWiFiModeSetting()
                    : ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED;
        } catch (Exception e) {
            ImsLog.e(mContext.getSlotId(), "getVoWiFiModeSetting: " + e.toString());
            return ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED;
        }
    }
}
