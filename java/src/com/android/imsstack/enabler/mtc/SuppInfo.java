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

package com.android.imsstack.enabler.mtc;

import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;

public class SuppInfo implements Parcelable {

    public static class SuppService {
        public int            type;

        public String        strValue;
        public int            intValue;
        public boolean        boolValue;
    };

    public ArrayList<SuppService> objSuppService = new ArrayList<SuppService>();

    // SuppService Type
    public static final int TYPE_CALLERID = 0;                          // int
    public static final int TYPE_CNAP = 1;                              // String
    public static final int TYPE_CNAPEX = 2;                            // String
    public static final int TYPE_MMC = 3;                               // boolean
    public static final int TYPE_GTT = 4;                               // boolean
    public static final int TYPE_CDIV_CAUSE = 5;                        // int
    public static final int TYPE_CDIV_HISTORY = 6;                      // String
    public static final int TYPE_CW = 7;                                // String
    public static final int TYPE_VM = 8;                                // boolean
    public static final int TYPE_HD = 9;                                // boolean
    public static final int TYPE_ANSWERHOLD = 10;                       // boolean
    public static final int TYPE_MCID = 11;                             // boolean
    public static final int TYPE_DUALNUMBER = 12;                       // String
    public static final int TYPE_ENFORCE_LT = 13;                       // boolean
    public static final int TYPE_TARGET_URI = 14;                       // String
    public static final int TYPE_CALLING_NUM_VERIFICATION = 15;         // int
    public static final int TYPE_VRBT = 16;                             // boolean
    public static final int TYPE_TIP = 17;                              // int, String
    public static final int TYPE_GEOLOCATION = 18;                      // boolean
    public static final int TYPE_CALL_PULL = 19;                        // boolean
    public static final int TYPE_CALL_COMPOSER_PRIORITY = 20;           // int
    public static final int TYPE_CALL_COMPOSER_SUBJECT = 21;            // String
    public static final int TYPE_CALL_COMPOSER_LOCATION_LAT = 22;       // String (from double)
    public static final int TYPE_CALL_COMPOSER_LOCATION_LONG = 23;      // String (from double)
    public static final int TYPE_CALL_COMPOSER_PICTURE_URL = 24;        // String
    public static final int TYPE_CALL_COMPOSER_IS_BUSINESS = 25;        // boolean
    public static final int TYPE_SESSION_ID = 26;                       // String

    // CallerID
    public static final int CALLERID_NONE = 0;
    public static final int CALLERID_NETWORK = 1;
    public static final int CALLERID_RESTRICTED = 2;
    public static final int CALLERID_IDENTITY = 3;

    // Calling Number Verification
    public static final int CALLING_NUM_VERSTAT_NONE = 0;
    public static final int CALLING_NUM_VERSTAT_VERIFIED = 1;
    public static final int CALLING_NUM_VERSTAT_NOT_VERIFIED = 2;

    // TIP
    public static final int TIP_NONE = 0;
    public static final int TIP_IDENTITY = 1;
    public static final int TIP_RESTRICTED = 2;

    //------------------------------------------------------------------------------------------//

    public SuppInfo() {
        ImsLog.i("");
    }

    public SuppInfo(SuppInfo suppInfo) {

        int service_num = suppInfo.objSuppService.size();
        ImsLog.w("service_num[" + service_num + "]");

        for (int index = 0; index < service_num; index++) {

            SuppService service = suppInfo.objSuppService.get(index);
            SuppService tService = new SuppService();

            tService.type = service.type;
            tService.strValue = service.strValue;
            tService.intValue = service.intValue;
            tService.boolValue = service.boolValue;

            objSuppService.add(tService);
        }

        logIn("init");
    }
    public SuppInfo(Parcel source) {
        readFromParcel(source);
    }

    public void logIn(String tag) {
        ImsLog.i(tag + " - size[" + objSuppService.size() + "]");

        for (int index = 0; index < objSuppService.size(); index++) {

            SuppService service = objSuppService.get(index);

            ImsLog.i("[" + index + "]"
                    + " type : " + service.type
                    + " strValue : " + ImsLog.hiddenString(service.strValue)
                    + " intValue : " + service.intValue
                    + " boolValue : " + service.boolValue
                     );
        }

    }

    public void addService_str(int _type, String _value) {

        ImsLog.d("addService_str() [" + objSuppService.size() + "]" +
                "[" + _type + "]" + "[" + _value + "]");

        SuppService service = new SuppService();
        service.type = _type;
        service.strValue= _value;
        service.intValue = 0;
        service.boolValue = false;

        objSuppService.add(service);

    }

