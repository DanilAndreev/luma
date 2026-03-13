#pragma once

enum class KeyID {
    None,
    Q,
    W,
    E,
    A,
    S,
    D,
    Z,
    X,
    C,
    Left,
    Up,
    Down,
    Right,
    Count,
};

class InputDeviceState {
public:
    bool Pressed(KeyID key) noexcept;

public:
    void Signal(KeyID key, bool isPressed) noexcept;

private:
    bool m_Key[static_cast<size_t>(KeyID::Count)] = {};
};
