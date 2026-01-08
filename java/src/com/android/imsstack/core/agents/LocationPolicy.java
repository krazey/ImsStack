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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Objects;

/**
 * This class defines the rules to obtain the location information.
 */
public final class LocationPolicy {
    public static final int POLICY_NONE = 0;
    /**
     * Indicates if the cached location can be provided for location information.
     */
    public static final int POLICY_ENABLE_CACHED_LOCATION = 0x00000001;
    /**
     * Indicates if the cached location can be used when getting the last known location.
     */
    public static final int POLICY_USE_CACHED_LOCATION = 0x00000002;
    /**
     * Even if the location can't be resolved,
     * it requires to provide country information using various methods.
     */
    public static final int POLICY_PROVIDE_COUNTRY_IF_NO_LOCATION = 0x00000004;
    /**
     * Indicates if the fixed location update interval is used.
     */
    public static final int POLICY_USE_FIXED_LOCATION_UPDATE_INTERVAL = 0x00000008;
    /**
     * Indicates if the last known country can be updated via other schemes.
     */
    public static final int POLICY_UPDATE_COUNTRY_VIA_OTHER_SCHEME = 0x00000010;
    /**
     * Indicates if the last known country can be updated by MCC/MNC in USIM.
     */
    public static final int POLICY_UPDATE_COUNTRY_FROM_USIM = 0x00000020;
    /**
     * Indicates if MOCK location can be updated or not.
     */
    public static final int POLICY_ALLOW_MOCK_LOCATION_UPDATE = 0x00000040;
    /**
     * Indicates if it requires the change notification (to native layer) of country information.
     */
    public static final int POLICY_NOTIFY_COUNTRY_CHANGED_EVENT = 0x00000080;
    /**
     * Indicates if location field should be initialized before getting last known location
     * from LocationManager.
     */
    public static final int POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION = 0x00000100;
    /**
     * Indicates if location fix has been accomplished for an instant location info request.
     */
    public static final int POLICY_NOTIFY_LOCATION_FIXED_FOR_INSTANT_REQUEST = 0x00000200;

    /**
     * Indicates if the last known country can be updated via other schemes.
     */
    public static final int POLICY_UPDATE_COUNTRY_VIA_SHARED_PREFERENCES = 0x00000800;

    /**
     * Indicates if address caching mechanism is used.
     * If there's no cached address, then it'll make a new address translation.
     */
    public static final int POLICY_USE_CACHED_ADDRESS = 0x00001000;

    /**
     * Indicates if only address caching mechanism is used to acquire an address.
     * If there's no cached address, then it'll return null for address.
     */
    public static final int POLICY_USE_ONLY_CACHED_ADDRESS = 0x00002000;

    /**
     * Indicates if a cached address is valid considering distance.
     * A cached address is valid if distance between the location address is translated from
     * and the last known location is short.
     */
    public static final int POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE = 0x00004000;

    /**
     * Indicates if a cached address is valid considering time.
     * A cached address is valid if the address translation is made not so long ago.
     */
    public static final int POLICY_CACHED_ADDRESS_VALIDITY_TIME = 0x00008000;

    /**
     * Indicates if enhanced location query scheme using SMD (Significant Motion Detection)
     * is used.
     * It is efficient to reduce current consumption in stationary condition
     */
    public static final int POLICY_LOCATION_UPDATE_USING_SMD = 0x00010000;

    /**
     * Indicates if peridic location polling is not used. Location can be updated by event driven.
     */
    public static final int POLICY_LOCATION_NOT_ALLOWED_PERIODIC_POLLING = 0x00020000;

    /**
     * Indicates if updates address directly after location acquired. (deprecated scheme)
     * This is not strongly recommended for reducing Google overloads of geocoding.
     */
    public static final int POLICY_UPDATE_ADDRESS_AFTER_LOCATION_ACQUIRED = 0x00040000;

    /**
     * Indicates if fused location provider is used instead of NLP + GPS.
     */
    public static final int POLICY_USE_FUSED_PROVIDER = 0x00080000;

    /** 10 minutes. (seconds) */
    public static final int LOCATION_UPDATE_INTERVAL = 10 * 60;
    /** 20 seconds. */
    public static final int LOCATION_SEARCH_DURATION_NLP = 20;
    /** 30 seconds. */
    public static final int LOCATION_SEARCH_DURATION_GPS = 30;
    /** 30 seconds. */
    public static final int LOCATION_SEARCH_DURATION_FLP = 30;
    /** 2 seconds. */
    public static final int ADDRESS_RESOLUTION_MAX_TIME = 2000;
    /** 3 seconds. (round-trip delay time) */
    public static final int ADDRESS_RESOLUTION_RTD_TIME = 3000;
    /** 30 minutes */
    public static final long LOCATION_VALIDITY_PERIOD = 30 * 60 * 1000 * 1000000L;
    /** 3 minutes for short location validity period */
    public static final long LOCATION_VALIDITY_PERIOD_SHORT = 3 * 60 * 1000 * 1000000L;
    /** 120 seconds. */
    public static final int RECENT_LOCATION_VALID_PERIOD = 120;

