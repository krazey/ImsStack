/*
 * Copyright (C) 2006 The Android Open Source Project
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

package com.android.imsstack.util;

import android.os.SystemClock;

import java.io.PrintWriter;
import java.time.Duration;
import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.ArrayDeque;
import java.util.Deque;
import java.util.Iterator;

/** Local storage for log messages */
public final class LocalLog {

    private final Deque<String> mLog;
    private final int mMaxLines;

    /**
     * {@code true} to use log timestamps expressed in local date/time, {@code false} to use log
     * timestamped expressed with the elapsed realtime clock and UTC system clock. {@code false} is
     * useful when logging behavior that modifies device time zone or system clock.
     */
    private final boolean mUseLocalTimestamps;

    public LocalLog(int maxLines) {
        this(maxLines, true /* useLocalTimestamps */);
    }

    public LocalLog(int maxLines, boolean useLocalTimestamps) {
        mMaxLines = Math.max(0, maxLines);
        mLog = new ArrayDeque<>(mMaxLines);
        mUseLocalTimestamps = useLocalTimestamps;
    }

    /** Store the log message */
    public void log(String msg) {
        if (mMaxLines <= 0) {
            return;
        }
        final String logLine;
        if (mUseLocalTimestamps) {
            logLine = LocalDateTime.now(ZoneId.systemDefault()) + " - " + msg;
        } else {
            logLine = Duration.ofMillis(SystemClock.elapsedRealtime())
                    + " / " + Instant.now() + " - " + msg;
        }
        append(logLine);
    }

    private synchronized void append(String logLine) {
        while (mLog.size() >= mMaxLines) {
            mLog.remove();
        }
        mLog.add(logLine);
    }

    /**
     * Dumps the content of local log to print writer with each log entry predeced with indent
     *
     * @param pw printer writer to write into
     */
    public synchronized void dump(PrintWriter pw) {
        Iterator<String> itr = mLog.iterator();
        while (itr.hasNext()) {
            pw.printf("%s\n", itr.next());
        }
    }
}
