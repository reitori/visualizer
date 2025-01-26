#include "Camera.h"

namespace viz{
    Camera::Camera(){
        data.projection = glm::mat4(1.0f);
        data.orientation = glm::mat4(1.0f);
        data.position = glm::vec3(0.0f, 0.0f, 1.0f);
        data.front = glm::vec3(0.0f, 0.0f, -1.0f);
        data.up = glm::vec3(0.0f, 1.0f, 0.0f);
        data.right = glm::vec3(1.0f, 0.0f, 0.0f);
        data.screenScale = glm::vec2(1.0f, 1.0f);
        data.screenSize = glm::vec2(1.0f, 1.0f);
        data.zoom = 1.0f;
    }

    void Camera::resize(int width, int height){
        data.screenSize.x = width; data.screenSize.y = height;
        calcProj();
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
    void Camera::setRotation(float rightAngle, float upAngle){
        if(cameraLocked)
            return;

        float upAngleRad = viz_TO_RADIANS(upAngle);
        float rightAngleRad = viz_TO_RADIANS(rightAngle);

        float x = cos(upAngleRad) * sin(rightAngleRad);
        float y = sin(upAngleRad);
        float z = cos(upAngleRad) * cos(rightAngleRad);

        data.front = glm::vec3(x, y, z);
        updateOrientation();
    }
    void Camera::rotate(float dispRightAngle, float dispUpAngle) { 
        if(cameraLocked)
            return;

        float upAngleRad = viz_TO_RADIANS(dispRightAngle);
        float rightAngleRad = viz_TO_RADIANS(dispUpAngle);

        glm::vec3 x = (float)(cos(upAngleRad) * sin(rightAngleRad)) * data.right;
        glm::vec3 y = (float)sin(upAngleRad) * data.up;
        glm::vec3 z = (float)(cos(upAngleRad) * cos(rightAngleRad)) * data.front;

        glm::vec3 front = glm::normalize(x+y+z);

        if(glm::dot(front, glm::vec3(0.0f, 1.0f, 0.0f)) > 0.999f)
            data.front = glm::vec3(0.0f, 1.0f, 0.0f);
        else{
            data.front = front;
            updateOrientation();
        }
    }

    void Camera::addScroll(float scroll){
        if(cameraLocked)
            return;
        data.screenScale += scroll;
        calcProj();
    }

    glm::mat4 Camera::getView() const{
        return data.orientation * glm::translate(glm::mat4(1.0f), -data.position);
    }

    inline void Camera::updateOrientation(){
        glm::vec3 front = data.front;
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));


        //matrix is orthogonal with respect to normal dot product so transpose is inverse
        data.orientation[0][0] = right.x;
        data.orientation[1][0] = right.y;
        data.orientation[2][0] = right.z;
        data.orientation[0][1] = up.x;
        data.orientation[1][1] = up.y;
        data.orientation[2][1] = up.z;
        data.orientation[0][2] = -front.x;
        data.orientation[1][2] = -front.y;
        data.orientation[2][2] = -front.z;

        data.right = right;
        data.up = up;
    }
}