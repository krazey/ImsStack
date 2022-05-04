/*
 * author : aromi.kwak
 * version : V1.0
 * date : 2015.12
 * brief : Create USSIEarlySession
 */

#ifndef USSI_EARLYSESSION_H_
#define USSI_EARLYSESSION_H_

#include "call/IMtcCall.h"

class USSIEarlySession
// TODO, MTC BUILD
// : public EarlySession
{
public:
    USSIEarlySession(IN ISession* pISession, IN IMtcCall* pSession);
    virtual ~USSIEarlySession();

protected:
    // TODO, MTC BUILD
    // virtual IMS_BOOL StateIDLE_Start(IN IMSMSG &objMsg);
    // virtual IMS_BOOL StateIDLE_IncomingSession(IN IMSMSG &objMsg);

    // virtual IMS_BOOL StateESTABLISHING_SSStarted(IN IMSMSG &objMsg);
    // virtual IMS_BOOL StateALERTING_UserAlert(IN IMSMSG &objMsg);
    // virtual IMS_BOOL StateALERTING_Accept(IN IMSMSG &objMsg);

    // virtual void SendStartedToListn();
    // virtual void AddUSSIHeaders(IN IMessage* pIMessage);
    // virtual IMSList<AString> FormRecvInfoHeader();
    // virtual IMSList<AString> FormAcceptHeader();
    // virtual IMSList<AString> FormContentTypeHeader();
    // virtual void LoadConfig();

    // Variable

protected:
private:
};
#endif /*  USSI_EARLYSESSION_H_ */
