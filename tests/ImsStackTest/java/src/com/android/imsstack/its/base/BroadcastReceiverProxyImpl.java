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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.util.ArraySet;

import androidx.annotation.NonNull;

import com.android.imsstack.base.BroadcastReceiverProxy;

/**
 * An implementation class to monitor the broadcast intent.
 */
public class BroadcastReceiverProxyImpl implements BroadcastReceiverProxy {
    private final Context mContext;
    private final Handler mScheduler;
    private final ArraySet<ReceiverRecord> mReceiverRecords = new ArraySet<>();

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
        if (receiver != null) {
            mReceiverRecords.add(new ReceiverRecord(receiver, filter, scheduler));
        }
        return null;
    }

    @Override
    public void unregisterReceiver(BroadcastReceiver receiver) {
        final ArraySet<ReceiverRecord> recordsToRemove = new ArraySet<>();
        mReceiverRecords.forEach((r) -> {
            if (r.hasBroadcastReceiver(receiver)) {
                recordsToRemove.add(r);
            }
        });

        recordsToRemove.forEach(mReceiverRecords::remove);
    }

    /**
     * Sends the specified broadcast intent.
     */
    public void sendIntent(@NonNull Intent intent) {
        mReceiverRecords.forEach((r) -> {
            if (r.hasAction(intent.getAction())) {
                r.dispatchIntent(mContext, intent);
            }
        });
    }

    private static final class ReceiverRecord {
        private final BroadcastReceiver mReceiver;
        private final IntentFilter mFilter;
        private final Handler mScheduler;

        ReceiverRecord(BroadcastReceiver receiver, IntentFilter filter, Handler scheduler) {
            mReceiver = receiver;
            mFilter = filter;
            mScheduler = scheduler;
        }

        boolean hasAction(String action) {
            return mFilter.hasAction(action);
        }

        boolean hasBroadcastReceiver(BroadcastReceiver receiver) {
            return mReceiver.equals(receiver);
        }

        void dispatchIntent(Context context, Intent intent) {
            mScheduler.post(() -> {
                mReceiver.onReceive(context, intent);
            });
        }
    }
}
