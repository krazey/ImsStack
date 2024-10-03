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

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.TriggerEvent;
import android.hardware.TriggerEventListener;
import android.location.Address;
import android.location.Location;
import android.location.LocationRequest;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemClock;
import android.telephony.ServiceState;
import android.text.TextUtils;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.SensorManagerProxy;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.ServiceCaps;
import com.android.imsstack.util.GeocoderProxy;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.MessageExecutor;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * A class for providing the location information to report the device's location to the network.
 * This information will be used to form PIDF-LO XML body.
 */
public class LocationAgent implements LocationInterface {
    enum ETimerType {
        TIMER_ASYNC_START,
        TIMER_LOCATION_UPDATE_INTERVAL,
        TIMER_SEARCH_DURATION
    }

    enum EReqState {
        STATE_IDLE,
        STATE_ACTIVE
    }

    private final Object mLock = new Object();
    private final int mSlotId;
    private final LocationApi mLocationApi;
    private final Handler mHandler;
    private final GeocoderProxy mGeocoderProxy;
    private final AddressResolver mAddressResolver;
    private final LocationPolicy mPolicy = new LocationPolicy();
    /** Cached locations */
    private final Location[] mLocations = new Location[MAX_LOCATIONS];

    // NOTICE !!!
    // LOCATION UPDATE has two states, IDLE and ACTIVE.
    // requestLocationUpdates() is allowed only if the state is IDLE.
    // The transition from ACTIVE to IDLE happens only when UpdateInterval timer expires.
    // THIS is to guarantee requestLocationUpdates() can be invoked only once within
    // LocationPolicy#LOCATION_UPDATE_INTERVAL, normally 10 minutes.
    private EReqState mLocationRequestState = EReqState.STATE_IDLE;

    private boolean mLocationInfoStarted = false;
    private boolean mInstantLocationInfoRequested = false;

    private Location mNetworkLocation = null;
    private Location mGpsLocation = null;
    private Location mFusedLocation = null;
    private boolean mIsAleadyUpdatedFromNetwork = false;
    private Address mResolvedAddress = null;

    private long mTimerIdAsyncStart = TimerInterface.INVALID_TID;
    private long mTimerIdUpdateInterval = TimerInterface.INVALID_TID;
    private long mTimerIdSearchDuration = TimerInterface.INVALID_TID;

    // default update interval is 10 mins
    private int mUpdateInterval = LocationPolicy.LOCATION_UPDATE_INTERVAL;
    private boolean mGpsLocationRequested = false;

    // For SMD (Significant Motion Detection)
    private boolean mIsSmdRequested = false;
    private boolean mIsLocationUpdatedAfterMotionDetected = false;
    private long mCachedTimeMotionDetected = 0L;
    private String mLastKnownCountryCode = GeocoderProxy.UNKNOWN_COUNTRY;

    private LocationApi.Listener mLocationListener = new LocationApi.Listener () {
        public void onLocationChanged(Location location) {
            ImsLog.d(this, mSlotId, "Provider: "
                    + (location == null ? "null" : location.getProvider()));

            // To prevent updating from network provider repeatedly
            if (LocationApi.isLocationFromNetwork(location)) {
                if (mIsAleadyUpdatedFromNetwork == true) {
                    ImsLog.d(this, mSlotId, "Already update from network provider");
                    return;
                } else {
                    mIsAleadyUpdatedFromNetwork = true;
                }
            }

            // NOTICE !!!
            // if requestLocationUpdates() is invoked, Modem is awake consuming battery.
            // So, whenever location information is updated, removeUpdates() must be invoked.
            if (mGpsLocationRequested
                    && mLocationApi.isProviderEnabled(LocationApi.GPS_PROVIDER)) {
                if (LocationApi.isLocationFromGps(location)) {
                    ImsLog.d(this, mSlotId, "gps location is updated");
                    removeLocationUpdates();

                    if (location != null) {
                        requestSmd();
                    }
                } else {
                    ImsLog.d(this, mSlotId, "Wait for an update by gps");

                    if (mPolicy.hasPolicy(LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD)) {
                        if (location != null) {
                            mIsLocationUpdatedAfterMotionDetected = true;
                        }
                    }
                }
            } else {
                ImsLog.d(this, mSlotId, "No waiting for gps");
                removeLocationUpdates();

                if (location != null) {
                    requestSmd();
                }
            }

            // Called when a new location is found.
            updateLocation(location);

            if (mPolicy.hasPolicy(
                LocationPolicy.POLICY_NOTIFY_LOCATION_FIXED_FOR_INSTANT_REQUEST)) {
                notifyEventOnLocationFixedForInstantRequest();
            }
        }

        public void onServiceDisconnected() {
            ImsLog.d(this, mSlotId, "onServiceDisconnected");
            removeLocationUpdates();
        }
    };

    private final TriggerEventListener mSmdListener = new TriggerEventListener() {
        @Override
        public void onTrigger(TriggerEvent event) {
            ImsLog.d(this, mSlotId, "onTrigger");

            if (!mIsSmdRequested) {
                ImsLog.d(this, mSlotId, "Ignore, because SMD is not requested");
                return;
            }

            mIsSmdRequested = false;

            if (mPolicy.hasPolicy(LocationPolicy.POLICY_LOCATION_NOT_ALLOWED_PERIODIC_POLLING)) {
                ImsLog.d(this, mSlotId, "Ignore, because periodic location polling is not allowed");
            } else {
                startTimer(ETimerType.TIMER_LOCATION_UPDATE_INTERVAL, (getUpdateInterval()*1000));
            }

            mLocationRequestState = EReqState.STATE_ACTIVE;

            mIsLocationUpdatedAfterMotionDetected = false;

            // Cache current time
            mCachedTimeMotionDetected = System.currentTimeMillis();
        }
    };

