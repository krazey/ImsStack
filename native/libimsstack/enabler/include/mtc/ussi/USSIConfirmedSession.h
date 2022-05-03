/*
 * author : aromi.kwak@
 * version : 1.0
 * date :
 * brief : Create USSIConfirmedSession
 */

#ifndef USSI_CONFIRMEDSESSION_H_
#define USSI_CONFIRMEDSESSION_H_

#include "call/IMtcCall.h"

class ISipConnection;
class ISipClientConnection;
class ISipServerConnection;

class USSIConfirmedSession
// TODO, MTC BUILD
// : public ConfirmedSession
{
public:
    USSIConfirmedSession(IN IMtcCall* pSession);
    virtual ~USSIConfirmedSession();

protected:
    // TODO, MTC BUILD
    // public:
    //     virtual IMS_BOOL Control(IN IMS_UINT32 nCmdType,
    //             IN IMS_UINTP nInParam, OUT IMS_UINTP *pnOutParam);
    //     virtual void SetUSSType(IN IMS_UINT32 nUSSType);

    // protected:
    //     virtual IMS_BOOL OnMessage(IN IMSMSG &objMSG);
    //     virtual IMS_BOOL IsUSSIInfoReceived(IN ISipMessage* pSIPMessage);

    //     virtual IMS_BOOL StateCONVERSATION_SS_Terminated(IN IMSMSG &objMsg);
    //     virtual IMS_BOOL StateXXX_SS_TransactionReceived(IN IMSMSG &objMsg);
    //     virtual IMS_BOOL StateXXX_SS_SendTransaction(IN IMSMSG &objMsg);

    //     virtual void LoadConfig();

    //     virtual void SendUSSITransactionToListn(IN ISipMessage* pISIPMessage);

    //     virtual void ClientConnection_NotifyResponse(IN ISipClientConnection *piSCC,
    //             IN ISipClientConnection * piForkedSCC /* = IMS_NULL */);
    //     virtual void Error_NotifyError(IN ISipConnection *piSC, IN IMS_SINT32 nCode,
    //             IN const AString &strMessage);
    //     virtual void SendTransactionWithErrorCode(IN IMS_UINT32 nErrorCode);

    // private:
    //     virtual void SendTransactionResponse(IN IMS_UINT32 nResponseCode,
    //             IN const AString& strPhrase = AString::ConstEmpty());
    //     virtual IMS_BOOL SetUSSIBody(IN ISipMessage* pISIPMessage, IN const AString& strUSSDStr,
    //             IN IMS_UINT32 nErrorCode);

    // Variable

public:
    enum
    {
        CMD_HANDLING_RESULT = 0
    };

    enum
    {
        // TODO, MTC BUILD
        USSI_CONFIRMED_SEND_ERROR_INFO = 0
        // USSI_CONFIRMED_SEND_ERROR_INFO = CONFIRMED_COM_DEFAULT + 1
    };

protected:
    ISipClientConnection* m_pISIPClientConnection;
    ISipServerConnection* m_pISIPServerConnection;

    IMS_UINT32 m_nUSSType;
};

#endif /*  USSI_CONFIRMEDSESSION_H_ */
