#define GLEW_STATIC

#include "renderer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL\glew.h>
#include <SFML\OpenGL.hpp>
#include <iostream>
#include <fstream>

// Shader compiling reference: http://www.nexcius.net/2012/11/20/how-to-load-a-glsl-shader-in-opengl-using-c/

std::string readFile(const char *filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if (!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }

    std::string line = "";
    while (!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}

GLuint loadShaders() {
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertShaderStr = readFile("D:\\Daniel Hua\\Home\\CMU\\2015 Spring\\15-462\\hw\\04\\shaders\\test.vert");
    std::string fragShaderStr = readFile("D:\\Daniel Hua\\Home\\CMU\\2015 Spring\\15-462\\hw\\04\\shaders\\test.frag");
    const char *vertShaderSrc = vertShaderStr.c_str();
    const char *fragShaderSrc = fragShaderStr.c_str();

    GLint result = GL_FALSE;
    int logLength;

    std::cout << "Compiling vertex shader." << std::endl;
    glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vertShader);

    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
    Vector<GLchar> vertShaderError((logLength > 1) ? logLength : 1);
    glGetShaderInfoLog(vertShader, logLength, NULL, &vertShaderError[0]);
    std::cout << &vertShaderError[0] << std::endl;

    std::cout << "Compiling fragment shader." << std::endl;
    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
    Vector<GLchar> fragShaderError((logLength > 1) ? logLength : 1);
    glGetShaderInfoLog(fragShader, logLength, NULL, &fragShaderError[0]);
    std::cout << &fragShaderError[0] << std::endl;

    std::cout << "Linking program" << std::endl;
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> programError((logLength > 1) ? logLength : 1);
    glGetProgramInfoLog(program, logLength, NULL, &programError[0]);
    std::cout << &programError[0] << std::endl;

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}

GLuint shaderProgram;
GLint uniform_mvpMat;
GLint uniform_normalMat;

bool Renderer::initialize( const Camera& camera, const Scene& scene )
{
    // Initialize glew
    glewExperimental = TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cout << "glewInit failed, aborting." << std::endl;
        return false;
    }

    shaderProgram = loadShaders();
    uniform_mvpMat = glGetUniformLocation(shaderProgram, "uniform_mvpMat");
    uniform_normalMat = glGetUniformLocation(shaderProgram, "uniform_normalMat");
    if (uniform_mvpMat == -1) {
        printf("Could not find uniform_mvpMat\n\n");
    }
    if (uniform_normalMat == -1) {
        printf("Could not find uniform_normalMat\n\n");
    }

    const Vector<StaticModel> models = scene.getModels();

    for (StaticModel sm : models) {
        if (meshMap.count(sm.model->getName()) == 0) {
            std::cout << "Loading " << sm.model->getName() << std::endl;
            ModelInfo mesh = ModelInfo(sm);

            for (int i = 0; i < mesh.submeshes.size(); i++) {

                SubMesh submesh = mesh.submeshes[i];

                glGenVertexArrays(1, &submesh.vao);
                glBindVertexArray(submesh.vao);

                glGenBuffers(1, &submesh.vertexBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, submesh.vertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * submesh.vertexArray.size(), &submesh.vertexArray[0], GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(0);
                glBindAttribLocation(shaderProgram, 0, "in_Position");

                glGenBuffers(1, &submesh.normalBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, submesh.normalBuffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * submesh.normalArray.size(), &submesh.normalArray[0], GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(1);
                glBindAttribLocation(shaderProgram, 1, "in_Normal");

                if (submesh.vType == Triangle::VertexType::POSITION_TEXCOORD || submesh.vType == Triangle::VertexType::POSITION_TEXCOORD_NORMAL) {
                    glGenBuffers(1, &submesh.texCoordBuffer);
                    glBindBuffer(GL_ARRAY_BUFFER, submesh.texCoordBuffer);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord) * submesh.texCoordArray.size(), &submesh.texCoordArray[0], GL_STATIC_DRAW);
                    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
                    glEnableVertexAttribArray(2);
                    glBindAttribLocation(shaderProgram, 2, "in_TexCoord");
                }

                mesh.submeshes[i] = submesh;
            }
            meshMap.insert({ sm.model->getName(), mesh });
            glBindVertexArray(0);
        }
        else {
            std::cout << "Skipping " << sm.model->getName() << std::endl;
        }
    }

    /*
    for (StaticModel sm : models) {
        auto iter = meshMap.find(sm.model->getName());
        if (iter != meshMap.end()) {
            ModelInfo mesh = iter->second;

            std::cout << sm.model->getName() << std::endl;

            printf("Vertex array:\n");
            for (Point3f p : mesh.vertexArray) {
                printf("v %f, %f, %f\n", p.x, p.y, p.z);
            }
            printf("Normal array:\n");
            for (Point3f p : mesh.normalArray) {
                printf("n %f, %f, %f\n", p.x, p.y, p.z);
            }
            printf("TexCoord array:\n");
            for (TexCoord t : mesh.texCoordArray) {
                printf("tx %f, %f\n", t.u, t.v);
            }
            printf("Index array:\n");
            for (int idx : mesh.indexArray) {
                printf("i %d\n", idx);
            }
        }
    }//*/

    glEnable(GL_DEPTH_TEST);
	return true;
}

void Renderer::render( const Camera& camera, const Scene& scene ) {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    glm::mat4 cameraProj = camera.getProjectionMatrix();
    glm::mat4 cameraView = camera.getViewMatrix();

    const Vector<StaticModel> models = scene.getModels();

    for (StaticModel sm : models) {
        auto iter = meshMap.find(sm.model->getName());

        if (iter != meshMap.end()) {
            glm::mat4 modelTransform = glm::mat4();

            glm::vec3 eulerAngles = sm.orientation;

            modelTransform = glm::scale(modelTransform, sm.scale);
            modelTransform = glm::rotate(modelTransform, eulerAngles.x, glm::vec3(1, 0, 0)); //pitch
            modelTransform = glm::rotate(modelTransform, eulerAngles.y, glm::vec3(0, 1, 0)); //yaw
            modelTransform = glm::rotate(modelTransform, eulerAngles.z, glm::vec3(0, 0, 1)); //roll
            modelTransform = glm::translate(modelTransform, sm.position);

            glm::mat4 mvpMat = cameraProj * cameraView * modelTransform;
            glm::mat4 normalMat = glm::transpose(glm::inverse(mvpMat));

            ModelInfo mesh = iter->second;

            glUniformMatrix4fv(uniform_mvpMat, 1, GL_FALSE, glm::value_ptr(mvpMat));
            glUniformMatrix4fv(uniform_normalMat, 1, GL_FALSE, glm::value_ptr(normalMat));

            for (SubMesh submesh : mesh.submeshes) {
                glBindVertexArray(submesh.vao);

                glDrawElements(GL_TRIANGLES, submesh.indexArray.size(), GL_UNSIGNED_INT, &(submesh.indexArray[0]));
            }
        }
    }
    glBindVertexArray(0);
}

void Renderer::release()
{
    glDisable(GL_DEPTH_TEST);
}