    private final TimerInterface.Listener mTimerListener = new TimerInterface.Listener() {
        @Override
        public void onTimerExpired(long tid) {
            mHandler.post(() -> {
                if (tid == mTimerIdAsyncStart) {
                    stopTimer(ETimerType.TIMER_ASYNC_START);

                    if (mLocationInfoStarted || mInstantLocationInfoRequested) {
                        if (mInstantLocationInfoRequested) {
                            mLocationRequestState = EReqState.STATE_IDLE;
                        }

                        requestLocationUpdates();
                    }
                } else if (tid == mTimerIdUpdateInterval) {
                    stopTimer(ETimerType.TIMER_LOCATION_UPDATE_INTERVAL);

                    mLocationRequestState = EReqState.STATE_IDLE;

                    if (mLocationInfoStarted) {
                        requestLocationUpdates();
                    } else {
                        ImsLog.d(this, mSlotId, "Location is not required - stop location update.");
                    }
                } else if (tid == mTimerIdSearchDuration) {
                    stopTimer(ETimerType.TIMER_SEARCH_DURATION);

                    removeLocationUpdates();

                    // Only if location is updated after motion detection, request SMD
                    if (mIsLocationUpdatedAfterMotionDetected) {
                        requestSmd();
                    }
                }
            });
        }
    };

    private final ConfigInterface.Listener mConfigListener = new ConfigInterface.Listener() {
        @Override
        public void onCarrierConfigChanged(int slotId, int subId) {
            ImsLog.d(this, mSlotId, "onCarrierConfigChanged: subId=" + subId);
            initPolicy();
        }
    };

    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();

    public LocationAgent(int slotId) {
        ImsLog.d(this, slotId, "LocationAgent" + slotId);

        mSlotId = slotId;
        mLocationApi = LocationApi.getInstance();
        mHandler = new Handler(AppContext.getInstance().getMainLooper());
        mGeocoderProxy = new GeocoderProxy(AppContext.getInstance(), mHandler);
        // Instantiate AddressResolver to update location details
        // when location is fixed by LocationManager.
        mAddressResolver = new AddressResolver();
        mLocations[CACHE_I_GPS] = new Location(LocationApi.GPS_PROVIDER);
        mLocations[CACHE_I_NLP] = new Location(LocationApi.NETWORK_PROVIDER);
        mLocations[CACHE_I_FLP] = new Location(LocationApi.FUSED_PROVIDER);
    }

    @Override
    public void init(Context context) {
        ConfigInterface config = getConfigInterface();
        if (config != null) {
            config.addListener(mConfigListener);
        }

        initPolicy();
        mLocationApi.start();
    }

    @Override
    public void cleanup() {
        mLocationApi.stop();
        stopListeningForLocation();
        removeLocationUpdates();
        mHandler.removeCallbacksAndMessages(null);

        stopTimer(ETimerType.TIMER_SEARCH_DURATION);
        stopTimer(ETimerType.TIMER_ASYNC_START);
        stopTimer(ETimerType.TIMER_LOCATION_UPDATE_INTERVAL);

        ConfigInterface config = getConfigInterface();
        if (config != null) {
            config.removeListener(mConfigListener);
        }

        mLocationRequestState = EReqState.STATE_IDLE;
        mGpsLocationRequested = false;
        mNetworkLocation = null;
        mGpsLocation = null;
        mFusedLocation = null;
        mResolvedAddress = null;
        mUpdateInterval = LocationPolicy.LOCATION_UPDATE_INTERVAL;
        mLastKnownCountryCode = GeocoderProxy.UNKNOWN_COUNTRY;
    }

    @Override
    public void initLastKnownLocation() {
        getLastKnownLocation(LOCATION_CATEGORY_ALL);
    }

    @Override
    public LocationPolicy getLocationPolicy() {
        synchronized (mLock) {
            return mPolicy;
        }
    }

    @Override
    public void setLocationPolicy(LocationPolicy lp) {
        synchronized (mLock) {
            if (lp != null) {
                mPolicy.set(lp);

                if (mPolicy.hasPolicy(LocationPolicy.POLICY_USE_FIXED_LOCATION_UPDATE_INTERVAL)) {
                    mUpdateInterval = mPolicy.getFixedUpdateInterval();
                }
            } else {
                mPolicy.reset();
            }
        }
    }

    @Override
    public Location[] getCachedLocations() {
        synchronized (mLock) {
            return Arrays.copyOf(mLocations, mLocations.length);
        }
    }

    @Override
    public String getLastKnownCountryCode() {
        synchronized (mLock) {
            return mLastKnownCountryCode;
        }
    }

    @Override
    public String[] getLastKnownLocation(@LocationCategory int category) {
        try {
            initLocationIfRequired();

            StringBuilder log = new StringBuilder("Last known location ::");

            Location locationFromFlp = null;
            if (mLocationApi.isProviderEnabled(LocationApi.FUSED_PROVIDER)
                    && mPolicy.hasPolicy(LocationPolicy.POLICY_USE_FLP)) {
                log.append(" FLP enabled");

                locationFromFlp = mLocationApi.getLastKnownLocation(LocationApi.FUSED_PROVIDER);
                updateFusedLocation(locationFromFlp);

                if (locationFromFlp == null) {
                    log.append("(null)");
                }
            }

            if (locationFromFlp == null) {
                if (mLocationApi.isProviderEnabled(LocationApi.GPS_PROVIDER)) {
                    log.append(" GPS enabled");

                    Location location = mLocationApi.getLastKnownLocation(LocationApi.GPS_PROVIDER);
                    updateGpsLocation(location);

                    if (location == null) {
                        log.append("(null)");
                    }
                }

                if (mLocationApi.isProviderEnabled(LocationApi.NETWORK_PROVIDER)) {
                    log.append(" NLP enabled");

                    Location location = mLocationApi.getLastKnownLocation(
                            LocationApi.NETWORK_PROVIDER);
                    updateNetworkLocation(location);

                    if (location == null) {
                        log.append("(null)");
                    }
                }
            }

            ImsLog.w(this, mSlotId, log.toString());

            if (mIsSmdRequested) {
                // If motion is not detect yet, update current time to cache time
                mCachedTimeMotionDetected = System.currentTimeMillis();
            }

            Location location = getBestLocation();
            String[] locationInfo = parseLocationInfo(location, category);

            if (locationInfo == null || locationInfo[7] == null
                    || "".equals(locationInfo[7].trim())) {
                String cc = getCountryCodeViaOtherScheme();

                if (cc != null) {
                    if (locationInfo == null) {
                        locationInfo = new String[13];
                    }
                    locationInfo[7] = cc;
                }
            }

            return locationInfo;
        } catch (Exception e) {
            ImsLog.d(this, mSlotId, "getLastKnownLocation: " + e.toString());
            return null;
        }
    }

