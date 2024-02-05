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

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.hardware.TriggerEventListener;
import android.util.ArraySet;

import androidx.annotation.Nullable;

import com.android.imsstack.base.SystemServiceProxy.SensorManagerProxy;

/**
 * An implementation class to access the {@link SensorManager}.
 */
public class SensorManagerProxyImpl implements SensorManagerProxy {
    private final SensorManager mSensorManager;
    private final ArraySet<TriggerSensorRecord> mTriggerSensorRecords = new ArraySet<>();

    SensorManagerProxyImpl(Context context) {
        mSensorManager = context.getSystemService(SensorManager.class);
    }

    @Override
    public @Nullable Sensor getDefaultSensor(int type) {
        return mSensorManager.getDefaultSensor(type);
    }

    @Override
    public boolean requestTriggerSensor(TriggerEventListener listener, Sensor sensor) {
        mTriggerSensorRecords.add(new TriggerSensorRecord(listener, sensor));
        return true;
    }

    @Override
    public boolean cancelTriggerSensor(TriggerEventListener listener, Sensor sensor) {
        final ArraySet<TriggerSensorRecord> recordsToRemove = new ArraySet<>();
        mTriggerSensorRecords.forEach((r) -> {
            if (r.hasTriggerEventListener(listener)) {
                recordsToRemove.add(r);
            }
        });

        recordsToRemove.forEach(mTriggerSensorRecords::remove);
        return true;
    }

    /**
     * Notifies the application that the sensor is triggered.
     */
    public void notifyTriggerEvent() {
        mTriggerSensorRecords.forEach((r) -> r.dispatchTriggerEvent());
    }

    private static final class TriggerSensorRecord {
        private final TriggerEventListener mListener;
        private final Sensor mSensor;

        TriggerSensorRecord(TriggerEventListener listener, Sensor sensor) {
            mListener = listener;
            mSensor = sensor;
        }

        boolean hasTriggerEventListener(TriggerEventListener listener) {
            return mListener.equals(listener);
        }

        void dispatchTriggerEvent() {
            mListener.onTrigger(null);
        }
    }
}
