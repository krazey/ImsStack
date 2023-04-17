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
package com.android.imsstack.imsservice.mmtel;

import android.content.Context;
import android.location.Location;
import android.location.LocationRequest;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.IAlarmTimer;
import com.android.imsstack.core.agents.LocationApi;
import com.android.imsstack.core.agents.LocationInterface;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.util.GeocoderProxy;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.SystemUtils;

/**
 * A class that handles location for emergency call.
 */
public class ImsLocationHelper {
    public static interface Listener {
        public void onLocationUpdateTimeout();
        public void onLocationUpdated();
    }

    private static final int EVENT_LOCATION_UPDATES_WAITING_TIMER_EXPIRED = 101;
    private static final int EVENT_LOCATION_UPDATES_COMPLETED = 102;

    private ICallContext mCallContext;
    private LocationApi mLocationApi = LocationApi.getInstance();
    private Listener mListener;
    private LocationHandler mHandler;
    private int mWaitingTimer = 0;
    private boolean mPositionInfoRequired = false;
    private long mValidityPeriod = 0;
    private boolean mLocationUpdateStarted = false;

    private LocationListenerProxy[] mLocationListeners = new LocationListenerProxy[] {
            new LocationListenerProxy(LocationApi.GPS_PROVIDER),
            new LocationListenerProxy(LocationApi.NETWORK_PROVIDER)
    };

    public ImsLocationHelper(ICallContext callContext, Listener listener, Looper looper) {
        mCallContext = callContext;
        mListener = listener;
        mHandler = new LocationHandler(looper);
    }

    public static String getCountry(Context context, Location location) {
        return GeocoderProxy.getCountryFromLocation(context, location);
    }

    public void dispose() {
        stopLocationUpdates();
        mListener = null;
        mLocationUpdateStarted = false;
    }

    public void setValidityOption(boolean positionInfoRequired, long validityPeriod) {
        mPositionInfoRequired = positionInfoRequired;
        mValidityPeriod = validityPeriod;
    }

    public Location getCurrentLocation() {
        // Go in best to worst order
        for (int i = 0; i < mLocationListeners.length; i++) {
            Location l = mLocationListeners[i].current();

            if (l != null) {
                if (!checkValidity(l)) {
                    continue;
                }
                return l;
            }
        }

        log("No location received yet.");

        return null;
    }

    public Location getLastKnownLocationInfo() {
        // Go in best to worst order
        for (int i = 0; i < mLocationListeners.length; i++) {
            Location l = mLocationListeners[i].getLastKnownLocationInfo();

            if (l != null) {
                if (!checkValidity(l)) {
                    continue;
                }
                return l;
            }
        }

        log("No last known location information.");

        return null;
    }

    public Location getCachedLocation() {
        // Go in best to worst order
        Location[] locations = getCachedLocations();

        if (locations != null) {
            for (int i = 0; i < locations.length; i++) {
                Location l = locations[i];

                if (l != null) {
                    if (!checkValidity(l)) {
                        continue;
                    }
                    return l;
                }
            }
        }

        log("No proper cached location information.");

        return null;
    }

    public void startLocationUpdates(long waitingTime) {
        mLocationUpdateStarted = true;

        final LocationRequest requestGps = new LocationRequest.Builder(500)
                .setMinUpdateIntervalMillis(500)
                .setQuality(LocationRequest.QUALITY_HIGH_ACCURACY)
                .build();

        if (!mLocationApi.requestLocationUpdates(
                LocationApi.GPS_PROVIDER,
                requestGps,
                mLocationListeners[0],
                mHandler::post)) {
            log("Location(gps) update request is failed");
        }

        final LocationRequest requestNetwork = new LocationRequest.Builder(500)
                .setMinUpdateIntervalMillis(500)
                .setQuality(LocationRequest.QUALITY_HIGH_ACCURACY)
                .build();

        if (!mLocationApi.requestLocationUpdates(
                LocationApi.NETWORK_PROVIDER,
                requestNetwork,
                mLocationListeners[1],
                mHandler::post))
        {
            log("Location(network) update request is failed");
        }

        if (waitingTime > 0) {
            stopWaitingTimer();
            startWaitingTimer(waitingTime);
        }

        log("startLocationUpdates");
    }

    public void stopLocationUpdates() {
        if (mLocationUpdateStarted) {
            for (int i = 0; i < mLocationListeners.length; i++) {
                mLocationApi.removeUpdates(mLocationListeners[i]);
            }

            log("stopLocationUpdates");
        }

        mLocationUpdateStarted = false;
        stopWaitingTimer();
    }

