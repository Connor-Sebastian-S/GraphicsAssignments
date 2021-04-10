#define GLEW_STATIC
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h> // For wglSwapInterval
#endif

#include <GL/freeglut.h>

// GLM defines and includes
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

// Include SOIL
#include <SOIL.h>

#include "../inc/CameraSetup.h"

Camera::Camera()
    : viewport_(0)
    , position_(0)
    , rotation_()
    , projectionMatrix_(1)
    , viewMatrix_(1)
    , viewDirty_(false)
{}

Camera::Camera(const int screenWidth, int screenHeight )
    : viewport_( 0, 0, screenWidth, screenHeight )
    , position_(0)
    , rotation_()
    , viewMatrix_(1)
    , projectionMatrix_(1)
    , viewDirty_( false )
{

}

void Camera::SetViewport( int x, int y, int width, int height )
{
    viewport_ = glm::vec4( x, y, width, height );
    glViewport(x, y, width, height);
}

glm::vec4 Camera::GetViewport() const
{
    return viewport_;
}

void Camera::SetProjectionRH( float fov, float aspectRatio, float zNear, float zFar )
{
    projectionMatrix_ = glm::perspective( glm::radians(fov), aspectRatio, zNear, zFar );
}

void Camera::ApplyViewMatrix()
{
    UpdateViewMatrix();
}

void Camera::SetPosition( const glm::vec3& pos )
{
    position_ = pos;
    viewDirty_ = true;
}

glm::vec3 Camera::GetPosition() const
{
    return position_;
}

void Camera::Translate( const glm::vec3& delta, bool local /* = true */ )
{
    if ( local )
        position_ += rotation_ * delta;
    else
        position_ += delta;
    viewDirty_ = true;
}

void Camera::SetRotation( const glm::quat& rot )
{
    rotation_ = rot;
    viewDirty_ = true;
}

glm::quat Camera::GetRotation() const
{
    return rotation_;
}

void Camera::SetEulerAngles( const glm::vec3& eulerAngles )
{
    rotation_ = glm::quat(glm::radians(eulerAngles));
}

glm::vec3 Camera::GetEulerAngles() const
{
    return glm::degrees(glm::eulerAngles( rotation_ ));
}

void Camera::Rotate( const glm::quat& rot )
{
    rotation_ = rotation_ * rot;
    viewDirty_ = true;
}

glm::mat4 Camera::GetProjectionMatrix()
{
    return projectionMatrix_;
}

glm::mat4 Camera::GetViewMatrix()
{
    UpdateViewMatrix();
    return viewMatrix_;
}

void Camera::UpdateViewMatrix()
{
    if ( viewDirty_ )
    {
	    const glm::mat4 translate = glm::translate(-position_);
	    const glm::mat4 rotate = glm::transpose(glm::toMat4(rotation_));

        viewMatrix_ = rotate * translate;

        viewDirty_ = false;
    }
}
