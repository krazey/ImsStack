/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100323  hwangoo.park@             Created
    </table>

    Description
     This class defines an extended SIP dialog.
    It has a dialog usage to support a multiple dialog usage in SIP.
*/

#ifndef _SIP_DIALOG_EX_H_
#define _SIP_DIALOG_EX_H_

#include "SIPDialogBase.h"

class SIPDialogUsage;

class SIPDialogEx : public SIPDialogBase
{
public:
    explicit SIPDialogEx(IN SIPDialogState* pDState_);
    SIPDialogEx(IN CONST SIPDialogEx& objRHS);
    virtual ~SIPDialogEx();

private:
    SIPDialogEx();

public:
    SIPDialogEx& operator=(IN CONST SIPDialogEx& objRHS);

public:
    // For an initial requests
    IMS_BOOL InitDialog(IN CONST SipMethod& objMethod);
    // For a dialog request or incoming requests
    IMS_BOOL InitDialog(IN CONST SIPMessageInfo& objMInfo);
    IMS_BOOL InitDialogWithDelay(IN CONST SIPMessageInfo& objMInfo);
    IMS_BOOL CompareTo(IN CONST SIPMessageInfo& objMInfo) const;
    IMS_BOOL Equals(IN SIPDialogEx* pDialogEx) const;
    IMS_BOOL IsInviteUsage() const;
    IMS_BOOL IsDialogTerminated() const;
    void TerminateDialogUsage();
    IMS_SINT32 UpdateDialogDetails(IN CONST SIPMessageInfo& objMInfo);

    static SIPDialogEx* CreateDialog(IN CONST SipMethod& objMethod);
    static SIPDialogEx* CreateDialog(IN SIPDialogState* pDState, IN CONST SipMethod& objMethod);
    static SIPDialogEx* CreateDialog(IN SIPDialogState* pDState, IN CONST SIPMessageInfo& objMInfo);

protected:
    // SIPDialogBase class
    virtual IMS_BOOL OnInit();
    virtual void OnTerminated();
    virtual IMS_SINT32 OnUpdateDialogDetails(IN CONST SIPMessageInfo& objMInfo,
            IN IMS_SINT32 nUsage, IN IMS_SINT32 nAction, IN IMS_SINT32 nTrigger);

private:
    IMS_BOOL bFlag_PermanentDialog;
    IMS_BOOL bFlag_InitWithDelay;
    IMS_BOOL bFlag_IsDialogTerminated;

    SIPDialogUsage* pDialogUsage;
};

#endif  // _SIP_DIALOG_EX_H_