    public void addService_int(int _type, int _value) {

        ImsLog.d("addService_int() [" + objSuppService.size() + "]" +
                "[" + _type + "]" + "[" + _value + "]");

        SuppService service = new SuppService();
        service.type = _type;
        service.strValue= null;
        service.intValue = _value;
        service.boolValue = false;

        objSuppService.add(service);

    }

    public void addService_bool(int _type, boolean _value) {

        ImsLog.d("addService_bool() [" + objSuppService.size() + "]" +
                "[" + _type + "]" + "[" + _value + "]");

        SuppService service = new SuppService();
        service.type = _type;
        service.strValue= null;
        service.intValue = 0;
        service.boolValue = _value;

        objSuppService.add(service);

    }

    /**
     * This method is to set and service values
     *
     * @param type service type
     * @param boolValue service bool value
     * @param intValue service int value
     * @param strValue service string value
     */
    @VisibleForTesting
    public void addService(int type, boolean boolValue, int intValue, String strValue) {
        ImsLog.d("addService [" + objSuppService.size() + "]"
                + "[" + type + "]" + "[" + boolValue + "]"
                + "[" + intValue + "]" + "[" + strValue + "]");

        SuppService service = new SuppService();
        service.type = type;
        service.strValue = strValue;
        service.intValue = intValue;
        service.boolValue = boolValue;
        objSuppService.add(service);
    }

    public int getServiceSize() {

        ImsLog.i("getServiceSize : [" + objSuppService.size() + "]");
        return objSuppService.size();
    }

    public boolean isService(int type) {

        boolean bIs = false;

        for (int index = 0; index < objSuppService.size(); index++) {

            SuppService service = objSuppService.get(index);

            if ( service.type == type ) {
                bIs = true;
            }
        }

        ImsLog.i("isService : [" + type + "]" + "[" + bIs + "]");
        return bIs;
    }

    public SuppService getService(int type) {

        ImsLog.i("getService : [" + type + "]");

        for (int index = 0; index < objSuppService.size(); index++) {

            SuppService service = objSuppService.get(index);

            if ( service.type == type ) {
                return service;
            }
        }

        return null;
    }

    public void updateService(SuppService _service) {
        boolean bUpdated = false;
        int oldSize = objSuppService.size();

        for (int index = 0; index < objSuppService.size(); index++) {

            SuppService service = objSuppService.get(index);

            if ( service.type == _service.type ) {
                service.strValue = _service.strValue;
                service.intValue = _service.intValue;
                service.boolValue = _service.boolValue;
                bUpdated = true;
            }
        }

        if ( !bUpdated ) {
            SuppService service = new SuppService();
            service.type = _service.type;
            service.strValue = _service.strValue;
            service.intValue = _service.intValue;
            service.boolValue = _service.boolValue;

            objSuppService.add(service);
        }

        ImsLog.i("updateService() [" + oldSize + " >> " + objSuppService.size()
                + "], type=" + _service.type);
    }

    public void updateSuppInfo(SuppInfo _suppInfo) {
        int oldSize = objSuppService.size();

        objSuppService.clear();

        for (int index = 0; index < _suppInfo.objSuppService.size(); index++) {
            updateService(_suppInfo.objSuppService.get(index));
        }

        ImsLog.i("updateSuppInfo() [" + oldSize + " >> " + objSuppService.size() + "]");
    }

    public void readFromParcel(Parcel source) {
        int service_num = source.readInt();

        ImsLog.d("readFromParcel() " + service_num);

        for (int index = 0; index < service_num; index++) {

            SuppService service = new SuppService();
            service.type = source.readInt();
            service.strValue= source.readString();
            service.intValue = source.readInt();
            if ( source.readInt() == 1 ) {
                service.boolValue= true;
            }
            else {
                service.boolValue = false;
            }

            objSuppService.add(service);

        }

        logIn("read");
    }

    public void writeToParcel(Parcel dest, int flags) {
        logIn("write");

        dest.writeInt(objSuppService.size());

        for (int index = 0; index < objSuppService.size(); index++) {

            SuppService service = objSuppService.get(index);

            dest.writeInt(service.type);

            dest.writeString(service.strValue);
            dest.writeInt(service.intValue);

            if (service.boolValue) {
                dest.writeInt(1);
            } else {
                dest.writeInt(0);
            }
        }

    }

    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<SuppInfo> CREATOR =
            new Parcelable.Creator<SuppInfo>() {
        public SuppInfo createFromParcel(Parcel source) {
            return new SuppInfo(source);
        }

        public SuppInfo[] newArray(int size) {
            return new SuppInfo[size];
        }
    };
}
