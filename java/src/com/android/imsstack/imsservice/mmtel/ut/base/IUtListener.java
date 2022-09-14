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

/**
 * Provides the UtListener interface for Ut operation.
 */
public interface IUtListener {
    /**
     * Notifies when Ut configuration updates.
     */
    void utConfigurationUpdated(int id);

    /**
     * Notifies when Ut configuration update failed.
     */
    void utConfigurationUpdateFailed(int id, ImsReasonInfo reasonInfo);

    /**
     * Notifies for line Identification Supplementary Service Response
     */
    void lineIdentificationSupplementaryServiceResponse(int id, ImsSsInfo ssInfo);

    /**
     * Notifies when Ut configuration call barring queried.
     */
    void utConfigurationCallBarringQueried(int id, ImsSsInfo[] cbInfo);

    /**
     * Notifies when Ut configuration call forward queried.
     */
    void utConfigurationCallForwardQueried(int id, ImsCallForwardInfo[] cfInfo);

    /**
     * Notifies when Ut configuration call waiting queried.
     */
    void utConfigurationCallWaitingQueried(int id, ImsSsInfo[] cwInfo);

    /**
     * Notifies when Ut configuration query failed.
     */
    void utConfigurationQueryFailed(int id, ImsReasonInfo reasonInfo);
}
