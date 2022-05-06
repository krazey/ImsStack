#ifndef _INTERFACE_REG_BINDING_STATE_LISTENER_H_
#define _INTERFACE_REG_BINDING_STATE_LISTENER_H_

/**
 * @brief This class provides an interface to notify the caller capability changes
 *        from the registration binding.
 */
class IRegBindingStateListener
{
public:
    /**
     * @brief Notifies the application when the registration binding is updated.
     *
     * For example, if the caller capability of the specific core service was changed,
     * it can be invoked.
     */
    virtual void RegBindingState_CallerCapabilityChanged() = 0;
};

#endif  // _INTERFACE_REG_BINDING_STATE_LISTENER_H_
