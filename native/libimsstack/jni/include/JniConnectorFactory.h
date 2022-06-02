#ifndef JNI_CONNECTOR_FACTORY_H_
#define JNI_CONNECTOR_FACTORY_H_

#include "IMSTypeDef.h"
#include "IMutex.h"
#include "JniConnector.h"

class IAosService;
class MtcCallController;
class IMtcService;
class IMtsService;
class JniAosService;
class JniConnectorHolder;
class JniMediaSession;
class JniMtcCall;
class JniMtcService;
class JniMtsService;
class IMediaManager;

class JniConnectorFactory
{
private:
    JniConnectorFactory();

public:
    ~JniConnectorFactory();

public:
    static JniConnectorFactory* GetInstance();
    void ReleaseConnectors(IN IMS_SINT32 nSlotId);

    JniConnector<IAosService, JniAosService>* GetAosServiceConnector(IN IMS_SINT32 nSlotId);
    JniConnector<MtcCallController, JniMtcCall>* GetMtcCallConnector(IN IMS_SINT32 nSlotId);
    JniConnector<IMtcService, JniMtcService>* GetMtcServiceConnector(IN IMS_SINT32 nSlotId);
    JniConnector<IMtsService, JniMtsService>* GetMtsServiceConnector(IN IMS_SINT32 nSlotId);
    JniConnector<IMediaManager, JniMediaSession>* GetMediaSessionConnector(IN IMS_SINT32 nSlotId);

private:
    static JniConnectorFactory* s_pFactory;
    JniConnectorHolder* m_pConnectorHolder;
    IMutex* m_piLock;
};

#endif
