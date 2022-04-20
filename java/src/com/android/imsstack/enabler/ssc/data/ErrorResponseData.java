package com.android.imsstack.enabler.ssc.data;

import com.android.imsstack.enabler.ssc.ESsType;

public class ErrorResponseData extends SscServiceData {
    protected int mErrorCode = -1;
    protected String mErrorPhrase   = null;

    public ErrorResponseData(int slotId, ESsType ssType, int eventNum, int transactionId, int state,
            int errorCode, String errorPhrase) {
        super(slotId, ssType, eventNum, transactionId, state);
        mErrorCode = errorCode;
        mErrorPhrase = errorPhrase;
    }

    public int getErrorCode() {
        return mErrorCode;
    }

    public String getErrorPhrase() {
        return mErrorPhrase;
    }

    public String toString() {
        return super.toString() + "\n" + "ErrorCode : " + mErrorCode + "\n" + "ErrorPhrase : "
                + mErrorPhrase + "\n";
    }
}
