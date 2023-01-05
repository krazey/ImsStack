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

package com.android.imsstack.enabler.mtc.conf;

import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;

public class UsersInfo implements Parcelable {

    public static class User{
        public long         callID = 0;
        public String       target = "";

        public String       userEntity = "";
        public String       epEntity = "";
        public String       displayName = "";

        public int          status = USER_STATUS_IDLE;
        public int          statusCode = 0;

        public int          ccType = CCTYPE_TO;
        public boolean      anonymize = false;
    };

    public ArrayList<User>    Users = new ArrayList<User>();

    // status
    public static final int USER_STATUS_IDLE                = 0;

    public static final int USER_STATUS_PROGRESSING         = 1;
    public static final int USER_STATUS_CONNECTED           = 2;
    public static final int USER_STATUS_DISCONNECTED        = 3;
    public static final int USER_STATUS_ONHOLD              = 4;
    public static final int USER_STATUS_MUTEDVIAFOCUS       = 5;
    public static final int USER_STATUS_PENDING             = 6;
    public static final int USER_STATUS_ALERTING            = 7;
    public static final int USER_STATUS_DIALING_IN          = 8;
    public static final int USER_STATUS_DIALING_OUT         = 9;
    public static final int USER_STATUS_DISCONNECTING       = 10;

    public static final int USER_STATUS_FAIL                = 20;
    public static final int USER_STATUS_REJECT              = 21;
    public static final int USER_STATUS_BUSY                = 22;
    public static final int USER_STATUS_SERVERERROR         = 23;
    public static final int USER_STATUS_NOTSUPPORTED        = 24;
    public static final int USER_STATUS_NOTACCEPTABLE       = 25;
    public static final int USER_STATUS_NOANSWER            = 26;
    public static final int USER_STATUS_NOTREACHABLE        = 27;
    public static final int USER_STATUS_LOWBATTERY          = 28;
    public static final int USER_STATUS_FORBIDDEN           = 29;
    public static final int USER_STATUS_INTSERVERERROR      = 30;


    // ccType
    public static final int CCTYPE_TO       = 0;
    public static final int CCTYPE_CC       = 1;
    public static final int CCTYPE_BCC      = 2;

    //------------------------------------------------------------------------------------------//
    public UsersInfo() {
        ImsLog.i("");
    }

    public UsersInfo(UsersInfo userInfo) {

        int users_num = userInfo.Users.size();
        ImsLog.i("Users[" + users_num + "]");

        for (int index = 0; index < users_num; index++) {
            User user = userInfo.Users.get(index);
            addUser(user);
        }

    }

    public UsersInfo(Parcel source) {
        readFromParcel(source);
    }

    public void logIn(String tag) {

        ImsLog.i(tag + " - User : size[" + Users.size() + "]");

        for (int index = 0; index < Users.size(); index++) {

            User user = Users.get(index);
            ImsLog.i("[" + index + "]"
                    + " callID : " + user.callID
                    + " target : " + ImsLog.hiddenString(user.target)
                    + " userEntity : " + ImsLog.hiddenString(user.userEntity)
                    + " epEntity : " + ImsLog.hiddenString(user.epEntity)
                    + " displayName : " + ImsLog.hiddenString(user.displayName)
                    + " status : " + user.status
                    + " statuscode : " + user.statusCode
                    + " ccType : " + user.ccType
                    + " anonymize : " + user.anonymize
                     );
        }

    }

    public int getSize() {

        ImsLog.i("getSize : [" + Users.size() + "]");
        return Users.size();
    }

    public User getUser(int index) {

        ImsLog.i("getUser : [" + index + "]");
        return Users.get(index);
    }

    public void addUser(User _user) {

        if (ImsLog.isDebuggable()) {
            ImsLog.d("addUser [" + Users.size() + "]"
                        + " callID : " + _user.callID
                        + " target : " + _user.target
                        + " userEntity : " + _user.userEntity
                        + " epEntity : " + _user.epEntity
                        + " displayName : " + _user.displayName
                        + " status : " + _user.status
                        + " statusCode : " + _user.statusCode
                        + " ccType : " + _user.ccType
                        + " anonymize : " + _user.anonymize
                        );
        }

        User user = new User();
        user.callID = _user.callID;
        user.target = _user.target;
        user.userEntity = _user.userEntity;
        user.epEntity = _user.epEntity;
        user.displayName = _user.displayName;
        user.status = _user.status;
        user.statusCode = _user.statusCode;
        user.ccType = _user.ccType;
        user.anonymize = _user.anonymize;

        Users.add(user);
        logIn("addUser");
    }

    public void addUser(long _callID, String _target, String _userEntity, String _epEntity, String _displayName, int _status, int _ccType, boolean _anonymize) {

        if (ImsLog.isDebuggable()) {
            ImsLog.d("addUser [" + Users.size() + "]"
                        + " callID : " + _callID
                        + " target : " + _target
                        + " userEntity : " + _userEntity
                        + " epEntity : " + _epEntity
                        + " displayName : " + _displayName
                        + " status : " + _status
                        + " ccType : " + _ccType
                        + " anonymize : " + _anonymize
                        );
        }

        User user = new User();
        user.callID = _callID;
        user.target = _target;
        user.userEntity = _userEntity;
        user.epEntity = _epEntity;
        user.displayName = _displayName;
        user.status = _status;
        user.statusCode = -1;
        user.ccType = _ccType;
        user.anonymize = _anonymize;

        Users.add(user);
        logIn("addUser");
    }

    public void readFromParcel(Parcel source) {

        int users_num = source.readInt();
        ImsLog.i("users[" + users_num + "]");

        for (int index = 0; index < users_num; index++) {
            long        callID = source.readLong();
            String      target = source.readString();
            String      userEntity = source.readString();
            String      epEntity = source.readString();
            String      displayName = source.readString();
            int         status = source.readInt();
            int         statusCode = source.readInt();
            int         ccType = source.readInt();
            boolean     anonymize;
            if (source.readInt() == 1) {
                anonymize = true;
            }
            else {
                anonymize = false;
            }

            User user = new User();
            user.callID = callID;
            user.target = target;
            user.userEntity = userEntity;
            user.epEntity = epEntity;
            user.displayName = displayName;
            user.status = status;
            user.statusCode = statusCode;
            user.ccType = ccType;
            user.anonymize = anonymize;

            Users.add(user);

        }
        logIn("read");

    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(Users.size());
        for (int index = 0; index < Users.size(); index++) {
            User user = Users.get(index);

            dest.writeLong(user.callID);
            dest.writeString(user.target);
            dest.writeString(user.userEntity);
            dest.writeString(user.epEntity);
            dest.writeString(user.displayName);
            dest.writeInt(user.status);
            dest.writeInt(user.statusCode);
            dest.writeInt(user.ccType);

            if (user.anonymize) {
                dest.writeInt(1);
            } else {
                dest.writeInt(0);
            }
        }

        logIn("write");
    }

    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<UsersInfo> CREATOR =
            new Parcelable.Creator<UsersInfo>() {
        public UsersInfo createFromParcel(Parcel source) {
            return new UsersInfo(source);
        }

        public UsersInfo[] newArray(int size) {
            return new UsersInfo[size];
        }
    };
}
