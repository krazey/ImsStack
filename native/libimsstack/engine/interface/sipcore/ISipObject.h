#ifndef _INTERFACE_SIP_OBJECT_H_
#define _INTERFACE_SIP_OBJECT_H_

/**
 * @brief This class provides a base interface for the major classes of J180 layer.
 */
class ISIPObject
{
public:
    /**
     * @brief Destroys the all the resources for SIP object.
     */
    virtual void Destroy() = 0;
};

#endif // _INTERFACE_SIP_OBJECT_H_
