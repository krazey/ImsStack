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
package com.android.imsstack.core.agents;

import android.annotation.NonNull;
import android.location.LastLocationRequest;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.util.ArraySet;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.LocationManagerProxy;
import com.android.imsstack.util.ImsLog;

import java.util.Set;
import java.util.concurrent.Executor;

/**
 * This class provides utilities classes/methods for Location information retrieval.
 */
public final class LocationApi {
    public static final String GPS_PROVIDER = LocationManager.GPS_PROVIDER;
    public static final String NETWORK_PROVIDER = LocationManager.NETWORK_PROVIDER;
    public static final String FUSED_PROVIDER = LocationManager.FUSED_PROVIDER;
    public static final String PROVIDERS_CHANGED_ACTION =
            LocationManager.PROVIDERS_CHANGED_ACTION;

    public static final int FLAG_NONE = 0x00000000;
    // If current user is the same user which IMS process runs,
    // then it will obtain the location information from LocationManager directly.
    public static final int FLAG_SELECT_LOCATION_SCHEME_BY_CURRENT_USER = 0x00000001;

    /** Wrapper interface for LocationListener */
    public interface Listener extends LocationListener {
        /** Notifies the application that the service is disconnected. */
        void onServiceDisconnected();
    }

    // It indicates that Location API is ready to use.
    // If it's not set, LocationManager will be used as a default.
    private static final int FLAG_LOCATION_API_READY = 0x80000000;
    private static final boolean ALWAYS_PROVIDER_ENABLED = true;

    private static LocationApi sLocationApi = null;
    private final Object mLock = new Object();
    private int mFlag = FLAG_NONE;
    // Listeners for LocationManager
    private ListenerPool mListenerPool = null;

    /* package */ LocationApi() {
    }

    /** Returns the LocationApi instance. */
    public static LocationApi getInstance() {
        if (sLocationApi == null) {
            sLocationApi = new LocationApi();
        }

        return sLocationApi;
    }

    /**
     * Adds the specified flag.
     *
     * @param flag a flag to be added
     */
    public void addFlag(int flag) {
        synchronized (mLock) {
            mFlag |= flag;
        }
    }

    /**
     * Clears the specified flag.
     *
     * @param flag a flag to be cleared
     */
    public void clearFlag(int flag) {
        synchronized (mLock) {
            mFlag &= (flag);
        }
    }

    /**
     * Starts the location API use.
     */
    public void start() {
        if (isLocationApiReady()) {
            return;
        }

        logi(this, "start - 0x" + Integer.toHexString(mFlag));

        addFlag(FLAG_LOCATION_API_READY);

        if (isLocationSchemeSelectedByCurrentUser()) {
            mListenerPool = new ListenerPool();
        }
    }

    /**
     * Stops the location API use.
     */
    public void stop() {
        if (!isLocationApiReady()) {
            return;
        }

        clearFlag(FLAG_LOCATION_API_READY);

        logi(this, "stop - 0x" + Integer.toHexString(mFlag));

        if (mListenerPool != null) {
            mListenerPool.notifyServiceDisconnectedForLocationManagerApi();
        }
    }

    /**
     * Returns the current enabled/disabled status of the given provider.
     *
     * @param provider the name of the provider
     * @return true if the provider exists and is enabled
     */
    public boolean isProviderEnabled(String provider) {
        if (ALWAYS_PROVIDER_ENABLED) {
            return true;
        }

        return LocationManagerApi.isProviderEnabled(provider);
    }

    /**
     * Returns a Location indicating the data from the last known
     * location fix obtained from the given provider.
     *
     * @param provider the name of the provider
     * @return the last known location for the provider, or null
     */
    public Location getLastKnownLocation(String provider) {
        return LocationManagerApi.getLastKnownLocation(provider);
    }

    /**
     * Register for location updates from the specified provider, using a {@link LocationRequest},
     * and a callback on the specified {@link Executor}.
     *
     * @param provider the name of the provider with which to register
     * @param request the location request containing location parameters
     * @param listener a {@link LocationListener} whose {@link LocationListener#onLocationChanged}
     *                 method will be called for each location update
     * @param executor the executor handling listener callbacks
     */
    public boolean requestLocationUpdates(@NonNull String provider,
            @NonNull LocationRequest request, @NonNull LocationApi.Listener listener,
            @NonNull Executor executor) {
        if (listener == null) {
            return false;
        }

        if (!LocationManagerApi.requestLocationUpdates(provider, request, listener, executor)) {
            return false;
        }

        if (mListenerPool != null) {
            mListenerPool.add(listener);
        }

        return true;
    }

    /**
     * Removes all location updates for the specified IImsLocationListener.
     *
     * @param listener listener object that no longer needs location updates
     */
    public void removeUpdates(LocationApi.Listener listener) {
        if (listener == null) {
            return;
        }

        if (mListenerPool != null) {
            mListenerPool.remove(listener);
        }

        LocationManagerApi.removeUpdates(listener);
    }