    private boolean startWaitingTimer(long interval) {
        IAlarmTimer alarmTimer = (IAlarmTimer)AgentFactory.getAgent(AgentFactory.ALARM_TIMER);

        if (alarmTimer == null) {
            return false;
        }

        int timerId = alarmTimer.getTimerId();

        if (timerId <= 0) {
            return false;
        }

        if (!alarmTimer.startTimer(timerId, interval)) {
            log("WaitingTimer :: start failed");
            return false;
        }

        log("WaitingTimer :: started");

        mWaitingTimer = timerId;
        alarmTimer.registerForTimerExpired(timerId, mHandler,
                EVENT_LOCATION_UPDATES_WAITING_TIMER_EXPIRED, null);

        return true;
    }

    private void stopWaitingTimer() {
        if (mWaitingTimer == 0) {
            return;
        }

        IAlarmTimer alarmTimer = (IAlarmTimer)AgentFactory.getAgent(AgentFactory.ALARM_TIMER);

        if (alarmTimer != null) {
            log("WaitingTimer :: stopped");
            alarmTimer.stopTimer(mWaitingTimer);
            alarmTimer.unregisterForTimerExpired(mWaitingTimer, mHandler);
        }

        mWaitingTimer = 0;
    }

    private boolean checkValidity(Location l) {
        if (mPositionInfoRequired
                && (Double.compare(l.getLatitude(), 0.0) == 0)
                && (Double.compare(l.getLongitude(), 0.0) == 0)) {
            // Ignore this location
            log(l.getProvider() + " :: Lat/Lon is invalid; lat="
                    + l.getLatitude() + ", lon=" + l.getLongitude());
            return false;
        }

        if (mValidityPeriod > 0) {
            long locationUpdateTime = l.getElapsedRealtimeNanos();
            long currentTime = SystemClock.elapsedRealtimeNanos();
            long timeLag = (currentTime - locationUpdateTime);

            if (ImsLog.isDebuggable()) {
                logi("Location information :: timeLag=" + timeLag
                        + "; " + locationUpdateTime
                        + "(" + SystemUtils.getUtcTimeFormat(locationUpdateTime / 1000000) + ")"
                        + " >> " + currentTime
                        + "(" + SystemUtils.getUtcTimeFormat(currentTime / 1000000) + ")");
            } else {
                logi("Location information :: timeLag=" + timeLag
                        + "; " + locationUpdateTime + " >> " + currentTime);
            }

            if ((timeLag > 0) && (timeLag > mValidityPeriod)) {
                log(l.getProvider() + " :: Location information is expired; validityPeriod="
                        + mValidityPeriod);
                return false;
            }
        }

        return true;
    }

    private void checkAndNotifyLocationUpdatesCompleted() {
        boolean updateAllDone = true;

        for (int i = 0; i < mLocationListeners.length; i++) {
            if (!mLocationListeners[i].isUpdateDone()) {
                updateAllDone = false;
            }
        }

        if (updateAllDone) {
            stopWaitingTimer();

            if (mHandler != null) {
                Message.obtain(mHandler, EVENT_LOCATION_UPDATES_COMPLETED).sendToTarget();
            }
        }
    }

    private Location[] getCachedLocations() {
        LocationInterface location = mCallContext.getLocationInterface();
        return (location != null) ? location.getCachedLocations() : null;
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    private class LocationHandler extends Handler {
        public LocationHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case EVENT_LOCATION_UPDATES_WAITING_TIMER_EXPIRED:
                stopWaitingTimer();

                if (mListener != null) {
                    mListener.onLocationUpdateTimeout();
                }
                break;

            case EVENT_LOCATION_UPDATES_COMPLETED:
                if (mListener != null) {
                    mListener.onLocationUpdated();
                }
                break;

            default:
                break;
            }
        }
    }

    class LocationListenerProxy implements LocationApi.Listener {
        Location mLastLocation;
        boolean mValid = false;
        boolean mUpdateDone = false;
        String mProvider;

        public LocationListenerProxy(String provider) {
            mProvider = provider;
            mLastLocation = new Location(mProvider);
        }

        @Override
        public void onLocationChanged(Location newLocation) {
            if (!mValid) {
                log(mProvider + " :: Got first location.");

                if (ImsLog.isDebuggable()) {
                    log("Location=" + newLocation);
                }
            }

            mLastLocation.set(newLocation);
            mValid = true;
            mUpdateDone = true;

            checkAndNotifyLocationUpdatesCompleted();
        }

        @Override
        public void onProviderEnabled(String provider) {
            // no-op
        }

        @Override
        public void onProviderDisabled(String provider) {
            mValid = false;
            mUpdateDone = true;

            checkAndNotifyLocationUpdatesCompleted();
        }

        @Override
        public void onServiceDisconnected() {
            log("onServiceDisconnected");

            mValid = false;
            mUpdateDone = true;

            checkAndNotifyLocationUpdatesCompleted();
        }

        public Location current() {
            return mValid ? mLastLocation : null;
        }

        public Location getLastKnownLocationInfo() {
            return mLocationApi.getLastKnownLocation(mProvider);
        }

        public boolean isUpdateDone() {
            return mUpdateDone;
        }
    }
}