    @Override
    public void startListeningForLocation(int updateIntervalSec) {
        synchronized (mLock) {
            if (updateIntervalSec < mPolicy.getDefaultUpdateInterval()) {
                ImsLog.w(this, mSlotId, "Update interval is very short. Use a default interval.");
                updateIntervalSec = mPolicy.getDefaultUpdateInterval();
            }

            ImsLog.d(this, mSlotId, "startListeningForLocation: interval(s)=" + updateIntervalSec);

            if (mPolicy.hasPolicy(LocationPolicy.POLICY_USE_FIXED_LOCATION_UPDATE_INTERVAL)) {
                mUpdateInterval = mPolicy.getFixedUpdateInterval();
                ImsLog.d(this, mSlotId, "Forced to " + mUpdateInterval);
            } else {
                mUpdateInterval = updateIntervalSec;
            }

            if (mLocationInfoStarted) {
                ImsLog.d(this, mSlotId, "Location listening is already started.");
                return;
            }

            mLocationInfoStarted = true;

            startTimer(ETimerType.TIMER_ASYNC_START, 100);
        }
    }

    @Override
    public void stopListeningForLocation() {
        synchronized (mLock) {
            ImsLog.d(this, mSlotId, "stopListeningForLocation");

            mLocationInfoStarted = false;

            cancelSmd();

            mIsLocationUpdatedAfterMotionDetected = false;
            mCachedTimeMotionDetected = 0L;
        }
    }

    @Override
    public void startInstantLocationUpdate() {
        synchronized (mLock) {
            if (!mInstantLocationInfoRequested) {
                mInstantLocationInfoRequested = true;
                startTimer(ETimerType.TIMER_ASYNC_START, 100);
            }
        }
    }

    @Override
    public void addListener(Listener listener) {
        mListeners.add(listener);
    }

    @Override
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    private boolean requestLocationUpdates() {
        if (mLocationRequestState == EReqState.STATE_ACTIVE) {
            ImsLog.d(this, mSlotId, "Location update is already running");
            return false;
        }

        cancelSmd();

        mGpsLocationRequested = false;

        if (isGpsLocationUpdatedRecently()) {
            processLocationUpdateDone();
            return true;
        }

        boolean isGpsProviderEnabled
                = mLocationApi.isProviderEnabled(LocationApi.GPS_PROVIDER);
        boolean isNetworkProviderEnabled
                = mLocationApi.isProviderEnabled(LocationApi.NETWORK_PROVIDER);
        boolean isFusedProviderEnabled
                = mLocationApi.isProviderEnabled(LocationApi.FUSED_PROVIDER);

        ImsLog.d(this, mSlotId, "LocationProvider: gps=" + isGpsProviderEnabled
                + ", network=" + isNetworkProviderEnabled
                + ", fused=" + isFusedProviderEnabled);

        int searchTime = 0;

        if (isFusedProviderEnabled && mPolicy.hasPolicy(LocationPolicy.POLICY_USE_FLP)) {
            final LocationRequest request = new LocationRequest.Builder(0)
                    .setMinUpdateIntervalMillis(0)
                    .setQuality(LocationRequest.QUALITY_HIGH_ACCURACY)
                    .setLocationSettingsIgnored(true)
                    .build();
            mLocationApi.requestLocationUpdates(LocationApi.FUSED_PROVIDER,
                    request, mLocationListener, mHandler::post);

            searchTime = mPolicy.getSearchDurationForFlp();
        } else {
            if (isGpsProviderEnabled) {
                final LocationRequest request = new LocationRequest.Builder(0)
                        .setMinUpdateIntervalMillis(0)
                        .setQuality(LocationRequest.QUALITY_HIGH_ACCURACY)
                        .build();
                mLocationApi.requestLocationUpdates(LocationApi.GPS_PROVIDER,
                        request, mLocationListener, mHandler::post);
                mGpsLocationRequested = true;
            }

            if (isNetworkProviderEnabled) {
                final LocationRequest request = new LocationRequest.Builder(0)
                        .setMinUpdateIntervalMillis(0)
                        .setQuality(LocationRequest.QUALITY_BALANCED_POWER_ACCURACY)
                        .build();
                mLocationApi.requestLocationUpdates(LocationApi.NETWORK_PROVIDER,
                        request, mLocationListener, mHandler::post);
            }

            searchTime = Math.max(
                    isGpsProviderEnabled ? mPolicy.getSearchDurationForGps() : 0,
                    isNetworkProviderEnabled ? mPolicy.getSearchDurationForNlp() : 0);
        }

        if (searchTime <= 0) {
            return false;
        }

        stopTimer(ETimerType.TIMER_SEARCH_DURATION);
        startTimer(ETimerType.TIMER_SEARCH_DURATION, searchTime * 1000);

        if (mPolicy.hasPolicy(LocationPolicy.POLICY_LOCATION_NOT_ALLOWED_PERIODIC_POLLING)) {
            ImsLog.d(this, mSlotId, "Blocked: periodic location polling is not allowed");
        } else {
            stopTimer(ETimerType.TIMER_LOCATION_UPDATE_INTERVAL);
            startTimer(ETimerType.TIMER_LOCATION_UPDATE_INTERVAL,
                    (getUpdateInterval() * 1000) + (searchTime * 1000));
        }

        mLocationRequestState = EReqState.STATE_ACTIVE;

        return true;
    }

    private void removeLocationUpdates() {
        ImsLog.d(this, mSlotId, "removeLocationUpdates");

        mLocationApi.removeUpdates(mLocationListener);

        mIsAleadyUpdatedFromNetwork = false;

        mInstantLocationInfoRequested = false;

        stopTimer(ETimerType.TIMER_SEARCH_DURATION);
    }

    private Location findLatestLocation(Location... locations) {
        Location latestLocation = null;
        StringBuilder log = new StringBuilder("Elapsed time:");

        for (Location location : locations) {
            if (location == null) {
                continue;
            }

            log.append(" ")
                    .append(location.getProvider())
                    .append("=")
                    .append(location.getElapsedRealtimeNanos());

            if (latestLocation == null ||
                    location.getElapsedRealtimeNanos()
                        > latestLocation.getElapsedRealtimeNanos()) {
                latestLocation = location;
            }
        }

        log.append(" >> ")
                .append(latestLocation == null ? "null" : latestLocation.getProvider());
        ImsLog.d(this, mSlotId, log.toString());

        return latestLocation;
    }

