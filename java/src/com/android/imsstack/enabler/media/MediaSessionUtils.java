/*
 * Copyright (C) 2025 The Android Open Source Project
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

package com.android.imsstack.enabler.media;

/**
 * A utility class for common media session related operations.
 */
public final class MediaSessionUtils {

    /**
     * Private constructor to prevent instantiation of this utility class.
     */
    private MediaSessionUtils() {}

    /**
     * Determines if a message should be discarded when the session is in the process of closing.
     *
     * <p>Certain messages, like the confirmation of a session closure, should not be discarded.
     *
     * @param requestType The type of the message request from {@link MediaConstants}.
     * @return {@code true} if the message should be discarded, {@code false} otherwise.
     */
    public static boolean isDiscardRequired(int requestType) {
        return (requestType != MediaConstants.RESPONSE_SESSION_CLOSED
                && requestType != MediaConstants.RESPONSE_SESSION_CLOSED_TIMEOUT
                && requestType != MediaConstants.NOTIFY_MEDIA_DETACH);
    }

    /**
     * Determines if a message should wait for the session to be established.
     *
     * <p>Certain messages, like the request to open a session, should not wait.
     *
     * @param requestType The type of the message request from {@link MediaConstants}.
     * @return {@code true} if the message should wait, {@code false} otherwise.
     */
    public static boolean isWaitRequired(int requestType) {
        return (requestType != MediaConstants.REQUEST_OPEN_SESSION
                && requestType != MediaConstants.RESPONSE_OPEN_SESSION
                && requestType != MediaConstants.REQUEST_QOS
                && requestType != MediaConstants.NOTIFY_MEDIA_DETACH);
    }
}
