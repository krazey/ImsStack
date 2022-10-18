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

package com.android.imsstack.enabler.mtc.dialogs;

import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.util.ImsLog;

import java.util.ArrayList;

public class DialogsInfo implements Parcelable {

    public class Dialog{
        public String       id = "";

        public int          state = DIALOG_STATE_IDLE;
        public int          reason = CallReasonInfo.CODE_NONE;
        public int          code = 0;

        public String       localName = "";
        public String       localNumber = "";

        public String       remoteName = "";
        public String       remoteNumber = "";

        public boolean      initiator = true;
        public boolean      isConf = false;
        public boolean      enablePull = false;

        public MediaInfo    mediaInfo;
    };

    public ArrayList<Dialog>    dialogs = new ArrayList<Dialog>();

    // state
    public static final int DIALOG_STATE_IDLE           = 0;
    public static final int DIALOG_STATE_TRYING         = 1;
    public static final int DIALOG_STATE_PROCEEDING     = 2;
    public static final int DIALOG_STATE_EARLY          = 3;
    public static final int DIALOG_STATE_CONFIRMED      = 4;
    public static final int DIALOG_STATE_TERMINATED     = 5;

    public static final int DIALOG_STATE_ONHOLD         = 6;


    //------------------------------------------------------------------------------------------//
    public DialogsInfo() {
        ImsLog.i("");
    }

    public DialogsInfo(DialogsInfo dialogsInfo) {

        int dialogs_num = dialogsInfo.dialogs.size();
        ImsLog.i("[" + dialogs_num + "]");

        for (int index = 0; index < dialogs_num; index++) {
            Dialog dialog = dialogsInfo.dialogs.get(index);
            addDialog(dialog);
        }

    }

    public DialogsInfo(Parcel source) {
        readFromParcel(source);
    }

    public void logIn(String tag) {

        ImsLog.i(tag + " - Dialogs : size[" + dialogs.size() + "]");

        for (int index = 0; index < dialogs.size(); index++) {

            Dialog dialog = dialogs.get(index);
            ImsLog.i("[" + index + "]"
                    + " id : " + dialog.id
                    + " state : " + dialog.state
                    + " reason : " + dialog.reason
                    + " code : " + dialog.code
                    + " localName : " + ImsLog.hiddenString(dialog.localName)
                    + " localNumber : " + ImsLog.hiddenString(dialog.localNumber)
                    + " remoteName : " + ImsLog.hiddenString(dialog.remoteName)
                    + " remoteNumber : " + ImsLog.hiddenString(dialog.remoteNumber)
                    + " initiator : " + dialog.initiator
                    + " isConf : " + dialog.isConf
                    + " enablePull : " + dialog.enablePull
                     );
             dialog.mediaInfo.logIn("Dialog");
        }

    }

    public void logIn(DialogsInfo dialogsInfo) {

        ImsLog.i("DialogSize[" + dialogsInfo.dialogs.size() + "]");

        for (int index = 0; index < dialogsInfo.dialogs.size(); index++) {

            Dialog dialog = dialogsInfo.dialogs.get(index);
            ImsLog.i("[" + index + "]"
                    + " id : " + dialog.id
                    + " state : " + dialog.state
                    + " reason : " + dialog.reason
                    + " code : " + dialog.code
                    + " localName : " + ImsLog.hiddenString(dialog.localName)
                    + " localNumber : " + ImsLog.hiddenString(dialog.localNumber)
                    + " remoteName : " + ImsLog.hiddenString(dialog.remoteName)
                    + " remoteNumber : " + ImsLog.hiddenString(dialog.remoteNumber)
                    + " initiator : " + dialog.initiator
                    + " isConf : " + dialog.isConf
                    + " enablePull : " + dialog.enablePull
                     );
             dialog.mediaInfo.logIn("Dialog");
        }

    }

