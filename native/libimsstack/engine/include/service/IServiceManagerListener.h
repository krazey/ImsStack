/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100609  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_SERVICE_MANAGER_LISTENER_H_
#define _INTERFACE_SERVICE_MANAGER_LISTENER_H_

class Service;

class IServiceManagerListener
{
public:
    virtual void ServiceClosed(IN Service* pService) = 0;
};

#endif  // _INTERFACE_SERVICE_MANAGER_LISTENER_H_
