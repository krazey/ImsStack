#include "IMSMap.h"
#include "call/MtcCallController.h"
#include "IMtcService.h"
#include "IMtsService.h"
#include "JniAosService.h"
#include "JniConnector.h"
#include "JniConnectorFactory.h"
#include "JniMediaSession.h"
#include "JniMtcCall.h"
#include "JniMtcService.h"
#include "JniMtsService.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"

__IMS_TRACE_TAG_USER_DECL__("JNI");

JniConnectorFactory* JniConnectorFactory::s_pFactory = IMS_NULL;

struct Connectors
{
public:
    inline Connectors() :
            pAosServiceConnector(IMS_NULL),
            pMtcCallConnector(IMS_NULL),
            pMtcServiceConnector(IMS_NULL),
            pMtsServiceConnector(IMS_NULL),
            pMediaSessionConnector(IMS_NULL)
    {
        IMS_TRACE_D("+Connectors", 0, 0, 0);
    }

    inline JniConnector<IAosService, JniAosService>* GetAosServiceConnector()
    {
        if (pAosServiceConnector == IMS_NULL)
        {
            pAosServiceConnector = new JniConnector<IAosService, JniAosService>();
        }
        return pAosServiceConnector;
    }

    inline JniConnector<MtcCallController, JniMtcCall>* GetMtcCallConnector()
    {
        if (pMtcCallConnector == IMS_NULL)
        {
            pMtcCallConnector = new JniConnector<MtcCallController, JniMtcCall>();
        }
        return pMtcCallConnector;
    }

    inline JniConnector<IMtcService, JniMtcService>* GetMtcServiceConnector()
    {
        if (pMtcServiceConnector == IMS_NULL)
        {
            pMtcServiceConnector = new JniConnector<IMtcService, JniMtcService>();
        }
        return pMtcServiceConnector;
    }

    inline JniConnector<IMtsService, JniMtsService>* GetMtsServiceConnector()
    {
        if (pMtsServiceConnector == IMS_NULL)
        {
            pMtsServiceConnector = new JniConnector<IMtsService, JniMtsService>();
        }
        return pMtsServiceConnector;
    }

    inline JniConnector<IMediaManager, JniMediaSession>* GetMediaSessionConnector()
    {
        if (pMediaSessionConnector == IMS_NULL)
        {
            pMediaSessionConnector = new JniConnector<IMediaManager, JniMediaSession>();
        }
        return pMediaSessionConnector;
    }

    inline void Release()
    {
        delete pMtcCallConnector;
        pMtcCallConnector = IMS_NULL;

        delete pMtcServiceConnector;
        pMtcServiceConnector = IMS_NULL;

        delete pMtsServiceConnector;
        pMtsServiceConnector = IMS_NULL;

        delete pMediaSessionConnector;
        pMediaSessionConnector = IMS_NULL;

        delete pAosServiceConnector;
        pAosServiceConnector = IMS_NULL;
    }

private:
    JniConnector<IAosService, JniAosService>* pAosServiceConnector;
    JniConnector<MtcCallController, JniMtcCall>* pMtcCallConnector;
    JniConnector<IMtcService, JniMtcService>* pMtcServiceConnector;
    JniConnector<IMtsService, JniMtsService>* pMtsServiceConnector;
    JniConnector<IMediaManager, JniMediaSession>* pMediaSessionConnector;
};

class JniConnectorHolder
{
public:
    inline JniConnectorHolder() :
            m_objConnectorsMap(IMSMap<IMS_SINT32, Connectors*>()){};
    inline virtual ~JniConnectorHolder(){};

private:
    JniConnectorHolder(IN CONST JniConnectorHolder& objRHS);
    JniConnectorHolder& operator=(IN CONST JniConnectorHolder& objRHS);

public:
    inline Connectors* GetConnectors(IN IMS_SINT32 nSlotId)
    {
        if (m_objConnectorsMap.GetIndexOfKey(nSlotId) < 0)
        {
            m_objConnectorsMap.Add(nSlotId, new Connectors());
        }
        return m_objConnectorsMap.GetValue(nSlotId);
    }

    inline void DestroyConnectors(IN IMS_SINT32 nSlotId)
    {
        if (m_objConnectorsMap.GetIndexOfKey(nSlotId) < 0)
        {
            return;
        }

        Connectors* pConnectors = m_objConnectorsMap.GetValueAt(nSlotId);
        pConnectors->Release();
        delete pConnectors;
        m_objConnectorsMap.Remove(nSlotId);
    }

private:
    IMSMap<IMS_SINT32, Connectors*> m_objConnectorsMap;
};

PUBLIC
JniConnectorFactory::JniConnectorFactory() :
        m_pConnectorHolder(new JniConnectorHolder()),
        m_piLock(IMS_NULL)
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC
JniConnectorFactory::~JniConnectorFactory()
{
    MutexService::GetMutexService()->DestroyMutex(m_piLock);

    for (IMS_SINT32 i = 0; i < SystemConfig::GetMaxSimSlot(); i++)
    {
        ReleaseConnectors(i);
    }

    delete m_pConnectorHolder;
}

PUBLIC GLOBAL JniConnectorFactory* JniConnectorFactory::GetInstance()
{
    if (s_pFactory == IMS_NULL)
    {
        s_pFactory = new JniConnectorFactory();
    }
    return s_pFactory;
}

PUBLIC
void JniConnectorFactory::ReleaseConnectors(IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);
    m_pConnectorHolder->DestroyConnectors(nSlotId);
}

PUBLIC
JniConnector<IAosService, JniAosService>* JniConnectorFactory::GetAosServiceConnector(
        IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);
    IMS_TRACE_D("GetAosServiceConnector", 0, 0, 0);
    return m_pConnectorHolder->GetConnectors(nSlotId)->GetAosServiceConnector();
}

PUBLIC
JniConnector<MtcCallController, JniMtcCall>* JniConnectorFactory::GetMtcCallConnector(
        IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);
    IMS_TRACE_D("GetMtcCallConnector", 0, 0, 0);
    return m_pConnectorHolder->GetConnectors(nSlotId)->GetMtcCallConnector();
}

PUBLIC
JniConnector<IMtcService, JniMtcService>* JniConnectorFactory::GetMtcServiceConnector(
        IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);
    IMS_TRACE_D("GetMtcServiceConnector", 0, 0, 0);
    return m_pConnectorHolder->GetConnectors(nSlotId)->GetMtcServiceConnector();
}

PUBLIC
JniConnector<IMtsService, JniMtsService>* JniConnectorFactory::GetMtsServiceConnector(
        IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);
    IMS_TRACE_D("GetMtsServiceConnector", 0, 0, 0);
    return m_pConnectorHolder->GetConnectors(nSlotId)->GetMtsServiceConnector();
}

PUBLIC
JniConnector<IMediaManager, JniMediaSession>* JniConnectorFactory::GetMediaSessionConnector(
        IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);
    IMS_TRACE_D("GetMediaSessionConnector", 0, 0, 0);
    return m_pConnectorHolder->GetConnectors(nSlotId)->GetMediaSessionConnector();
}
