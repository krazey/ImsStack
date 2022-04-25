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

package com.android.imsstack.imsservice.mmtel.ut.base;

import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;

public class UtListener {
    public void utConfigurationUpdated(final int id) {
        // no-op
    }

    public void utConfigurationUpdateFailed(final int id, final ImsReasonInfo reasonInfo) {
        // no-op
    }

    public void lineIdentificationSupplementaryServiceResponse(final int id, ImsSsInfo ssInfo) {
        // no-op
    }

    public void utConfigurationCallBarringQueried(final int id, final ImsSsInfo[] cbInfo) {
        // no-op
    }

    public void utConfigurationCallForwardQueried(final int id, final ImsCallForwardInfo[] cfInfo) {
        // no-op
    }

    public void utConfigurationCallWaitingQueried(final int id, final ImsSsInfo[] cwInfo) {
        // no-op
    }

    public void utConfigurationQueryFailed(final int id, final ImsReasonInfo reasonInfo) {
        // no-op
    }
}
