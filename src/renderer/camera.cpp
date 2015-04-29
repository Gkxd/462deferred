#include "camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SFML/Window.hpp>

#define Keyboard sf::Keyboard
#define TOGGLE_DELAY 100

Camera::Camera() : eye_pos( glm::vec3( 0.0f, 0.0f, 0.0f ) ),
				   view_dir( glm::vec3( 0.0f, 0.0f, -1.0f ) ),
				   up_dir( glm::vec3( 0.0f, 1.0f, 0.0f ) ),
				   proj_mat( glm::perspective( 45.0f, 1.25f, 1.0f, 1000.0f )),
                   view_mat(glm::mat4())
{
}

Camera::Camera( float fovy, float aspect, float near, float far )
	: eye_pos( glm::vec3( 0.0f, 0.0f, 0.0f ) ),
	  view_dir( glm::vec3( 0.0f, 0.0f, -1.0f ) ),
	  up_dir( glm::vec3( 0.0f, 1.0f, 0.0f ) ),
	  proj_mat( glm::perspective( fovy, aspect, near, far ) )
{
}

Camera::~Camera()
{
}

float ang = 180;
int delayCounter;

void Camera::handleInput( float deltaTime )
{
	// adjust the camera position and orientation to account for movement over deltaTime seconds
	// use sf::IsKeyPressed( sf::Keyboard::A ) to check if 'a' is currently pressed, etc

    if (Keyboard::isKeyPressed(Keyboard::W)) {
        eye_pos += view_dir * deltaTime;
    }
    if (Keyboard::isKeyPressed(Keyboard::A)) {
        eye_pos -= glm::cross(view_dir, glm::vec3(0, 1, 0)) * deltaTime;
    }
    if (Keyboard::isKeyPressed(Keyboard::S)) {
        eye_pos -= view_dir * deltaTime;
    }
    if (Keyboard::isKeyPressed(Keyboard::D)) {
        eye_pos += glm::cross(view_dir, glm::vec3(0, 1, 0)) * deltaTime;
    }
    if (Keyboard::isKeyPressed(Keyboard::LShift)) {
        eye_pos -= glm::vec3(0, deltaTime, 0);
    }
    if (Keyboard::isKeyPressed(Keyboard::Space)) {
        eye_pos += glm::vec3(0, deltaTime, 0);
    }

    if (Keyboard::isKeyPressed(Keyboard::Q)) {
        ang += 15 * deltaTime;
        view_dir = glm::vec3(glm::sin(glm::radians(ang)), 0, glm::cos(glm::radians(ang)));
    }
    if (Keyboard::isKeyPressed(Keyboard::E)) {
        ang -= 15 * deltaTime;
        view_dir = glm::vec3(glm::sin(glm::radians(ang)), 0, glm::cos(glm::radians(ang)));
    }

    if (Keyboard::isKeyPressed(Keyboard::T)) {
        if (delayCounter <= 0) {
            toggle1 = !toggle1;
            delayCounter = TOGGLE_DELAY;
        }
    }

    if (delayCounter > 0) {
        delayCounter--;
    }

    view_mat = glm::lookAt(eye_pos, eye_pos + view_dir, up_dir);
}

// get a read-only handle to the projection matrix
const glm::mat4& Camera::getProjectionMatrix() const
{
	return proj_mat;
}

glm::mat4 Camera::getViewMatrix() const
{
	// construct and return a view matrix from your position representation
	return view_mat;
}

glm::vec3 Camera::getEye() const {
    return eye_pos;
}
glm::vec3 Camera::getDirection() const {
    return view_dir;
}
glm::vec3 Camera::getUp() const {
    return up_dir;
}