    private String[] parseLocationInfo(Location location, @LocationCategory int category) {
        if (location == null) {
            ImsLog.w(this, mSlotId, "Location is null");
            return null;
        }

        // confidence is defined as 68
        int confidence = 68;

        String method = "";

        if (LocationApi.isLocationFromGps(location)) {
            method = "GPS";
        } else if (LocationApi.isLocationFromNetwork(location)) {
            method = "Cell";

            WifiInterface wifi = AgentFactory.getInstance().getAgent(WifiInterface.class);

            if (wifi != null) {
                IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
                if ((dnw != null)
                        && (dnw.getDataServiceState() != ServiceState.STATE_IN_SERVICE)
                        && wifi.isWifiConnected()) {
                    method = "802.11";
                }
            }
        } else if (LocationApi.isLocationFromFlp(location)) {
            method = "Hybrid_A-GPS";
        }

        ImsLog.d(this, mSlotId, method + ": accuracy=" + location.getAccuracy()
                + ", category=" + category);

        if (TextUtils.isEmpty(method)) {
            return null;
        }

        Address address = null;

        if (category != LOCATION_CATEGORY_POSITION) {
            // Use network country first
            if (category == LOCATION_CATEGORY_POSITION_N_COUNTRY) {
                String networkCountry = getCountryCodeFromNetwork();
                if (networkCountry != null) {
                    address = new Address(Locale.US);
                    address.setCountryCode(networkCountry);
                }
            }

            if (address == null) {
                if (mPolicy.hasPolicy(LocationPolicy.POLICY_USE_CACHED_ADDRESS)) {
                    address = getCachedAddress(location);

                    if (address == null) {
                        if (mPolicy.hasPolicy(LocationPolicy.POLICY_USE_ONLY_CACHED_ADDRESS)
                                && mPolicy.hasPolicy(LocationPolicy
                                        .POLICY_UPDATE_ADDRESS_AFTER_LOCATION_ACQUIRED)) {
                            // Do not get address by policy
                        } else {
                            address = translateLocationIntoAddress(location);

                            if (address != null) {
                                address.setLatitude(location.getLatitude());
                                address.setLongitude(location.getLongitude());
                            }

                            cacheAddress(address);
                        }
                    }
                } else {
                    address = translateLocationIntoAddress(location);
                }
            }
        }

        if (address == null) {
            if ((Double.compare(location.getLatitude(), 0.0) == 0)
                    || (Double.compare(location.getLongitude(), 0.0) == 0)) {
                ImsLog.d(this, mSlotId, "Address is null and no location information");
                return null;
            }
        }

        // Latitude / Longitude / Radius / Shape / Confidence / Timestamp / Method /
        // Altitude / Vertical Accuracy
        String[] locationInfo = new String[13];

        locationInfo[0] = Double.toString(location.getLatitude());
        locationInfo[1] = Double.toString(location.getLongitude());
        locationInfo[2] = Float.toString(location.getAccuracy());
        locationInfo[3] = mPolicy.getShape();
        locationInfo[4] = Integer.toString(confidence);
        locationInfo[5] = ImsUtils.getUtcTimeFormat(location.getTime());
        locationInfo[6] = method;
        locationInfo[11] = Double.toString(location.getAltitude());
        locationInfo[12] = Float.toString(location.getVerticalAccuracyMeters());

        // Country code / State / City / Postal
        if (address == null) {
            ImsLog.d(this, mSlotId, "Address is null");
            locationInfo[7] = getCountryCodeViaOtherScheme();
        } else {
            locationInfo[7] = address.getCountryCode();
            locationInfo[8] = address.getAdminArea();
            locationInfo[9] = address.getLocality();
            locationInfo[10] = address.getPostalCode();

            if (!location.isMock()) {
                setLastKnownCountryCode(address.getCountryCode());
            }
        }

        ImsLog.d(this, mSlotId, fromLocationInfo(locationInfo));

        return locationInfo;
    }

    private Address translateLocationIntoAddress(Location l) {
        if (l == null) {
            return null;
        }

        List<Address> addresses = null;

        try {
            mGeocoderProxy.setLatAndLong(l.getLatitude(), l.getLongitude());

            int resolutionTime = mPolicy.getDefaultAddressResolutionTime();

            ImsLog.w(this, mSlotId, "Waits for address resolution for " + resolutionTime + "ms");

            mGeocoderProxy.findAddressFromLocation(resolutionTime);

            addresses = mGeocoderProxy.getAddresses();
        } catch (Exception e) {
            ImsLog.d(this, mSlotId, "Failed while trying to get address from GeoCode");
        }

        if (addresses == null || addresses.size() == 0) {
            return null;
        }

        return addresses.get(0);
    }

    private Address getCachedAddress(Location l) {
        if (mPolicy.hasPolicy(LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE)) {
            if (isValidAddressPerDistance(l)) {
                return mResolvedAddress;
            }
        }

        if (mPolicy.hasPolicy(LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_TIME)) {
            if (isValidAddressPerTime()) {
                return mResolvedAddress;
            }
        }

        return null;
    }

    private boolean isValidAddressPerDistance(Location l) {
        if (l == null) {
            return false;
        }

        if (mResolvedAddress == null) {
            return false;
        }

        float distance[] = {0.0f, 0.0f};

        try {
            Location.distanceBetween(l.getLatitude(), l.getLongitude(),
                mResolvedAddress.getLatitude(), mResolvedAddress.getLongitude(),
                distance);
        } catch (IllegalArgumentException iae) {
            return false;
        }

        if (mPolicy.getAddressTolerableDistance() > distance[0]) {
            ImsLog.d(this, mSlotId, "Cached address is valid considering distance");
            return true;
        }

        return false;
    }

