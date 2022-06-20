#ifndef USSI_CONTROLLER_H_
#define USSI_CONTROLLER_H_

#include "IMSTypeDef.h"
#include "call/IMtcCallContext.h"
#include "ussi/UssiEventNotifier.h"
#include "ussi/UssiData.h"

class IMessage;
class ISipClientConnection;
class ISipMessage;
class ISipServerConnection;

class UssiController
{
public:
    UssiController(IN IMtcCallContext& objContext);
    virtual ~UssiController();

private:
    UssiController(IN CONST UssiController& objRHS);
    UssiController& operator=(IN CONST UssiController& objRHS);

public:
    static IMS_BOOL IsNetworkInitiatedUssi(IN IMessage* piMessage);

    IMS_BOOL HasValidXmlBodyForNetworkInitiatedUssi(IN IMessage* piMessage);
    IMS_BOOL IsByeForUssi(IN IMessage* piMessage);
    IMS_BOOL IsUssiInfoReceived(IN ISipServerConnection* piSipServerConnection);
    IMS_BOOL HasXmlBodyInInfo(IN ISipServerConnection* piSipServerConnection);

    UssiResult ParseUssiBodyAndCheckResult(
            IN ISipMessage* piSipMessage, IN IMS_SINT32 nReceivedMethod);

    IMS_RESULT FormStartUssiRequest(IN const AString& strTargetNumber);
    IMS_RESULT FormAcceptUssi();
    IMS_RESULT FormInfoRequest(IN ISipClientConnection* piSipClientConnection,
            IN const AString& strUssdString, IN UssiError eErrorCode);

    void SetNextActionByTerminateUssi();

    UssiResult GetLastResult() const;

private:
    IMS_RESULT FormHeadersForStartUssi(IN IMessage* piMessage);
    IMS_RESULT FormStartUssiBody(IN ISipMessage* piSipMessage, IN const AString& strTarget);

    IMS_RESULT SetRecvInfoHeader(IN IMessage* piMessage);
    IMS_RESULT SetAcceptHeader(IN IMessage* piMessage);

    IMS_RESULT FormHeadersForInfo(IN ISipClientConnection* piSipClientConnection);
    IMS_RESULT FormBodyForInfo(
            IN ISipMessage* piSipMessage, IN const AString& strUssdString, IN UssiError eErrorCode);

    UssiData* GetParsedUssiData(IN ISipMessage* piSipMessage);
    void NotifyUssiEvent(IN AString strUssdString, IN UssiModeType eType, IN UssiError eErrorCode);

    void SetUssiModeTypeForNetworkInitiated(IN UssiModeType eType);
    void SetLastResult(IN UssiResult objResult);

    IMS_BOOL IsUeInitiated();

private:
    IMtcCallContext& m_objContext;
    UssiEventNotifier m_objEventNotifier;
    UssiModeType m_eUssiModeType;
    UssiResult m_objLastResult;
};

#endif
