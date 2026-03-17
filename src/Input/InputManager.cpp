#include "InputManager.h"

#include "InputDeviceState.h"

void InputManager::Tick() noexcept {
    static clock_t prev = clock();

    constexpr float speed = 0.003f;

    clock_t curTime = clock();
    clock_t delta = curTime - prev;



    DirectX::XMFLOAT3 movementDelta{};
    DirectX::XMFLOAT2 rotationDelta{};
    float deltaTime = static_cast<float>(delta) * speed;
    if (g_Input.Pressed(KeyID::A)) {
        movementDelta.x += 1;
    }
    if (g_Input.Pressed(KeyID::D)) {
        movementDelta.x += -1;
    }
    if (g_Input.Pressed(KeyID::W)) {
        movementDelta.z += 1;
    }
    if (g_Input.Pressed(KeyID::S)) {
        movementDelta.z += -1;
    }
    if (g_Input.Pressed(KeyID::Up)) {
        rotationDelta.x += 10;
    }
    if (g_Input.Pressed(KeyID::Down)) {
        rotationDelta.x += -10;
    }
    if (g_Input.Pressed(KeyID::Left)) {
        rotationDelta.y += 10;
    }
    if (g_Input.Pressed(KeyID::Right)) {
        rotationDelta.y += -10;
    }
    if (movementDelta.x != 0 || movementDelta.y != 0 || movementDelta.z != 0) {
        g_Cam.MoveBy({movementDelta.x * deltaTime, movementDelta.y * deltaTime, movementDelta.z * deltaTime});
    }
    g_Cam.RotateBy({rotationDelta.x * deltaTime, rotationDelta.y * deltaTime});

    prev = curTime;
}