    public static final String SHAPE_CIRCLE = "Circle";
    public static final String SHAPE_ELLIPSOID = "Ellipsoid";

    private int mPolicy = POLICY_NONE;
    private int mDefaultUpdateInterval = LOCATION_UPDATE_INTERVAL;
    private int mSearchDurationNlp = LOCATION_SEARCH_DURATION_NLP;
    private int mSearchDurationGps = LOCATION_SEARCH_DURATION_GPS;
    private int mSearchDurationFlp = LOCATION_SEARCH_DURATION_FLP;
    private int mDefaultAddressResolutionTime = ADDRESS_RESOLUTION_MAX_TIME;
    private int mFixedUpdateInterval = 0;
    private long mValidityPeriod = 0L;
    private int mCachedAddressTolerableDistance = 0;
    private long mAddressValidityPeriod = 0L;
    private int mRecentLocationValidPeriod = RECENT_LOCATION_VALID_PERIOD;
    private String mShape = SHAPE_CIRCLE;

    public LocationPolicy() {
    }

    public LocationPolicy(@NonNull LocationPolicy lp) {
        Objects.requireNonNull(lp, "lp must not be null");
        set(lp);
    }

    public LocationPolicy(int policy, int validityPeriod) {
        mPolicy = policy;
        mValidityPeriod = validityPeriod;
    }

    /**
     * Resets all the member fields.
     */
    public void reset() {
        mPolicy = POLICY_NONE;
        mDefaultUpdateInterval = LOCATION_UPDATE_INTERVAL;
        mSearchDurationNlp = LOCATION_SEARCH_DURATION_NLP;
        mSearchDurationGps = LOCATION_SEARCH_DURATION_GPS;
        mSearchDurationFlp = LOCATION_SEARCH_DURATION_FLP;
        mDefaultAddressResolutionTime = ADDRESS_RESOLUTION_MAX_TIME;
        mFixedUpdateInterval = 0;
        mValidityPeriod = 0L;
        mAddressValidityPeriod = 0L;
        mCachedAddressTolerableDistance = 0;
        mRecentLocationValidPeriod = RECENT_LOCATION_VALID_PERIOD;
        mShape = SHAPE_CIRCLE;
    }

    /**
     * Sets the location policy from the given object.
     *
     * @param lp The location policy to be set.
     */
    public void set(@NonNull LocationPolicy lp) {
        Objects.requireNonNull(lp, "lp must not be null");
        mPolicy = lp.mPolicy;
        mDefaultUpdateInterval = lp.mDefaultUpdateInterval;
        mSearchDurationNlp = lp.mSearchDurationNlp;
        mSearchDurationGps = lp.mSearchDurationGps;
        mSearchDurationFlp = lp.mSearchDurationFlp;
        mDefaultAddressResolutionTime = lp.mDefaultAddressResolutionTime;
        mFixedUpdateInterval = lp.mFixedUpdateInterval;
        mValidityPeriod = lp.mValidityPeriod;
        mAddressValidityPeriod = lp.mAddressValidityPeriod;
        mCachedAddressTolerableDistance = lp.mCachedAddressTolerableDistance;
        mRecentLocationValidPeriod = lp.mRecentLocationValidPeriod;
        mShape = lp.mShape;
    }

    /**
     * Returns the curren policy.
     */
    public int getPolicy() {
        return mPolicy;
    }

    /**
     * Checks whether the specified policy is contained or not.
     */
    public boolean hasPolicy(int policy) {
        return (mPolicy & policy) == policy;
    }

    /**
     * Clears the specified policy.
     */
    public void clearPolicy(int policy) {
        mPolicy &= (~policy);
    }

    /**
     * Sets the specified policy.
     */
    public void setPolicy(int policy) {
        mPolicy |= policy;
    }

    /**
     * Gets the default interval for location update. (seconds)
     */
    public int getDefaultUpdateInterval() {
        return mDefaultUpdateInterval;
    }

    /**
     * Sets the default interval for location update. (seconds)
     */
    public void setDefaultUpdateInterval(int interval) {
        mDefaultUpdateInterval = interval;
    }

    /**
     * Gets the searching duration for NLP (network location provider). (seconds)
     */
    public int getSearchDurationForNlp() {
        return mSearchDurationNlp;
    }

    /**
     * Sets the searching duration for NLP (network location provider). (seconds)
     */
    public void setSearchDurationForNlp(int duration) {
        mSearchDurationNlp = duration;
    }

    /**
     * Gets the searching duration for GPS. (seconds)
     */
    public int getSearchDurationForGps() {
        return mSearchDurationGps;
    }

    /**
     * Sets the searching duration for GPS. (seconds)
     */
    public void setSearchDurationForGps(int duration) {
        mSearchDurationGps = duration;
    }

    /**
     * Gets the searching duration for FLP. (seconds)
     */
    public int getSearchDurationForFlp() {
        return mSearchDurationFlp;
    }

