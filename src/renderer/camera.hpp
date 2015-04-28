#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <glm/glm.hpp>

class Camera {
private:
	glm::mat4 proj_mat;
    glm::mat4 view_mat;

	// change these to implement camera movement as you see fit
	glm::vec3 eye_pos;
	glm::vec3 view_dir;
	glm::vec3 up_dir;

public:

	Camera();
	Camera( float fovy, float aspect, float near, float far );
	~Camera();

	const glm::mat4& getProjectionMatrix() const;
	glm::mat4 getViewMatrix() const;

    glm::vec3 getEye() const;
    glm::vec3 getDirection() const;
    glm::vec3 getUp() const;

	void handleInput( float deltaTime );
};

#endif // #ifndef _CAMERA_H_