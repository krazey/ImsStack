/*
 * Copyright (C) 2023 The Android Open Source Project
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
package com.android.imsstack.base;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;

import androidx.annotation.NonNull;

/**
 * An implementation class to monitor the broadcast intent.
 */
public class BroadcastReceiverProxyImpl implements BroadcastReceiverProxy {
    private final Context mContext;
    private final Handler mScheduler;

    BroadcastReceiverProxyImpl(Context context, Handler scheduler) {
        mContext = context;
        mScheduler = scheduler;
    }

    @Override
    public Intent registerReceiver(BroadcastReceiver receiver, IntentFilter filter) {
        return registerReceiver(receiver, filter, mScheduler);
    }

    @Override
    public Intent registerReceiver(BroadcastReceiver receiver, IntentFilter filter,
            @NonNull Handler scheduler) {
        return mContext.registerReceiver(receiver, filter, null, scheduler,
                Context.RECEIVER_EXPORTED);
    }

    @Override
    public void unregisterReceiver(BroadcastReceiver receiver) {
        mContext.unregisterReceiver(receiver);
    }
}
