#pragma once

class Camera {
public:
    DirectX::XMFLOAT2 GetGimbal() const;
    DirectX::XMFLOAT3 GetPosition() const;

public:
    void MoveTo(const DirectX::XMFLOAT3 &position);
    void MoveBy(const DirectX::XMFLOAT3 &delta);
    void RotateBy(const DirectX::XMFLOAT2 &angles);

public:
    DirectX::XMFLOAT4X4 ViewTransform() const;

protected:
    void CalculateFront();

protected:
    DirectX::XMFLOAT2 m_Gimbal = {0.0f, -90.0f}; /// pitch, yaw
    DirectX::XMFLOAT3 m_Position = {0.0f, 0.0f, 3.0f};
    DirectX::XMFLOAT3 m_Front = {0.0f, 0.0f, -1.0f};
    DirectX::XMFLOAT3 m_WorldUp = {0.0f, 1.0f, 0.0f};
};
