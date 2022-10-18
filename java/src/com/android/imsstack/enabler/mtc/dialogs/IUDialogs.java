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

package com.android.imsstack.enabler.mtc.dialogs;

import com.android.imsstack.enabler.mtc.IUMtcService;

public class IUDialogs {

    public static final int EVENT_U2I        = IUMtcService.EVENT_U2I + 60;
    public static final int EVENT_I2U        = IUMtcService.EVENT_I2U + 60;

    // Event : UI to IMS

    // Event : IMS to UI
    public static final int NOTIFY_DIALOG_INFO      = (EVENT_I2U + 1);

};