    /**
     * Returns a flag indicating if the specified location is obtained by which provider
     *
     * @return true if location is obtained via GPS provider
     */
    public static boolean isLocationFromGps(Location location) {
        return (location != null) && location.getProvider().equals(GPS_PROVIDER);
    }

    /**
     * Returns a flag indicating if the specified location is obtained by which provider
     *
     * @return true if location is obtained via network provider
     */
    public static boolean isLocationFromNetwork(Location location) {
        return (location != null) && location.getProvider().equals(NETWORK_PROVIDER);
    }

    /**
     * Returns a flag indicating if the specified location is obtained by which provider
     *
     * @return true if location is obtained via FLP
     */
    public static boolean isLocationFromFlp(Location location) {
        return (location != null) && location.getProvider().equals(FUSED_PROVIDER);
    }

    private boolean isLocationApiReady() {
        synchronized (mLock) {
            return (mFlag & FLAG_LOCATION_API_READY) != 0;
        }
    }

    private boolean isLocationSchemeSelectedByCurrentUser() {
        synchronized (mLock) {
            return (mFlag & FLAG_SELECT_LOCATION_SCHEME_BY_CURRENT_USER) != 0;
        }
    }

    private static LocationManagerProxy getLocationManagerProxy() {
        return AppContext.getInstance().getSystemServiceProxy(LocationManagerProxy.class);
    }

    private static void log(Object o, String s) {
        ImsLog.d(o, "LocationApi: " + s);
    }

    private static void loge(String s) {
        ImsLog.e(null, "LocationApi: " + s);
    }

    private static void logi(Object o, String s) {
        ImsLog.i(o, "LocationApi: " + s);
    }

    private static class ListenerPool {
        private final Set<LocationApi.Listener> mListeners = new ArraySet<LocationApi.Listener>();

        ListenerPool() {
        }

        public void add(LocationApi.Listener listener) {
            if (listener == null) {
                return;
            }

            synchronized (mListeners) {
                if (!mListeners.contains(listener)) {
                    mListeners.add(listener);
                }
            }
        }

        public void remove(LocationApi.Listener listener) {
            if (listener == null) {
                return;
            }

            synchronized (mListeners) {
                mListeners.remove(listener);

                log(this, "LP - listeners=" + mListeners.size());
            }
        }

        public void clear() {
            synchronized (mListeners) {
                mListeners.clear();
            }
        }

        // Listeners using LocationManagerApi
        public void notifyServiceDisconnectedForLocationManagerApi() {
            LocationApi.Listener[] listeners = toArray();

            if (listeners == null) {
                return;
            }

            clear();

            log(this, "LP - notifyServiceDisconnectedForLocationManagerApi");

            for (LocationApi.Listener listener : listeners) {
                LocationManagerApi.removeUpdates(listener);
            }

            for (LocationApi.Listener listener : listeners) {
                listener.onServiceDisconnected();
            }
        }

        private LocationApi.Listener[] toArray() {
            synchronized (mListeners) {
                if (mListeners.isEmpty()) {
                    return null;
                }

                return mListeners.toArray(new LocationApi.Listener[mListeners.size()]);
            }
        }
    }

    private static class LocationManagerApi {
        public static boolean isProviderEnabled(String provider) {
            LocationManagerProxy lmp = getLocationManagerProxy();

            try {
                return lmp.isProviderEnabled(provider);
            } catch (Exception e) {
                loge(e.toString());
            }

            return false;
        }

        public static Location getLastKnownLocation(String provider) {
            LocationManagerProxy lmp = getLocationManagerProxy();

            try {
                return lmp.getLastKnownLocation(
                        provider, new LastLocationRequest.Builder().build());
            } catch (Exception e) {
                loge(e.toString());
            }

            return null;
        }

        /**
         * Register for location updates from the specified provider, using a
         * {@link LocationRequest}, and a callback on the specified {@link Executor}.
         *
         * @param provider the name of the provider with which to register
         * @param request the location request containing location parameters
         * @param listener a {@link LocationListener} whose
         *                 {@link LocationListener#onLocationChanged} method will be called
         *                 for each location update
         * @param executor the executor handling listener callbacks
         */
        public static boolean requestLocationUpdates(@NonNull String provider,
                @NonNull LocationRequest request, @NonNull LocationListener listener,
                @NonNull Executor executor) {
            if (listener == null) {
                return false;
            }

            LocationManagerProxy lmp = getLocationManagerProxy();

            try {
                logi(null, "LM: requestLocationUpdates - " + request);
                lmp.requestLocationUpdates(provider, request, executor, listener);
                return true;
            } catch (SecurityException e) {
                loge("Location update request is failed; ignore - " + e);
            } catch (IllegalArgumentException e) {
                loge("Provider does not exist; " + e.getMessage());
            } catch (Exception e) {
                loge(e.toString());
            }

            return false;
        }

        public static void removeUpdates(LocationListener listener) {
            LocationManagerProxy lmp = getLocationManagerProxy();

            try {
                lmp.removeUpdates(listener);
            } catch (Exception e) {
                loge(e.toString());
            }
        }
    }
}
