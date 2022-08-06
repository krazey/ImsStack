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

import android.os.Handler;

/**
 * This provides an interface to access, control, and monitor the call settings.
 */
public interface ICallSetting {
    /** Return VoWifi setting is enabled or not in call setting menu */
    boolean isWfcEnabled();

    /** return video call setting is enabled or not in call setting menu */
    boolean isVideoCallEnabled();

    /**
     * return RTT setting value in call setting menu IMS_RTT_MODE_NONE = 0;
     * IMS_RTT_VISIBLE_DURING_CALLS = 1; IMS_RTT_ALWAYS_VISIBLE = 2; IMS_RTT_CAPABLE_OFF = 3;
     */
    int getRTTMode();

    /**
     * notify the setting values to native layer if the setting values are needed from native layer
     */
    void notifySystemEvents();

    /**
     * notify the re-setting values to native layer if the setting values are needed from native
     * layer
     */
    void updateForSetting();

    /**
     * notify the re-setting values to native layer if the roaming setting values are needed from
     * native layer
     */
    void updateForRoamingSetting(boolean bRoaming);

    /** Register mobile data setting */
    void registerForMobileDataSettingChanged(Handler h, int what, Object obj);

    /** Register video call setting */
    void registerForVideoCallSetChanged(Handler h, int what, Object obj);

    /** Register video call romaing setting */
    void registerForVideoCallRoamingSetChanged(Handler h, int what, Object obj);

    /** Register volte  setting */
    void registerForVoLTESetChanged(Handler h, int what, Object obj);

    /** Register volte roaming setting */
    void registerForVoLTERoamingSetChanged(Handler h, int what, Object obj);

    /** Register vowifi setting */
    void registerForVoWIFISetChanged(Handler h, int what, Object obj);

    /** Register video call setting */
    void registerForVoWIFIPreferenceChanged(Handler h, int what, Object obj);

    /** Register vowifi roaming setting */
    void registerForVoWIFIRoamingSetChanged(Handler h, int what, Object obj);

     /** Register rtt roaming setting */
    void registerForDataRoamingSettingChanged(Handler h, int what, Object obj);

    /** Register rtt mode setting */
    void registerForRttModeSettingChanged(Handler h, int what, Object obj);

    /** Unregister mobile data setting */
    void unregisterForMobileDataSettingChanged(Handler h);

    /** Unregister video call setting */
    void unregisterForVideoCallSetChanged(Handler h);

    /** Unregister video call roaming setting */
    void unregisterForVideoCallRoamingSetChanged(Handler h);

    /** Unregister volte setting */
    void unregisterForVoLTESetChanged(Handler h);

    /** Unregister volte roaming setting */
    void unregisterForVoLTERoamingSetChanged(Handler h);

    /** Unregister vowifi setting */
    void unregisterForVoWIFISetChanged(Handler h);

    /** Unregister vowifi preference setting */
    void unregisterForVoWIFIPreferenceChanged(Handler h);

    /** Unregister vowifi roaming setting */
    void unregisterForVoWIFIRoamingSetChanged(Handler h);

    /** Unregister data roaming setting */
    void unregisterForDataRoamingSettingChanged(Handler h);

    /** Unregister rtt mode setting */
    void unregisterForRttModeSettingChanged(Handler h);

    /** REMOVE */
    boolean isNetworkMode3GOnly();
}
