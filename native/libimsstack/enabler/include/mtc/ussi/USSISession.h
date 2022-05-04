/*
 * author :
 * version :
 * date :
 * brief :
 */

#ifndef USSI_SESSION_H_
#define USSI_SESSION_H_

#include "ussi/UssiDataParser.h"
#include "IMtcApp.h"
#include "IMtcService.h"

class USSISession
// TODO, MTC BUILD
// : public UCSession
{
public:
    // TODO, MTC BUILD
    USSISession(
            IN IMtcApp* pApp, IN IMtcService* pService, IN AString aStrUIKey, IN IMS_UINTP nIMSKey);
    // USSISession(IN IMtcApp* pApp, IN IMtcService* pService, IN AString aStrUIKey,
    //         IN IMS_UINTP nIMSKey, IN IUCSessionListener* pListener);
    virtual ~USSISession();

    // TODO, MTC BUILD
    //     virtual AString GetToURI(IN ICoreService* /*pIService*/, IN ISession* /*pISession*/,
    //             IN const AString &aStrNumber);

    //     virtual IMS_BOOL Control(
    //             IN IMS_UINT32 nCmdType, IN IMS_UINTP nInParam, OUT IMS_UINTP *pnOutParam);
    //     virtual void Confirmed_TransactionReceived(IN IMS_UINTP nParam);

    // protected:
    //     virtual EarlySession* EarlySession_CreateCom(IN ISession* pISession);
    //     virtual ConfirmedSession* ConfirmedSession_CreateCom(IN ISession* pISession);
    //     virtual IMS_BOOL StateIDLE_Start(IN IMSMSG &objMsg);
    //     virtual IMS_BOOL StateIDLE_IncomingSession(IN IMSMSG &objMsg);
    //     virtual IMS_BOOL StateRINGING_Started(IN IMSMSG &objMsg);
    //     virtual IMS_BOOL StateCONVERSATION_Terminate(IN IMSMSG &objMsg);
    //     virtual IMS_BOOL StateCONVERSATION_SendTransaction(IN IMSMSG &objMsg);
    //     virtual IMS_BOOL StateXXX_SS_UpdateReceived(IN IMSMSG &objMsg);

    //     virtual IMS_BOOL SetUSSIBody(IN ISipMessage* pISIPMessage, IN const AString& strUSSDStr,
    //             IN IMS_BOOL bMultiPart = IMS_TRUE);
    //     virtual IMS_BOOL SendUSSIResultToUI(IN USSDDataParser* pUSSDData);

    //     virtual void LoadConfig();

    // private:
    //     USSDDataParser* GetParsedUSSIData(IN ISipMessage* pISIPMessage);
    //     IMS_BOOL IsUEInitiated();
    //     IMS_BOOL IsPreviousRequestByeMessage();

public:
    enum
    {
        USSD_MODE_ERROR = -1,
        USSD_MODE_NOTIFY = 0,
        USSD_MODE_REQUEST = 1,

        // Google_IMS_IF :: USSD {
        INFO_TYPE_USSD = 11,  // int & String
        // Google_IMS_IF :: USSD }
    };

    enum
    {
        CMD_CHECK_INCOMING_USSI_DATA = 0
    };
};
#endif  // USSI_SESSION_H_
