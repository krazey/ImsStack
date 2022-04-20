package com.android.imsstack.enabler.ssc;

import android.os.Handler;

public class SscTransactionFactory {
    public SscTransaction getSscTransaction(int slotId, Handler callbackHandler) {
        return new SscTransaction(slotId, callbackHandler);
    }
}