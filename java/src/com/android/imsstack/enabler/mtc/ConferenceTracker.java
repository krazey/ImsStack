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

package com.android.imsstack.enabler.mtc;

import com.android.imsstack.util.ImsArgs;

public interface ConferenceTracker {
    /**
     * Conference related events
     */
    /**
     * No arguments.
     */
    public static final int EVENT_MERGE = 1;

    /**
     * ImsArgs#mArg1 : UsersInfo
     */
    public static final int EVENT_EXTEND_TO_CONFERENCE = 2;
    public static final int EVENT_INVITE_PARTICIPANTS = 3;
    public static final int EVENT_REMOVE_PARTICIPANTS = 4;

    /**
     * ImsArgs#mArg1 : CallInfo
     * ImsArgs#mArg2 : MediaInfo
     * ImsArgs#mArg3 : SuppInfo
     * ImsArgs#mLongArg : optional field, conference call id
     */
    public static final int EVENT_EXTENDED = 5;
    public static final int EVENT_EXTEND_RECEIVED = 6;
    public static final int EVENT_MERGED = 7;

    /**
     * No arguments.
     */
    public static final int EVENT_INVITE_PARTICIPANTS_REQUEST_DELIVERED = 8;
    public static final int EVENT_REMOVE_PARTICIPANTS_REQUEST_DELIVERED = 9;

    /**
     * ImsArgs#mArg1 : CallReasonInfo
     */
    public static final int EVENT_EXTEND_FAILED = 10;
    public static final int EVENT_MERGE_FAILED = 11;
    public static final int EVENT_INVITE_PARTICIPANTS_REQUEST_FAILED = 12;
    public static final int EVENT_REMOVE_PARTICIPANTS_REQUEST_FAILED = 13;

    /**
     * Device's internal events
     */
    /**
     * ImsArgs#mArg1 : UsersInfo
     */
    public static final int EVENT_DELETE_PARTICIPANTS = 51;
    /**
     * No arguments if the operation is successfully done.
     * ImsArgs#mArg1 : CallReasonInfo if the operation is failed.
     */
    public static final int EVENT_DELETE_PARTICIPANTS_REQUEST_COMPLETED = 52;



    public void updateConferenceState(Object conference,
            int event, ImsArgs args);
}
