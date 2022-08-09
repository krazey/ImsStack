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
package com.android.imsstack.enabler;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;

import java.util.concurrent.Executor;

public interface IContext {
    /**
     * Utilities for this context.
     */
    public Context getContext();
    public Executor getExecutor();

    public Handler getDefaultHandler();
    public Looper getDefaultLooper();

    /**
     * Returns the phone id of this context.
     */
    public int getPhoneId();

    /**
     * Returns the slot-id of this context.
     */
    public int getSlotId();

    /**
     * Returns the subscription id of this context.
     */
    public int getSubId();
}