    private boolean isValidAddressPerTime() {
        if (mResolvedAddress == null) {
            return false;
        }

        long validityPeriod = mPolicy.getAddressValidityPeriod();

        if (validityPeriod > 0L) {
            Bundle extras  = mResolvedAddress.getExtras();
            long cachedTime = extras.getLong("cachedTime");
            long currentTime = SystemClock.elapsedRealtimeNanos();
            long timeLag = (currentTime - cachedTime);

            if (ImsLog.isDebuggable()) {
                ImsLog.d(this, mSlotId, "Location: timeLag=" + timeLag
                        + "; " + cachedTime
                        + "(" + ImsUtils.getUtcTimeFormat(cachedTime / 1000000) + ")"
                        + " >> " + currentTime
                        + "(" + ImsUtils.getUtcTimeFormat(currentTime / 1000000) + ")");
            } else {
                ImsLog.d(this, mSlotId, "Location: timeLag=" + timeLag
                        + "; " + cachedTime + " >> " + currentTime);
            }

            if ((timeLag > 0) && (timeLag <= validityPeriod)) {
                ImsLog.d(this, mSlotId, "Cached address is valid considering time");
                return true;
            }
        }

        return false;
    }

    private void cacheAddress(Address address) {
        if (address != null) {
            mResolvedAddress = address;

            Bundle extras = new Bundle();
            long cachedTime = SystemClock.elapsedRealtimeNanos();
            extras.putLong("cachedTime", cachedTime);
            mResolvedAddress.setExtras(extras);
        }
    }

    private void updateLocation(Location location) {
        if (LocationApi.isLocationFromGps(location)) {
            updateGpsLocation(location);
        } else if (LocationApi.isLocationFromNetwork(location)) {
            updateNetworkLocation(location);
        } else if (LocationApi.isLocationFromFlp(location)) {
            updateFusedLocation(location);
        } else {
            return;
        }

        // Update cached data from the recent location when location is fixed.
        if (mPolicy.hasPolicy(LocationPolicy.POLICY_UPDATE_ADDRESS_AFTER_LOCATION_ACQUIRED)) {
            Location betterLocation = null;

            synchronized (mLock) {
                betterLocation = findLatestLocation(
                        mFusedLocation, mGpsLocation, mNetworkLocation);
            }

            if (betterLocation != null) {
                mAddressResolver.updateLocationDetails(new Location(betterLocation));
            }
        }
    }

    private boolean isValidLocation(Location l) {
        if (l == null) {
            return false;
        }

        if (mPolicy.getValidityPeriod() <= 0L) {
            return true;
        }

        long locationUpdateTime = l.getElapsedRealtimeNanos();
        long currentTime = SystemClock.elapsedRealtimeNanos();
        long timeLag = (currentTime - locationUpdateTime);

        if (ImsLog.isDebuggable()) {
            ImsLog.d(this, mSlotId, "Location: timeLag=" + timeLag
                    + "; " + locationUpdateTime
                    + "(" + ImsUtils.getUtcTimeFormat(locationUpdateTime / 1000000) + ")"
                    + " >> " + currentTime
                    + "(" + ImsUtils.getUtcTimeFormat(currentTime / 1000000) + ")");
        } else {
            ImsLog.d(this, mSlotId, "Location: timeLag=" + timeLag
                    + "; " + locationUpdateTime + " >> " + currentTime);
        }

        if ((timeLag > 0) && (timeLag > mPolicy.getValidityPeriod())) {
            ImsLog.d(this, mSlotId, l.getProvider() + ": Location is considered as expiration");
            return false;
        }

        return true;
    }

    public int getUpdateInterval() {
        return mUpdateInterval;
    }

    private void startTimer(ETimerType timerType, int duration) {
        if (timerType == ETimerType.TIMER_LOCATION_UPDATE_INTERVAL) {
            if (mTimerIdUpdateInterval != TimerInterface.INVALID_TID) {
                ImsLog.w(this, mSlotId, "LocationUpdate timer already running");
                return;
            }
        } else if (timerType == ETimerType.TIMER_SEARCH_DURATION) {
            if (mTimerIdSearchDuration != TimerInterface.INVALID_TID) {
                ImsLog.w(this, mSlotId, "Search timer already running");
                return;
            }
        } else if (timerType == ETimerType.TIMER_ASYNC_START) {
            if (mTimerIdAsyncStart != TimerInterface.INVALID_TID) {
                ImsLog.w(this, mSlotId, "AsyncStart timer already running");
                return;
            }
        }

        TimerInterface timer = AgentFactory.getInstance().getAgent(TimerInterface.class);

        if (timer == null) {
            return;
        }

        long timerId = timer.startTimer(duration, mTimerListener);

        if (timerId == TimerInterface.INVALID_TID) {
            ImsLog.e(this, mSlotId, "Starting a timer failed.");
            return;
        }

        if (timerType == ETimerType.TIMER_LOCATION_UPDATE_INTERVAL) {
            mTimerIdUpdateInterval = timerId;
        } else if (timerType == ETimerType.TIMER_SEARCH_DURATION) {
            mTimerIdSearchDuration = timerId;
        } else if (timerType == ETimerType.TIMER_ASYNC_START) {
            mTimerIdAsyncStart = timerId;
        }

        ImsLog.i(this, mSlotId, "LocationAgent#startTimer: tid=" + timerId
                + ", duration=" + duration);
    }

    private void stopTimer(ETimerType timerType) {
        long timerId = TimerInterface.INVALID_TID;

        if (timerType == ETimerType.TIMER_LOCATION_UPDATE_INTERVAL) {
            timerId = mTimerIdUpdateInterval;
            mTimerIdUpdateInterval = TimerInterface.INVALID_TID;
        } else if (timerType == ETimerType.TIMER_SEARCH_DURATION) {
            timerId = mTimerIdSearchDuration;
            mTimerIdSearchDuration = TimerInterface.INVALID_TID;
        } else if (timerType == ETimerType.TIMER_ASYNC_START) {
            timerId = mTimerIdAsyncStart;
            mTimerIdAsyncStart = TimerInterface.INVALID_TID;
        }

        if (timerId == TimerInterface.INVALID_TID) {
            return;
        }

        TimerInterface timer = AgentFactory.getInstance().getAgent(TimerInterface.class);
        if (timer != null) {
            timer.stopTimer(timerId);
        }
    }

    private String getCountryCodeViaOtherScheme() {
        if (!mPolicy.hasPolicy(LocationPolicy.POLICY_UPDATE_COUNTRY_VIA_OTHER_SCHEME)) {
            return null;
        }

        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);

        String countryCode =
                (telephony != null) ? telephony.getNetworkCountryIso().toUpperCase(Locale.US) : "";

