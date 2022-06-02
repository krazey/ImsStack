#ifndef JNI_CONNECTOR_H_
#define JNI_CONNECTOR_H_

#include "IMSTypeDef.h"

/*
    EnablerService : (mandatory) the class which receives message from Jni
        (ex. AosService / EabApp / IMtcService / IMtsService)
    JniService : (mandatory) the JniService
        (ex. JniMtcService / JniAosService/ JniEabService / JniMtsService)
*/
template <typename EnablerService, typename JniService>
class JniConnector
{
public:
    inline JniConnector();
    inline virtual ~JniConnector();

    inline void SetEnablerService(IN EnablerService* pEnablerService)
    { m_pEnablerService = pEnablerService; }

    inline void SetJniService(IN JniService* pJniService)
    { m_pJniService = pJniService; }

    inline EnablerService* GetEnablerService();

    //EnablerService calls this to get JniService.
    inline JniService* GetJniService();

private:
    EnablerService* m_pEnablerService;
    JniService* m_pJniService;
};

PUBLIC
template <typename EnablerService, typename JniService> inline
JniConnector<EnablerService, JniService>::JniConnector() :
        m_pEnablerService(IMS_NULL),
        m_pJniService(IMS_NULL)
{
}

PUBLIC VIRTUAL
template <typename EnablerService, typename JniService> inline
JniConnector<EnablerService, JniService>::~JniConnector()
{
}

PUBLIC
template <typename EnablerService, typename JniService> inline
EnablerService* JniConnector<EnablerService, JniService>::GetEnablerService()
{
    return m_pEnablerService;
}

PUBLIC VIRTUAL
template <typename EnablerService, typename JniService> inline
JniService* JniConnector<EnablerService, JniService>::
        GetJniService()
{
    return m_pJniService;
}

#endif
