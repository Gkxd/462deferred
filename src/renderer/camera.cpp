#include "camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SFML/Window.hpp>

#define Keyboard sf::Keyboard

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

void Camera::handleInput( float deltaTime )
{
	// adjust the camera position and orientation to account for movement over deltaTime seconds
	// use sf::IsKeyPressed( sf::Keyboard::A ) to check if 'a' is currently pressed, etc

    if (Keyboard::isKeyPressed(Keyboard::W)) {
        view_mat = glm::translate(view_mat, glm::vec3(0, 0, deltaTime));
    }
    if (Keyboard::isKeyPressed(Keyboard::A)) {
        view_mat = glm::translate(view_mat, glm::vec3(deltaTime, 0, 0));
    }
    if (Keyboard::isKeyPressed(Keyboard::S)) {
        view_mat = glm::translate(view_mat, glm::vec3(0, 0, -deltaTime));
    }
    if (Keyboard::isKeyPressed(Keyboard::D)) {
        view_mat = glm::translate(view_mat, glm::vec3(-deltaTime, 0, 0));
    }
    if (Keyboard::isKeyPressed(Keyboard::LShift)) {
        view_mat = glm::translate(view_mat, glm::vec3(0, deltaTime, 0));
    }
    if (Keyboard::isKeyPressed(Keyboard::Space)) {
        view_mat = glm::translate(view_mat, glm::vec3(0, -deltaTime, 0));
    }

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