        // From attached mobile network
        if (!TextUtils.isEmpty(countryCode)
                && !GeocoderProxy.UNKNOWN_COUNTRY.equals(countryCode)) {
            return countryCode;
        } else {
            countryCode = getLastKnownCountryCode();

            // From recent cached country code if present
            if (!TextUtils.isEmpty(countryCode)
                    && !GeocoderProxy.UNKNOWN_COUNTRY.equals(countryCode)) {
                return countryCode;
            }

            // From USIM
            if (mPolicy.hasPolicy(LocationPolicy.POLICY_UPDATE_COUNTRY_FROM_USIM)) {
                countryCode = (telephony != null)
                        ? telephony.getSimCountryIso().toUpperCase(Locale.US) : "";

                if (!TextUtils.isEmpty(countryCode)
                        && !GeocoderProxy.UNKNOWN_COUNTRY.equals(countryCode)) {
                    return countryCode;
                }
            }
        }

        return null;
    }

    private String getCountryCodeFromNetwork() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);

        String countryCode =
                (telephony != null) ? telephony.getNetworkCountryIso().toUpperCase(Locale.US) : "";

        // From attached mobile network
        if (!TextUtils.isEmpty(countryCode)
                && !GeocoderProxy.UNKNOWN_COUNTRY.equals(countryCode)) {
            ImsLog.d(this, mSlotId, "getCountryCodeFromNetwork: " + countryCode);
            return countryCode;
        }

        return null;
    }

    @VisibleForTesting
    protected void notifyEventOnLastKnownCountryChanged(
            String oldCountryCode, String newCountryCode) {
        if (!mPolicy.hasPolicy(LocationPolicy.POLICY_NOTIFY_COUNTRY_CHANGED_EVENT)) {
            return;
        }

        if (!GeocoderProxy.UNKNOWN_COUNTRY.equals(oldCountryCode)
                && !GeocoderProxy.UNKNOWN_COUNTRY.equals(newCountryCode)) {
            for (Listener l : mListeners) {
                l.onLastKnownCountryUpdated();
            }
        }
    }

    @VisibleForTesting
    protected void notifyEventOnLocationFixedForInstantRequest() {
        synchronized (mLock) {
            if (mInstantLocationInfoRequested) {
                mInstantLocationInfoRequested = false;
            } else {
                return;
            }
        }

        for (Listener l : mListeners) {
            l.onInstantRequestedLocationUpdated();
        }
    }

    private void setLastKnownCountryCode(final String countryCode) {
        if (TextUtils.isEmpty(countryCode)) {
            return;
        }

        boolean ccChanged = false;
        String oldCountryCode = GeocoderProxy.UNKNOWN_COUNTRY;

        synchronized (mLock) {
            if (!mLastKnownCountryCode.equals(countryCode)) {
                ImsLog.d(this, mSlotId, "LastKnownCountryCode: "
                        + mLastKnownCountryCode + " >> " + countryCode);

                oldCountryCode = mLastKnownCountryCode;
                mLastKnownCountryCode = countryCode;

                ccChanged = true;
            }
        }

        if (ccChanged) {
            notifyEventOnLastKnownCountryChanged(oldCountryCode, countryCode);
        }
    }

    private Location getBestLocation() {
        Location location = null;

        synchronized (mLock) {
            location = findLatestLocation(mFusedLocation, mGpsLocation, mNetworkLocation);

            if (location != null
                    && mPolicy.hasPolicy(LocationPolicy.POLICY_USE_CACHED_LOCATION)) {

                location = updateCurrentTimeFromCachedTime(location);

                if (!isValidLocation(location)) {
                    location = null;
                }
            }
        }

        if (location == null
                && mPolicy.hasPolicy(LocationPolicy.POLICY_USE_CACHED_LOCATION)
                && (mLocationApi.isProviderEnabled(LocationApi.GPS_PROVIDER)
                        || mLocationApi.isProviderEnabled(LocationApi.NETWORK_PROVIDER)
                        || mLocationApi.isProviderEnabled(LocationApi.FUSED_PROVIDER))) {
            synchronized (mLock) {
                location = findLatestLocation(
                        mLocations[CACHE_I_GPS],
                        mLocations[CACHE_I_NLP],
                        mLocations[CACHE_I_FLP]);
            }

            if (location != null) {
                location = updateCurrentTimeFromCachedTime(location);
            }

            if (!isValidLocation(location)) {
                location = null;
            }

            ImsLog.d(this, mSlotId, "Cached location: "
                    + ((location != null) ? location.getProvider() : "(null)"));
        }

        return location;
    }

    private void initLocationIfRequired() {
        synchronized (mLock) {
            if (mPolicy.hasPolicy(LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION)) {
                mGpsLocation = null;
                mNetworkLocation = null;
                mFusedLocation = null;
            }
        }
    }

    private void updateGpsLocation(Location location) {
        synchronized (mLock) {
            if (!mPolicy.hasPolicy(LocationPolicy.POLICY_ALLOW_MOCK_LOCATION_UPDATE)) {
                if ((location != null) && location.isMock()) {
                    return;
                }
            }

            if (mPolicy.hasPolicy(LocationPolicy.POLICY_ENABLE_CACHED_LOCATION)) {
                if (location != null) {
                    mGpsLocation = location;
                }
            } else {
                mGpsLocation = location;
            }
        }

        updateCachedLocation(location, CACHE_I_GPS);
    }

    private void updateNetworkLocation(Location location) {
        synchronized (mLock) {
            if (!mPolicy.hasPolicy(LocationPolicy.POLICY_ALLOW_MOCK_LOCATION_UPDATE)) {
                if ((location != null) && location.isMock()) {
                    return;
                }
            }

            if (mPolicy.hasPolicy(LocationPolicy.POLICY_ENABLE_CACHED_LOCATION)) {
                if (location != null) {
                    mNetworkLocation = location;
                }
            } else {
                mNetworkLocation = location;
            }
        }

        updateCachedLocation(location, CACHE_I_NLP);
    }

    private void updateFusedLocation(Location location) {
        synchronized (mLock) {
            if (!mPolicy.hasPolicy(LocationPolicy.POLICY_ALLOW_MOCK_LOCATION_UPDATE)) {
                if ((location != null) && location.isMock()) {
                    return;
                }
            }

            if (mPolicy.hasPolicy(LocationPolicy.POLICY_ENABLE_CACHED_LOCATION)) {
                if (location != null) {
                    mFusedLocation = location;
                }
            } else {
                mFusedLocation = location;
            }
        }

        updateCachedLocation(location, CACHE_I_FLP);
    }

    private void updateCachedLocation(Location location, int locationType) {
        if (location == null) {
            return;
        }

        if (location.isMock()) {
            // Do not update MOCK location.
            return;
        }

        if ((Double.compare(location.getLatitude(), 0.0) == 0)
                || (Double.compare(location.getLongitude(), 0.0) == 0)) {
            // If latitude or longitude is not present, we consider it as invalid location.
            return;
        }

        synchronized (mLock) {
            if (locationType < 0 || locationType >= mLocations.length) {
                ImsLog.e(this, mSlotId, "Invalid location type: " + locationType);
                return;
            }

            mLocations[locationType].set(location);
        }
    }

    /** For debugging
    private void displayCachedLocations() {
        displayLocationDetails(mLocations[CACHE_I_GPS], null, "Cache-GPS");
        displayLocationDetails(mLocations[CACHE_I_NLP], null, "Cache-NLP");
        displayLocationDetails(mLocations[CACHE_I_FLP], null, "Cache-FLP");
    }
    */

    private static String displayLocationDetails(Location location,
            Address address, String logTag) {
        StringBuilder sb = new StringBuilder();

        if (logTag != null) {
            sb.append(logTag + " :: ");
        }

        sb.append("Location(lat|lon|accu|conf|ts|method|country|state|city|postal|alt|vaccu)=");
        sb.append("[");

        if (location != null) {
            sb.append(location.getLatitude());
            sb.append(", ");
            sb.append(location.getLongitude());
            sb.append(", ");
            sb.append(location.getAccuracy());
            sb.append(", ");
            sb.append("68, ");
            sb.append(ImsUtils.getUtcTimeFormat(location.getTime()));
            sb.append(", ");
            sb.append("NA, ");
            sb.append(location.getAltitude());
            sb.append(", ");
            sb.append(location.getVerticalAccuracyMeters());
            sb.append(", ");
        } else {
            sb.append("null, ");
            sb.append("null, ");
            sb.append("null, ");
            sb.append("68, ");
            sb.append("null, ");
            sb.append("NA, ");
            sb.append("null, ");
            sb.append("null, ");
        }

        if (address != null) {
            sb.append(address.getCountryCode());
            sb.append(", ");
            sb.append(address.getAdminArea());
            sb.append(", ");
            sb.append(address.getLocality());
            sb.append(", ");
            sb.append(address.getPostalCode());
        } else {
            sb.append("null, ");
            sb.append("null, ");
            sb.append("null, ");
            sb.append("null");
        }

        sb.append("]");

        return sb.toString();
    }

    private static String fromLocationInfo(String[] locationInfo) {
        if (locationInfo == null) {
            return "No location information";
        }

        StringBuilder sb = new StringBuilder();

        sb.append(
            "Location(lat|lon|accu|shape|conf|ts|method|country|state|city|postal|alt|vaccu)=");
        sb.append(java.util.Arrays.toString(locationInfo));

        return sb.toString();
    }

    private class AddressResolver {
        private final List<Location> mLocations = new ArrayList<Location>();
        private MessageExecutor mExecutor
                = new MessageExecutor(AddressResolver.class.getSimpleName());
        private Runner mRunner = new Runner();

        public AddressResolver() {
        }

        public void updateLocationDetails(Location location) {
            synchronized (mLocations) {
                mLocations.add(location);
            }

            mExecutor.execute(mRunner);
        }

        private Location getLocation() {
            Location location = null;

            synchronized (mLocations) {
                if (!mLocations.isEmpty()) {
                    location = mLocations.remove(0);
                }
            }

            return location;
        }

        private void updateLocationDetailsInternal(Location location, Address address) {
            if (address == null) {
                ImsLog.d(this, mSlotId, "Address is null");
                return;
            }

            if (ImsLog.isDebuggable()) {
                ImsLog.d(this, mSlotId,
                        LocationAgent.displayLocationDetails(location, address, null));
            }

            if (!location.isMock()) {
                setLastKnownCountryCode(address.getCountryCode());

                if (mPolicy.hasPolicy(LocationPolicy.POLICY_USE_CACHED_ADDRESS)) {
                    address.setLatitude(location.getLatitude());
                    address.setLongitude(location.getLongitude());
                    cacheAddress(address);
                }
            }
        }

        private final class Runner implements Runnable {
            @Override
            public void run() {
                final Location location = getLocation();

                if (location != null) {
                    final List<Address> addresses = mGeocoderProxy.getAddressesFromLocation(
                            location.getLatitude(), location.getLongitude(), 1);

                    if ((addresses != null) && !addresses.isEmpty()) {
                        updateLocationDetailsInternal(location, addresses.get(0));
                    }
                }
            }
        }
    }

    private boolean requestSmd() {
        if (mPolicy.hasPolicy(LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD) == false) {
            // SMD policy is not enabled
            return false;
        }

        if (mIsSmdRequested) {
            ImsLog.d(this, mSlotId, "SMD is already requested");
            return true;
        }

        ImsLog.d(this, mSlotId, "requestSmd");

        try {
            SensorManagerProxy smp =
                    AppContext.getInstance().getSystemServiceProxy(SensorManagerProxy.class);
            Sensor sigMotion = smp.getDefaultSensor(Sensor.TYPE_SIGNIFICANT_MOTION);
            smp.requestTriggerSensor(mSmdListener, sigMotion);
        } catch (Throwable t) {
            t.printStackTrace();
            return false;
        }

        mIsSmdRequested = true;

        // Stop update timer. It will be started when motion is detected
        stopTimer(ETimerType.TIMER_LOCATION_UPDATE_INTERVAL);
        mLocationRequestState = EReqState.STATE_IDLE;

        return true;
    }

    private void cancelSmd() {
        if (!mIsSmdRequested) {
            // SMD is already canceled
            return;
        }

        ImsLog.d(this, mSlotId, "cancelSmd");

        mIsSmdRequested = false;
        SensorManagerProxy smp =
                AppContext.getInstance().getSystemServiceProxy(SensorManagerProxy.class);
        Sensor sigMotion = smp.getDefaultSensor(Sensor.TYPE_SIGNIFICANT_MOTION);
        smp.cancelTriggerSensor(mSmdListener, sigMotion);
    }

    private Location updateCurrentTimeFromCachedTime(Location currentLocation) {
        ImsLog.d(this, mSlotId, "updateCurrentTimeFromCachedTime");

        Location updatedLocation = currentLocation;

        if (mPolicy.hasPolicy(LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD) == false) {
            return updatedLocation;
        }

        if (mCachedTimeMotionDetected == 0L) {
            return updatedLocation;
        }

        if (mCachedTimeMotionDetected > currentLocation.getTime()) {
            // if cached time is more recent than last known location info
            updatedLocation.setTime(mCachedTimeMotionDetected);
        }

        return updatedLocation;
    }

    private Boolean isGpsLocationUpdatedRecently() {
        if (mInstantLocationInfoRequested) {
            return false;
        }

        Location lastLocGPS = mLocationApi.getLastKnownLocation(LocationApi.GPS_PROVIDER);

        if (lastLocGPS != null) {
            long deltaSec = (SystemClock.elapsedRealtimeNanos()
                            - lastLocGPS.getElapsedRealtimeNanos())
                            / 1000000000L;
            if (deltaSec < mPolicy.getRecentLocationValidPeriod()) {
                ImsLog.d(this, mSlotId, "There is last known location acquired within "
                        + deltaSec + " seconds");
                return true;
            }
        }

        ImsLog.d(this, mSlotId, "There is no recent location");

        return false;
    }

    private void processLocationUpdateDone() {
        if (!requestSmd()) {
            if (mPolicy.hasPolicy(LocationPolicy.POLICY_LOCATION_NOT_ALLOWED_PERIODIC_POLLING)) {
                ImsLog.d(this, mSlotId, "Blocked: periodic location polling is not allowed");
            } else {
                stopTimer(ETimerType.TIMER_LOCATION_UPDATE_INTERVAL);
                startTimer(ETimerType.TIMER_LOCATION_UPDATE_INTERVAL, (getUpdateInterval()*1000));
            }
        }

        Location lastLocGPS = mLocationApi.getLastKnownLocation(LocationApi.GPS_PROVIDER);
        updateLocation(lastLocGPS);

        if (mPolicy.hasPolicy(
                LocationPolicy.POLICY_NOTIFY_LOCATION_FIXED_FOR_INSTANT_REQUEST)) {
            notifyEventOnLocationFixedForInstantRequest();
        }
    }

    // TODO: Need to refactor this based on carrier requirements.
    private void initPolicy() {
        LocationPolicy lp = null;
        int policy = LocationPolicy.POLICY_ENABLE_CACHED_LOCATION
                | LocationPolicy.POLICY_USE_CACHED_LOCATION;

        ConfigInterface config = getConfigInterface();
        if (config != null) {
            int updateType = CarrierConfig.Assets.LOCATION_UPDATE_POLICY_NONE;
            CarrierConfig cc = config.getCarrierConfig();
            if (cc != null) {
                updateType = cc.getInt(CarrierConfig.Assets.KEY_LOCATION_POLICY_UPDATE_TYPE_INT);
            }

            if ((CarrierConfig.Assets.LOCATION_UPDATE_POLICY_ONLY_WHEN_WFC_ENABLED == updateType
                    && ServiceCaps.isWfcEnabledByPlatform(mSlotId))
                    || CarrierConfig.Assets.LOCATION_UPDATE_POLICY_ALWAYS == updateType) {
                lp = getLocationPolicy();

                policy |= cc.getInt(CarrierConfig.Assets.KEY_LOCATION_ACQUISITION_POLICY_INT);
                if (cc.getBoolean(CarrierConfig.Assets
                        .KEY_LOCATION_ALLOW_MOCK_LOCATION_UPDATE_BOOL)) {
                    SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                            SubsInfoInterface.class, mSlotId);
                    if (subsInfo != null && subsInfo.isTestModeEnabled()) {
                        // For WFC E911 test which should be tested in Puerto Rico area,
                        // allow mock location
                        policy |= LocationPolicy.POLICY_ALLOW_MOCK_LOCATION_UPDATE;
                    }
                }

                lp.setDefaultAddressResolutionTime(cc.getInt(
                        CarrierConfig.Assets.KEY_LOCATION_ADDRESS_RESOLUTION_TIME_MILLIS_INT));
                int validityMinutes = cc.getInt(
                        CarrierConfig.Assets.KEY_LOCATION_VALIDITY_PERIOD_MIN_INT);
                lp.setValidityPeriod(validityMinutes * 60L * 1000L * 1000000L);
                validityMinutes = cc.getInt(
                        CarrierConfig.Assets.KEY_LOCATION_ADDRESS_VALIDITY_PERIOD_MIN_INT);
                lp.setAddressValidityPeriod(validityMinutes * 60L * 1000L * 1000000L);
                lp.setAddressTolerableDistance(cc.getInt(
                        CarrierConfig.Assets.KEY_LOCATION_TOLERABLE_DISTANCE_INT));
                lp.setSearchDurationForGps(cc.getInt(
                        CarrierConfig.Assets.KEY_LOCATION_GPS_SEARCHING_DURATION_SEC_INT));
                int shape = cc.getInt(CarrierConfig.Assets.KEY_LOCATION_GEODETIC_SHAPE_INT);
                lp.setShape(CarrierConfig.Assets.GEODETIC_SHAPE_ELLIPSOID == shape
                        ? LocationPolicy.SHAPE_ELLIPSOID : LocationPolicy.SHAPE_CIRCLE);
            }
        }

        if (lp == null && ServiceCaps.isWfcEnabledByPlatform(mSlotId)) {
            lp = getLocationPolicy();

            policy |= LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;
            policy |= LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD;
            policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_VIA_OTHER_SCHEME;
            policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE;
            policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_TIME;
            // policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_FROM_USIM;

            lp.setAddressTolerableDistance(3000);
            lp.setDefaultUpdateInterval(3600);
            // 10 Min: 10 * 60 * 1000 * 1000000L
            lp.setAddressValidityPeriod(10 * 60 * 1000 * 1000000L);
        }

        if (lp != null) {
            lp.setPolicy(policy);

            ImsLog.d(this, mSlotId, lp.toString());

            setLocationPolicy(lp);
        }
    }

    private ConfigInterface getConfigInterface() {
        return AgentFactory.getInstance().getAgent(ConfigInterface.class, mSlotId);
    }
}
