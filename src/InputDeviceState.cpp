#include "InputDeviceState.h"


bool InputDeviceState::Pressed(KeyID key) noexcept {
    assert(key < KeyID::Count);
    return m_Key[static_cast<size_t>(key)];
}

void InputDeviceState::Signal(KeyID key, bool isPressed) noexcept {
    assert(key < KeyID::Count);
    if (key == KeyID::None)
        return;
    m_Key[static_cast<size_t>(key)] = isPressed;
}
