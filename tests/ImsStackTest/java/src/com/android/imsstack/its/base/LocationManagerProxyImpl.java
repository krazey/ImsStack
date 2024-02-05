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

import android.annotation.CallbackExecutor;
import android.location.LastLocationRequest;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.util.ArraySet;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.SystemServiceProxy.LocationManagerProxy;

import java.util.concurrent.Executor;

/**
 * An implementation class to access the {@link LocationManager}.
 */
public class LocationManagerProxyImpl implements LocationManagerProxy {
    private boolean mProviderEnabled;
    private Location mLastKnownLocation;
    private final ArraySet<LocationListenerRecord> mListenerRecords = new ArraySet<>();

    @Override
    public boolean isProviderEnabled(@NonNull String provider) {
        return mProviderEnabled;
    }

    @Override
    @Nullable
    public Location getLastKnownLocation(@NonNull String provider,
            @NonNull LastLocationRequest lastLocationRequest) {
        return mLastKnownLocation;
    }

    @Override
    public void requestLocationUpdates(@NonNull String provider,
            @NonNull LocationRequest locationRequest,
            @NonNull @CallbackExecutor Executor executor,
            @NonNull LocationListener listener) {
        mListenerRecords.add(new LocationListenerRecord(locationRequest, listener, executor));
    }

    @Override
    public void removeUpdates(@NonNull LocationListener listener) {
        final ArraySet<LocationListenerRecord> recordsToRemove = new ArraySet<>();
        mListenerRecords.forEach((r) -> {
            if (r.hasListener(listener)) {
                recordsToRemove.add(r);
            }
        });

        recordsToRemove.forEach(mListenerRecords::remove);
    }

    /**
     * Sets the last known location information.
     *
     * @param location The {@link Location} object.
     */
    public void setLastKnownLocation(Location location) {
        mLastKnownLocation = location;
    }

    /**
     * Notifies the application that the location is changed.
     *
     * @param location The updated location information.
     */
    public void notifyLocationChanged(Location location) {
        mListenerRecords.forEach((r) -> r.dispatchLocationChanged(location));
    }

    private static final class LocationListenerRecord {
        private final LocationRequest mRequest;
        private final LocationListener mListener;
        private final Executor mScheduler;

        LocationListenerRecord(LocationRequest request, LocationListener listener,
                Executor scheduler) {
            mRequest = request;
            mListener = listener;
            mScheduler = scheduler;
        }

        boolean hasListener(LocationListener listener) {
            return mListener.equals(listener);
        }

        void dispatchLocationChanged(Location location) {
            mScheduler.execute(() -> mListener.onLocationChanged(location));
        }
    }
}
