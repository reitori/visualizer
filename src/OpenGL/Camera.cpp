#include "Camera.h"

namespace viz{
    Camera::Camera(){
        data.projection = glm::mat4(1.0f);
        data.position = glm::vec3(0.0f, 0.0f, 0.0f);
        data.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        data.screenScale = glm::vec2(1.0f, 1.0f);
        data.zoom = 1.0f;
    }

    void Camera::resize(int width, int height){
        data.projection = glm::perspective(glm::radians(data.zoom * 45.0f), ((float)width * data.screenScale.x) / ((float)height * data.screenScale.y), 0.5f, 500.0f); 
    }

    void Camera::setPos(glm::vec3 pos) { 
        if(cameraLocked)
            return;
        data.position = pos; 
    }

    void Camera::displace(glm::vec3 disp) {
        if(cameraLocked) 
            return;
        data.position += disp; 
    }
    void Camera::rotate(glm::vec3 rot) { 
        if(cameraLocked)
            return;
        data.orientation = glm::quat(rot) * data.orientation; 
    }

    glm::mat4 Camera::getView() const{
        return glm::toMat4(data.orientation) * glm::translate(glm::mat4(1.0f), -data.position);
    }
}