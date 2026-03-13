#include "Camera.h"

DirectX::XMFLOAT2 Camera::GetGimbal() const {
    return m_Gimbal;
}

DirectX::XMFLOAT3 Camera::GetPosition() const {
    return m_Position;
}

void Camera::MoveTo(const DirectX::XMFLOAT3& position) {
    m_Position = position;
}

void Camera::MoveBy(const DirectX::XMFLOAT3& delta) {
//    glm::vec4 pos = glm::vec4(this->translate.x, this->translate.y, this->translate.z, 0);
//    glm::mat4 toCameraSpace = this->viewTransform();
//    glm::mat4 fromCameraSpace = glm::inverse(toCameraSpace);

//    glm::mat4 I = fromCameraSpace * toCameraSpace ;
//
//    std::cout << "------" << std::endl;
//    for (int y = 0; y < 4; y++) {
//        for(int x = 0; x < 4; x++) {
//            std::cout << I[x][y] << " ";
//        }
//        std::cout << std::endl;
//    }

//    pos = fromCameraSpace * pos;
//    pos += glm::vec4(iDelta.x, iDelta.y, iDelta.z, 1.0f);
//    pos = toCameraSpace * pos;
//    this->translate = glm::vec3(pos.x, pos.y, pos.z);


    // this->position += iDelta.z * this->front;
    // this->position += glm::normalize(glm::cross(this->front, this->worldUp)) * iDelta.x;

    using namespace DirectX;

    XMVECTOR front = XMLoadFloat3(&m_Front);
    XMVECTOR up = XMLoadFloat3(&m_WorldUp);
    XMVECTOR pos = XMLoadFloat3(&m_Position);

    pos = XMVectorAdd(pos, XMVectorScale(front, delta.z));
    //m_Position +=  XMVector3Normalize(XMVector3Cross(front, up)) * delta.x;
    XMStoreFloat3(&m_Position, XMVectorAdd(pos, XMVectorScale(XMVector3Normalize(XMVector3Cross(front, up)), delta.x)));

}

void Camera::RotateBy(const DirectX::XMFLOAT2& angles) {
    m_Gimbal = {m_Gimbal.x + angles.x, m_Gimbal.y + angles.y};
    if (m_Gimbal.x > 89.0f)
        m_Gimbal.x = 89.0f;
    if (m_Gimbal.x < -89.0f)
        m_Gimbal.x = -89.0f;

    CalculateFront();
}

DirectX::XMFLOAT4X4 Camera::ViewTransform() const {
    using namespace DirectX;
    XMVECTOR pos = XMLoadFloat3(&m_Position);
    XMVECTOR front = XMLoadFloat3(&m_Front);
    XMVECTOR up = XMLoadFloat3(&m_WorldUp);

    auto matrix = XMMatrixLookAtLH(pos, XMVectorAdd(pos, front), up);
    XMFLOAT4X4 outMatrix;
    XMStoreFloat4x4(&outMatrix, matrix);
    return outMatrix;
}

void Camera::CalculateFront() {
    using namespace DirectX;
    m_Front.x = cos(XMConvertToRadians(m_Gimbal.x)) * cos(XMConvertToRadians(m_Gimbal.y));
    m_Front.y = sin(XMConvertToRadians(m_Gimbal.x));
    m_Front.z = cos(XMConvertToRadians(m_Gimbal.x)) * sin(XMConvertToRadians(m_Gimbal.y));
}
