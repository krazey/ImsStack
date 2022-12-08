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
package com.android.imsstack.util;

import android.content.Context;
import android.location.Address;
import android.location.Geocoder;
import android.location.Location;
import android.os.Handler;

import com.android.imsstack.util.ImsLog;

import java.io.IOException;
import java.util.List;
import java.util.Locale;

public class GeocoderProxy {
    public static final String UNKNOWN_COUNTRY = "ZZ";
    private static final int MAX_RESULTS = 1;

    private final Object mLock = new Object();
    private final Context mContext;
    private final Handler mHandler;
    private final GeocodeExecutor mGeocodeExecutor;
    private double mLatitude;
    private double mLongitude;
    private List<Address> mAddresses;

    public GeocoderProxy(Context context, Handler handler) {
        mContext = context;
        mHandler = handler;
        mGeocodeExecutor = new GeocodeExecutor();
    }

    public static String getCountryFromLocation(Context context, Location location) {
        if (location == null) {
            return UNKNOWN_COUNTRY;
        }

        Address address = getAddressFromLocation(context, location);

        return (address != null) ? address.getCountryCode() : UNKNOWN_COUNTRY;
    }

    public static Address getAddressFromLocation(Context context, Location location) {
        if (location == null) {
            return null;
        }

        return getAddressFromLocation(context,
                location.getLatitude(), location.getLongitude());
    }

    public static Address getAddressFromLocation(Context context,
            double latitude, double longitude) {
        List<Address> addresses = getAddressesFromLocation(
                context, latitude, longitude, MAX_RESULTS);

        if (addresses == null) {
            return null;
        } else if (addresses.isEmpty()) {
            return null;
        }

        return addresses.get(0);
    }

    public static List<Address> getAddressesFromLocation(Context context,
            double latitude, double longitude, int maxResults) {
        try {
            Geocoder gc = new Geocoder(context, Locale.US);
            return gc.getFromLocation(latitude, longitude, maxResults);
        } catch (IOException e) {
            ImsLog.w("GeocoderProxy :: " + e.getMessage());
        } catch (Exception e) {
            ImsLog.e("GeocoderProxy :: " + e.getMessage());
        }

        return null;
    }

    /**
     * @param waitTime waiting time to decode the location
     */
    @SuppressWarnings("WaitNotInLoop")
    public void findAddressFromLocation(long waitTime) {
        synchronized (mLock) {
            mHandler.post(mGeocodeExecutor);

            try {
                mLock.wait(waitTime);
            } catch (InterruptedException e) {
                ImsLog.w("Interrupted while trying to get address from Geocode");
            }
        }
    }

    public Address getAddress() {
        synchronized (mLock) {
            if (mAddresses == null) {
                return null;
            } else if (mAddresses.isEmpty()) {
                return null;
            }

            return mAddresses.get(0);
        }
    }

    public List<Address> getAddresses() {
        synchronized (mLock) {
            return mAddresses;
        }
    }

    public void setLatAndLong(double latitude, double longitude) {
        synchronized (mLock) {
            mLatitude = latitude;
            mLongitude = longitude;
            mAddresses = null;
        }
    }

    private final class GeocodeExecutor implements Runnable {
        @Override
        public void run() {
            double latitude;
            double longitude;

            synchronized (mLock) {
                latitude = mLatitude;
                longitude = mLongitude;
            }

            final List<Address> addresses = getAddressesFromLocation(
                    mContext, latitude, longitude, MAX_RESULTS);

            synchronized (mLock) {
                mAddresses = addresses;
                mLock.notifyAll();
            }
        }
    }
}
