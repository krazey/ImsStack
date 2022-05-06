/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100317  hwangoo.park@             Created
    </table>

    Description
     This class defines a shared dialog state.
    SIPDialogState class MUST have this class as its member.
*/

#ifndef _SIP_DIALOG_SHARED_STATE_H_
#define _SIP_DIALOG_SHARED_STATE_H_

#include "IMSList.h"
#include "SIPMessageInfo.h"

class SIPDialogEx;

class SIPDialogSharedState
{
public:
    SIPDialogSharedState();
    ~SIPDialogSharedState();

private:
    SIPDialogSharedState(IN CONST SIPDialogSharedState& objRHS);
    SIPDialogSharedState& operator=(IN CONST SIPDialogSharedState& objRHS);

private:
    IMS_BOOL AddDialog(IN SIPDialogEx* pDialogEx);
    void RemoveDialog(IN SIPDialogEx* pDialogEx);
    SIPDialogEx* GetDialog(IN CONST SIPMessageInfo& objMInfo);
    IMS_BOOL HasMultipleDialogUsages() const;
    inline IMS_BOOL IsShared() const { return (nSharedState == SHARED_STATE_ACTIVE); }

private:
    friend class SIPDialogState;

    enum
    {
        SHARED_STATE_INIT = 0,
        SHARED_STATE_ACTIVE,
        SHARED_STATE_TERMINATED
    };

    IMS_SINT32 nSharedState;

    // References of dialog state
    IMSList<SIPDialogEx*> objDialogExs;
};

#endif  // _SIP_DIALOG_SHARED_STATE_H_
