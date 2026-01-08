/*
 * Copyright (C) 2023 The Android Open Source Project
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

package com.android.imsstack.enabler.mtc.externalcalls;

import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.ims.ImsExternalCallState;

import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;

/**
 * Parcelable object to handle MultiEndpoint Dialog Event Package Information.
 */
public class ExternalCalls implements Parcelable {

    private ArrayList<ImsExternalCallState> mImsExternalCallStates =
            new ArrayList<ImsExternalCallState>();

    /**
     * Creates a new ExternalCalls instance to contain MultiEndpoint Dialog information.
     *
     * @param externalCalls The ExternalCalls object to be copied.
     */
    public ExternalCalls(ExternalCalls externalCalls) {
        for (ImsExternalCallState imsExternalCallState : externalCalls.mImsExternalCallStates) {
            mImsExternalCallStates.add(cloneImsExternalCallState(imsExternalCallState));
        }
    }

    /**
     * Creates a new ExternalCalls instance to contain MultiEndpoint Dialog information.
     *
     * @param source The Parcel that has MultiEndpoint Dialog Event Package Information.
     */
    public ExternalCalls(Parcel source) {
        readFromParcel(source);
    }

    /**
     * Leaves logs of an every member of list of ImsExternalCallState with tag.
     */
    public void logLn(String tag) {
        ImsLog.i(tag + " - mImsExternalCallStates : size[" + mImsExternalCallStates.size() + "]");

        for (int index = 0; index < mImsExternalCallStates.size(); index++) {
            ImsExternalCallState imsExternalCallState = mImsExternalCallStates.get(index);
            ImsLog.i("[" + index + "]" + imsExternalCallState.toString());
        }
    }

    /**
     * Leaves logs of the received ImsExternalCallState.
     */
    public void logLnImsExternalCallState(ImsExternalCallState imsExternalCallState) {
        ImsLog.i(imsExternalCallState.toString());
    }

    /**
     * Gets each ImsExternalCallState object from list.
     *
     * @param index The position of the ImsExternalCallState object in the list.
     * @return The object.
     */
    public ImsExternalCallState getImsExternalCallState(int index) {
        ImsLog.i("getImsExternalCallState : [" + index + "]");
        return mImsExternalCallStates.get(index);
    }

    /**
     * Gets list of ImsExternalCallState object .
     *
     * @return The list.
     */
    public ArrayList<ImsExternalCallState> getImsExternalCallStates() {
        return mImsExternalCallStates;
    }

    /**
     * Clones ImsExternalCallState object.
     *
     * @return The object.
     */
    public ImsExternalCallState cloneImsExternalCallState(
            ImsExternalCallState imsExternalCallState) {
        logLnImsExternalCallState(imsExternalCallState);

        ImsExternalCallState newImsExternalCallState =
                new ImsExternalCallState(imsExternalCallState.getCallId(),
                imsExternalCallState.getAddress(), imsExternalCallState.getLocalAddress(),
                imsExternalCallState.isCallPullable(), imsExternalCallState.getCallState(),
                imsExternalCallState.getCallType(), imsExternalCallState.isCallHeld());

        logLn("cloneImsExternalCallState");
        return newImsExternalCallState;
    }

    /**
     * Reads Dialog Event Package Information from Parcel and makes ImsExternalCallState instance
     * to put into list
     */
    public void readFromParcel(Parcel source) {
        int externalCallSize = source.readInt();
        ImsLog.i("ImsExternalCallStates[" + externalCallSize + "]");

        for (int index = 0; index < externalCallSize; index++) {
            ImsExternalCallState newImsExternalCallState =
                    new ImsExternalCallState(source.readString(),
                    Uri.parse(source.readString()), Uri.parse(source.readString()),
                    source.readInt() == 1 ? true : false, source.readInt(),
                    source.readInt(), source.readInt() == 1 ? true : false);

            mImsExternalCallStates.add(newImsExternalCallState);
        }
        logLn("readFromParcel");
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mImsExternalCallStates.size());
        for (int index = 0; index < mImsExternalCallStates.size(); index++) {
            ImsExternalCallState imsExternalCallState = mImsExternalCallStates.get(index);

            dest.writeString(Integer.toString(imsExternalCallState.getCallId()));
            dest.writeString(imsExternalCallState.getAddress().toString());
            dest.writeString(imsExternalCallState.getLocalAddress().toString());
            dest.writeInt(imsExternalCallState.isCallPullable() ? 1 : 0);
            dest.writeInt(imsExternalCallState.getCallState());
            dest.writeInt(imsExternalCallState.getCallType());
            dest.writeInt(imsExternalCallState.isCallHeld() ? 1 : 0);
        }

        logLn("writeToParcel");
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<ExternalCalls> CREATOR =
            new Parcelable.Creator<ExternalCalls>() {
        public ExternalCalls createFromParcel(Parcel in) {
            try {
                return new ExternalCalls(in);
            } catch (Exception e) {
                ImsLog.e("createFromParcel() ::"
                        + "Exception occurred when creating ExternalCalls from parcel", e);
            }
            return null;
        }

        public ExternalCalls[] newArray(int size) {
            return new ExternalCalls[size];
        }
    };
}