    public void logInDialog(Dialog  dialog) {

        ImsLog.i(" id : " + dialog.id
                    + " state : " + dialog.state
                    + " reason : " + dialog.reason
                    + " code : " + dialog.code
                    + " localName : " + ImsLog.hiddenString(dialog.localName)
                    + " localNumber : " + ImsLog.hiddenString(dialog.localNumber)
                    + " remoteName : " + ImsLog.hiddenString(dialog.remoteName)
                    + " remoteNumber : " + ImsLog.hiddenString(dialog.remoteNumber)
                    + " initiator : " + dialog.initiator
                    + " isConf : " + dialog.isConf
                    + " enablePull : " + dialog.enablePull
                 );
         dialog.mediaInfo.logIn("Dialog");

    }

    public int getSize() {

        ImsLog.i("getSize : [" + dialogs.size() + "]");
        return dialogs.size();
    }

    public Dialog getDialog(int index) {

        ImsLog.i("getDialog : [" + index + "]");
        return dialogs.get(index);
    }

    public void addDialog(Dialog _dialog) {

        logInDialog(_dialog);

        Dialog dialog = new Dialog();
        dialog.id = _dialog.id;
        dialog.state = _dialog.state;
        dialog.reason = _dialog.reason;
        dialog.code = _dialog.code;
        dialog.localName = _dialog.localName;
        dialog.localNumber = _dialog.localNumber;
        dialog.remoteName = _dialog.remoteName;
        dialog.remoteNumber = _dialog.remoteNumber;
        dialog.initiator = _dialog.initiator;
        dialog.isConf = _dialog.isConf;
        dialog.enablePull = _dialog.enablePull;
        dialog.mediaInfo = new MediaInfo(_dialog.mediaInfo);

        dialogs.add(dialog);
        logIn("addDialog");
    }

    public void readFromParcel(Parcel source) {

        int dialogs_num = source.readInt();
        ImsLog.i("dialogs[" + dialogs_num + "]");

        for (int index = 0; index < dialogs_num; index++) {

            Dialog dialog = new Dialog();
            dialog.id = source.readString();
            dialog.state = source.readInt();
            dialog.reason = source.readInt();
            dialog.code = source.readInt();
            dialog.localName = source.readString();
            dialog.localNumber = source.readString();
            dialog.remoteName = source.readString();
            dialog.remoteNumber = source.readString();
            if (source.readInt() == 1) {
                dialog.initiator = true;
            } else {
                dialog.initiator = false;
            }
            if (source.readInt() == 1) {
                dialog.isConf = true;
            } else {
                dialog.isConf = false;
            }
            if (source.readInt() == 1) {
                dialog.enablePull = true;
            } else {
                dialog.enablePull = false;
            }
            dialog.mediaInfo = new MediaInfo(source);

            dialogs.add(dialog);
        }
        logIn("read");
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(dialogs.size());
        for (int index = 0; index < dialogs.size(); index++) {
            Dialog dialog = dialogs.get(index);

            dest.writeString(dialog.id);
            dest.writeInt(dialog.state);
            dest.writeInt(dialog.reason);
            dest.writeInt(dialog.code);
            dest.writeString(dialog.localName);
            dest.writeString(dialog.localNumber);
            dest.writeString(dialog.remoteName);
            dest.writeString(dialog.remoteNumber);
            if (dialog.initiator) {
                dest.writeInt(1);
            } else {
                dest.writeInt(0);
            }
            if (dialog.isConf) {
                dest.writeInt(1);
            } else {
                dest.writeInt(0);
            }
            if (dialog.enablePull) {
                dest.writeInt(1);
            } else {
                dest.writeInt(0);
            }
            dialog.mediaInfo.writeToParcel(dest, flags);
        }

        logIn("write");
    }

    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<DialogsInfo> CREATOR
            = new Parcelable.Creator<DialogsInfo>() {
        public DialogsInfo createFromParcel(Parcel source) {
            try {
                return new DialogsInfo(source);
            } catch (Exception e) {
                ImsLog.e("createFromParcel() :: Exception occurred when creating DialogsInfo from parcel", e);
            }
            return null;
        }

        public DialogsInfo[] newArray(int size) {
            return new DialogsInfo[size];
        }
    };

};
