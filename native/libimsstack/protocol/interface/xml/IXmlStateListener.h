#ifndef INTERFACE_XML_STATE_LISTENER_H_
#define INTERFACE_XML_STATE_LISTENER_H_

#include "ImsTypeDef.h"

class IXmlStateListener
{
public:
    /**
     * @brief Notifies the application that the state of XML transaction provider is changed.
     */
    virtual void XmlState_NotifyStateChanged() = 0;
};

#endif
