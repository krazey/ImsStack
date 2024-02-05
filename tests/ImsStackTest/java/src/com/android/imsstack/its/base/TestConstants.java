/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.base;

import android.telephony.SubscriptionManager;

/**
 * This class defines the general constant values for testing.
 */
public interface TestConstants {
    // Invalid slot id.
    int INVALID_SLOT_ID = -1;
    // Invalid subscription id.
    int INVALID_SUB_ID = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

    // Slot 0.
    int SLOT0 = 0;
    // Slot 1.
    int SLOT1 = 1;
    // Subscription id for {@link #SLOT0}.
    int SUB_ID_1 = 1;
    // Subscription id for {@link #SLOT1}.
    int SUB_ID_2 = 2;

    // SIM serial numbers
    String SIM_SERIAL_NUMBER_1 = "1122334455";
    String SIM_SERIAL_NUMBER_2 = "5566778899";

    // Phone numbers
    String PHONE_NUMBER_1 = "1123456789";
    String PHONE_NUMBER_2 = "1234567890";

    // MCC + MNC
    String MCC = "001";
    String MNC = "01";
    String MCC_MNC = MCC + MNC;
}
