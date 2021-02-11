#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters
        this->cameraFrontDirection = glm::normalize(this->cameraTarget - this->cameraPosition);
        this->cameraRightDirection = -glm::normalize(glm::cross(this->cameraUpDirection, this->cameraFrontDirection));
        this->cameraUpDirection = glm::cross(this->cameraRightDirection, this->cameraFrontDirection);
        this->firstCameraUp = cameraUp;
    }

    glm::vec3 Camera::getPosition() {
        return this->cameraPosition;
    }

    void Camera::resizeGround() {
        if (cameraPosition.y < 0.5)
            cameraPosition.y = 0.5;
        if (cameraPosition.y > 35)
            cameraPosition.y = 35;
        if (cameraPosition.x > 85)
            cameraPosition.x = 85;
        if (cameraPosition.x < -85)
            cameraPosition.x = -85;
        if (cameraPosition.z > 85)
            cameraPosition.z = 85;
        if (cameraPosition.z < -85)
            cameraPosition.z = -85;
    }
    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        resizeGround();
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        if (direction == MOVE_FORWARD) {
            this->cameraPosition = cameraPosition + this->cameraFrontDirection * speed;
            this->cameraTarget = this->cameraPosition + this->cameraFrontDirection;//punct +vector=>punct deplasat pe vector
        }
        if (direction == MOVE_BACKWARD) {
            this->cameraPosition = cameraPosition - this->cameraFrontDirection * speed;
            this->cameraTarget = this->cameraPosition + this->cameraFrontDirection;
        }
        if (direction == MOVE_RIGHT) {
            this->cameraPosition = cameraPosition - this->cameraRightDirection * speed;
            this->cameraTarget = this->cameraPosition + this->cameraFrontDirection;
        }
        if (direction == MOVE_LEFT) {
            this->cameraPosition = cameraPosition + this->cameraRightDirection * speed;
            this->cameraTarget = this->cameraPosition + this->cameraFrontDirection;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::mat4 rotateMatrix = glm::yawPitchRoll(yaw, pitch, 0.0f);//noua matrice de orientare
        glm::vec4 vecFront = rotateMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

        cameraFrontDirection.x = vecFront.x;
        cameraFrontDirection.y = vecFront.y;
        cameraFrontDirection.z = vecFront.z;
        cameraFrontDirection = glm::normalize(cameraFrontDirection);
        cameraRightDirection = glm::normalize(glm::cross(firstCameraUp, cameraFrontDirection));
        cameraUpDirection = glm::cross(cameraFrontDirection, cameraRightDirection);
        cameraTarget = this->cameraPosition + this->cameraFrontDirection;

    }
}