    /**
     * Sets the searching duration for FLP. (seconds)
     */
    public void setSearchDurationForFlp(int duration) {
        mSearchDurationFlp = duration;
    }

    /**
     * Gets the fixed interval for location update. (seconds)
     * It can be applicable when POLICY_USE_FIXED_LOCATION_UPDATE_INTERVAL is set.
     */
    public int getFixedUpdateInterval() {
        return mFixedUpdateInterval;
    }

    /**
     * Sets the interval for location update. (seconds)
     * It can be applicable when POLICY_USE_FIXED_LOCATION_UPDATE_INTERVAL is set.
     */
    public void setFixedUpdateInterval(int interval) {
        mFixedUpdateInterval = interval;
    }

    /**
     * Gets the validity period of the location when cached location is used.
     */
    public long getValidityPeriod() {
        return mValidityPeriod;
    }

    /**
     * Sets the validity period of the location when cached location is used.
     */
    public void setValidityPeriod(long nanos) {
        mValidityPeriod = nanos;
    }

    /**
     * Gets the validity period of the cached address.
     */
    public long getAddressValidityPeriod() {
        return mAddressValidityPeriod;
    }

    /**
     * Sets the validity period of the cached address.
     */
    public void setAddressValidityPeriod(long nanos) {
        mAddressValidityPeriod = nanos;
    }

    /**
     * Gets the tolerable distance in meters to use cached address.
     */
    public int getAddressTolerableDistance() {
        return mCachedAddressTolerableDistance;
    }

    /**
     * Sets the tolerable distance in meters to use cached address.
     */
    public void setAddressTolerableDistance(int distance) {
        mCachedAddressTolerableDistance = distance;
    }

    /**
     * Gets the default time for address resolution. (milli seconds)
     */
    public int getDefaultAddressResolutionTime() {
        return mDefaultAddressResolutionTime;
    }

    /**
     * Sets the default time for address resolution. (milli seconds)
     */
    public void setDefaultAddressResolutionTime(int millis) {
        mDefaultAddressResolutionTime = millis;
    }

    /**
     * Gets the valid period to determine recent location. (seconds)
     */
    public int getRecentLocationValidPeriod() {
        return mRecentLocationValidPeriod;
    }

    /**
     * Sets the valid period to determine recent location. (seconds)
     */
    public void setRecentLocationValidPeriod(int duration) {
        mRecentLocationValidPeriod = duration;
    }

    /**
     * Gets the PIDF-LO geodetic shape.
     */
    public String getShape() {
        return mShape;
    }

    /**
     * Sets the PIDF-LO geodetic shape.
     */
    public void setShape(String shape) {
        mShape = shape;
    }

    @Override
    public boolean equals(@Nullable Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof LocationPolicy)) {
            return false;
        }

        LocationPolicy lp = (LocationPolicy) o;
        return mPolicy == lp.mPolicy
                && mDefaultUpdateInterval == lp.mDefaultUpdateInterval
                && mSearchDurationNlp == lp.mSearchDurationNlp
                && mSearchDurationGps == lp.mSearchDurationGps
                && mSearchDurationFlp == lp.mSearchDurationFlp
                && mDefaultAddressResolutionTime == lp.mDefaultAddressResolutionTime
                && mFixedUpdateInterval == lp.mFixedUpdateInterval
                && mValidityPeriod == lp.mValidityPeriod
                && mCachedAddressTolerableDistance == lp.mCachedAddressTolerableDistance
                && mAddressValidityPeriod == lp.mAddressValidityPeriod
                && mRecentLocationValidPeriod == lp.mRecentLocationValidPeriod
                && Objects.equals(mShape, lp.mShape);
    }

    @Override
    public int hashCode() {
        return Objects.hash(mPolicy,
                mDefaultUpdateInterval,
                mSearchDurationNlp,
                mSearchDurationGps,
                mSearchDurationFlp,
                mDefaultAddressResolutionTime,
                mFixedUpdateInterval,
                mValidityPeriod,
                mCachedAddressTolerableDistance,
                mAddressValidityPeriod,
                mRecentLocationValidPeriod,
                mShape);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("LocationPolicy: [ ");
        sb.append("policy=" + Integer.toHexString(mPolicy));
        sb.append(", defaultUpdateInterval=" + mDefaultUpdateInterval);
        sb.append(", searchDurationNlp=" + mSearchDurationNlp);
        sb.append(", searchDurationGps=" + mSearchDurationGps);
        sb.append(", searchDurationFlp=" + mSearchDurationFlp);
        sb.append(", fixedUpdateInterval=" + mFixedUpdateInterval);
        sb.append(", validityPeriod=" + mValidityPeriod);
        sb.append(", addressValidityPeriod=" + mAddressValidityPeriod);
        sb.append(", cachedAddressTolerableDistance=" + mCachedAddressTolerableDistance);
        sb.append(", mDefaultAddressResolutionTime=" + mDefaultAddressResolutionTime);
        sb.append(", mRecentLocationValidPeriod=" + mRecentLocationValidPeriod);
        sb.append(", mShape=" + mShape);
        sb.append(" ]");

        return sb.toString();
    }
}
