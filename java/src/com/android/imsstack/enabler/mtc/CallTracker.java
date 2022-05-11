/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20131015    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.enabler.mtc;

public interface CallTracker {
    /**
     * VoLTE/VT call state.
     */
    int CALL_STATE_IDLE = 0;
    int CALL_STATE_RINGING = 1;
    int CALL_STATE_OFFHOOK = 2;
    int CALL_STATE_RINGBACK = 3;

    /**
     * It will be used in {@code event} when calling {@link updateCallState}.
     * The event name in {@link IUMtcCall} can be also used in {@code event}.
     */
    int E_CALL_EVENT_NONE = 91;
    int E_CALL_EVENT_ORIGINATING = 92;
    int E_CALL_EVENT_CONNECTED = 93;
    int E_CALL_EVENT_END = 94;
    int E_CALL_EVENT_SRVCC = 95;

    /**
     * Call related event for creation / destruction
     */
    int CALL_EVENT_CREATE = 1;
    int CALL_EVENT_ESTABLISHING = 2;
    int CALL_EVENT_RINGING = 3;
    int CALL_EVENT_ACCEPT = 4;
    int CALL_EVENT_ESTABLISHED = 5;
    int CALL_EVENT_UPDATED = 6;
    int CALL_EVENT_TERMINATING = 7;
    int CALL_EVENT_TERMINATED = 8;
    int CALL_EVENT_DESTROY = 9;
    int CALL_EVENT_INCOMING_RECEIVED = 10;

    /** Creates new call identifier */
    default public String createCallId() {
        return null;
    }

    /** Returns the count of active(OFFHOOK) call. */
    public int getActiveCalls();
    public void updateCallState(Object call, int event, Object extraInfo);
}
