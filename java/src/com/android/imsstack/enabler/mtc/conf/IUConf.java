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

package com.android.imsstack.enabler.mtc.conf;

import com.android.imsstack.enabler.mtc.IUMtcCall;

public class IUConf {

    public static final int EVENT_U2I        = IUMtcCall.EVENT_U2I + 30;
    public static final int EVENT_I2U        = IUMtcCall.EVENT_I2U + 30;

    // Event : UI to IMS
    public static final int EXPAND              = (EVENT_U2I + 1);
    public static final int MERGE               = (EVENT_U2I + 2);
    public static final int JOIN                = (EVENT_U2I + 3);
    public static final int DROP                = (EVENT_U2I + 4);
    public static final int DELETE              = (EVENT_U2I + 5);

    // Event : IMS to UI
    public static final int EXPANDED                = (EVENT_I2U + 1);
    public static final int EXPANDFAILED            = (EVENT_I2U + 2);
    public static final int EXPANDED_BY             = (EVENT_I2U + 3);
    public static final int MERGED                  = (EVENT_I2U + 4);
    public static final int MERGEFAILED             = (EVENT_I2U + 5);
    public static final int JOINED                  = (EVENT_I2U + 6);
    public static final int DROPPED                 = (EVENT_I2U + 7);
    public static final int DELETED                 = (EVENT_I2U + 8);
    public static final int NOTIFY_USERS_INFO       = (EVENT_I2U + 9);
    public static final int NOTIFY_CONF_INFO        = (EVENT_I2U + 10);

    // Result Type
    public static final int RESULT_FAILURE                = 0;
    public static final int RESULT_SUCCESS                = 1;

    public class Failure_Reason {

        public static final int FAILURE_REASON_UNKNOWN      = 0;

    }; /* REJECT_REASON_ */


}
