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

GLuint loadVertexShader(const char* vertPath) {
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertShaderStr = readFile(vertPath);
    const char *vertShaderSrc = vertShaderStr.c_str();

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

    return vertShader;
}

GLuint loadFragmentShader(const char* fragPath) {
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragShaderStr = readFile(fragPath);
    const char *fragShaderSrc = fragShaderStr.c_str();

    GLint result = GL_FALSE;
    int logLength;

    std::cout << "Compiling fragment shader." << std::endl;
    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
    Vector<GLchar> fragShaderError((logLength > 1) ? logLength : 1);
    glGetShaderInfoLog(fragShader, logLength, NULL, &fragShaderError[0]);
    std::cout << &fragShaderError[0] << std::endl;

    return fragShader;
}

GLuint createShaderProgram(GLuint vertShader, GLuint fragShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    return program;
}

void linkShaderProgram(GLuint program) {
    GLint result = GL_FALSE;
    int logLength;

    std::cout << "Linking program" << std::endl;

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> programError((logLength > 1) ? logLength : 1);
    glGetProgramInfoLog(program, logLength, NULL, &programError[0]);
    std::cout << &programError[0] << std::endl;
}

void detachShaders(GLuint program, GLuint vertShader, GLuint fragShader) {
    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}

// Shadow map shader (Generates shadow maps)
GLuint shadowMapShader;
GLint shadowMapShader_lightMVPMat;

// Intermediate shader (Populates light maps)
GLuint intermediateShader;
GLint intermediateShader_lightMVPMat;
GLint intermediateShader_cameraMVPMat;
GLint intermediateShader_modelMat;
GLint intermediateShader_shadowMap;
GLint intermediateShader_lightPosition;
GLint intermediateShader_lightDirection;
GLint intermediateShader_lightType;

// Material shader (Populates material buffers)
GLuint materialShader;
GLint materialShader_cameraMVPMat;
GLint materialShader_cameraMVMat;
GLint materialShader_normalMat;
GLint materialShader_ambientTexture;
GLint materialShader_hasAmbientTexture;
GLint materialShader_diffuseTexture;
GLint materialShader_hasDiffuseTexture;
GLint materialShader_ambientColor;
GLint materialShader_diffuseColor;
GLint materialShader_specularColor;
GLint materialShader_specularExponent;

// Final pass shader (Does color computations)
GLuint finalPassShader;
GLint finalPass_lightMap;
GLint finalPass_normalTexture;
GLint finalPass_ambientTexture;
GLint finalPass_diffuseTexture;
GLint finalPass_specularTexture;
GLint finalPass_specularExponentTexture;
GLint finalPass_viewTexture;
GLint finalPass_ambientLight;
GLint finalPass_lightColor;
GLint finalPass_lightAttenuation;
GLint finalPass_lightType;
GLint finalPass_spotlightDirection; // For spotlights only
GLint finalPass_cosHalfLightAngle; // For spotlights only
GLint finalPass_spotlightFalloff; // For spotlights only
GLint finalPass_normalMatrix;

void initShaders(std::string shaderPath) {
    // Loading shaders

    GLuint vertShader;
    GLuint fragShader;

    //printf("%s\n", shaderPath.c_str);
    std::cout << shaderPath << std::endl;


    printf("Compiling shadow map shader...\n\n");
    vertShader = loadVertexShader((shaderPath + "\\shadowMap.vert").c_str());
    fragShader = loadFragmentShader((shaderPath + "\\shadowMap.frag").c_str());

    shadowMapShader = createShaderProgram(vertShader, fragShader);
    
    linkShaderProgram(shadowMapShader);
    detachShaders(shadowMapShader, vertShader, fragShader);

    // Setup uniforms for shadow map shader
    shadowMapShader_lightMVPMat = glGetUniformLocation(shadowMapShader, "lightMVPMat");
    if (shadowMapShader_lightMVPMat == -1) {
        printf("Could not find lightMVPMat\n\n");
    }
    printf("Finished compiling shadow map shader.\n\n");

    printf("Compiling intermediate shader...\n\n");
    vertShader = loadVertexShader((shaderPath + "\\intermediate.vert").c_str());
    fragShader = loadFragmentShader((shaderPath + "\\intermediate.frag").c_str());

    intermediateShader = createShaderProgram(vertShader, fragShader);

    linkShaderProgram(intermediateShader);
    detachShaders(intermediateShader, vertShader, fragShader);

    // Setup uniforms for intermediate shader
    intermediateShader_lightMVPMat = glGetUniformLocation(intermediateShader, "lightMVPMat");
    if (intermediateShader_lightMVPMat == -1) {
        printf("Could not find lightMVPMat\n\n");
    }
    intermediateShader_cameraMVPMat = glGetUniformLocation(intermediateShader, "cameraMVPMat");
    if (intermediateShader_cameraMVPMat == -1) {
        printf("Could not find cameraMVPMat\n\n");
    }
    intermediateShader_modelMat = glGetUniformLocation(intermediateShader, "modelMat");
    if (intermediateShader_modelMat == -1) {
        printf("Could not find modelMat\n\n");
    }
    intermediateShader_shadowMap = glGetUniformLocation(intermediateShader, "shadowMap");
    if (intermediateShader_shadowMap == -1) {
        printf("Could not find shadowMap\n\n");
    }
    intermediateShader_lightDirection = glGetUniformLocation(intermediateShader, "lightDirection");
    if (intermediateShader_lightDirection == -1) {
        printf("Could not find intermediateShader_lightDirection\n\n");
    }
    intermediateShader_lightPosition = glGetUniformLocation(intermediateShader, "lightPosition");
    if (intermediateShader_lightPosition == -1) {
        printf("Could not find lightPosition\n\n");
    }
    intermediateShader_lightType = glGetUniformLocation(intermediateShader, "lightType");
    if (intermediateShader_lightType == -1) {
        printf("Could not find lightType\n\n");
    }
    printf("Finished compiling intermediate shader.\n\n");

    printf("Compiling material shader...\n\n");
    vertShader = loadVertexShader((shaderPath + "\\material.vert").c_str());
    fragShader = loadFragmentShader((shaderPath + "\\material.frag").c_str());

    materialShader = createShaderProgram(vertShader, fragShader);

    glBindFragDataLocation(materialShader, 0, "normal");
    glBindFragDataLocation(materialShader, 1, "ambient");
    glBindFragDataLocation(materialShader, 2, "diffuse");
    glBindFragDataLocation(materialShader, 3, "specular");
    glBindFragDataLocation(materialShader, 4, "specularEx");

    linkShaderProgram(materialShader);
    detachShaders(materialShader, vertShader, fragShader);

    materialShader_cameraMVPMat = glGetUniformLocation(materialShader, "cameraMVPMat");
    if (materialShader_cameraMVPMat == -1) {
        printf("Could not find cameraMVPMat\n\n");
    }
    materialShader_cameraMVMat = glGetUniformLocation(materialShader, "cameraMVMat");
    if (materialShader_cameraMVMat == -1) {
        printf("Could not find cameraMVMat\n\n");
    }
    materialShader_normalMat = glGetUniformLocation(materialShader, "normalMat");
    if (materialShader_normalMat == -1) {
        printf("Could not find normalMat\n\n");
    }
    materialShader_ambientTexture = glGetUniformLocation(materialShader, "ambientTexture");
    if (materialShader_ambientTexture == -1) {
        printf("Could not find ambientTexture\n\n");
    }
    materialShader_hasAmbientTexture = glGetUniformLocation(materialShader, "hasAmbientTexture");
    if (materialShader_hasAmbientTexture == -1) {
        printf("Could not find hasAmbientTexture\n\n");
    }
    materialShader_diffuseTexture = glGetUniformLocation(materialShader, "diffuseTexture");
    if (materialShader_diffuseTexture == -1) {
        printf("Could not find diffuseTexture\n\n");
    }
    materialShader_hasDiffuseTexture = glGetUniformLocation(materialShader, "hasDiffuseTexture");
    if (materialShader_hasDiffuseTexture == -1) {
        printf("Could not find hasDiffuseTexture\n\n");
    }
    materialShader_ambientColor = glGetUniformLocation(materialShader, "ambientColor");
    if (materialShader_ambientColor == -1) {
        printf("Could not find ambientColor\n\n");
    }
    materialShader_diffuseColor = glGetUniformLocation(materialShader, "diffuseColor");
    if (materialShader_diffuseColor == -1) {
        printf("Could not find diffuseColor\n\n");
    }
    materialShader_specularColor = glGetUniformLocation(materialShader, "specularColor");
    if (materialShader_specularColor == -1) {
        printf("Could not find specularColor\n\n");
    }
    materialShader_specularExponent = glGetUniformLocation(materialShader, "specularExponent");
    if (materialShader_specularExponent == -1) {
        printf("Could not find specularExponent\n\n");
    }
    printf("Finished compiling material shader.\n\n");

    printf("Compiling final pass shader...\n\n");
    vertShader = loadVertexShader((shaderPath + "\\finalPass.vert").c_str());
    fragShader = loadFragmentShader((shaderPath + "\\finalPass.frag").c_str());

    finalPassShader = createShaderProgram(vertShader, fragShader);

    linkShaderProgram(finalPassShader);
    detachShaders(finalPassShader, vertShader, fragShader);

    // Setup uniforms for final pass shader
    finalPass_lightMap = glGetUniformLocation(finalPassShader, "lightMap");
    if (finalPass_lightMap == -1) {
        printf("Could not find lightMap\n\n");
    }
    finalPass_normalTexture = glGetUniformLocation(finalPassShader, "normalTexture");
    if (finalPass_normalTexture == -1) {
        printf("Could not find normalTexture\n\n");
    }
    finalPass_ambientTexture = glGetUniformLocation(finalPassShader, "ambientTexture");
    if (finalPass_ambientTexture == -1) {
        printf("Could not find ambientTexture\n\n");
    }
    finalPass_diffuseTexture = glGetUniformLocation(finalPassShader, "diffuseTexture");
    if (finalPass_diffuseTexture == -1) {
        printf("Could not find diffuseTexture\n\n");
    }
    finalPass_specularTexture = glGetUniformLocation(finalPassShader, "specularTexture");
    if (finalPass_specularTexture == -1) {
        printf("Could not find specularTexture\n\n");
    }
    finalPass_specularExponentTexture = glGetUniformLocation(finalPassShader, "specularExponentTexture");
    if (finalPass_specularExponentTexture == -1) {
        printf("Could not find specularExponentTexture\n\n");
    }
    finalPass_viewTexture = glGetUniformLocation(finalPassShader, "viewTexture");
    if (finalPass_viewTexture == -1) {
        printf("Could not find viewTexture\n\n");
    }
    finalPass_ambientLight = glGetUniformLocation(finalPassShader, "ambientLight");
    if (finalPass_ambientLight == -1) {
        printf("Could not find ambientLight\n\n");
    }
    finalPass_lightColor = glGetUniformLocation(finalPassShader, "lightColor");
    if (finalPass_lightColor == -1) {
        printf("Could not find lightColor\n\n");
    }
    finalPass_lightType = glGetUniformLocation(finalPassShader, "lightType");
    if (finalPass_lightType == -1) {
        printf("Could not find lightTyper\n\n");
    }
    finalPass_lightAttenuation = glGetUniformLocation(finalPassShader, "lightAttenuation");
    if (finalPass_lightAttenuation == -1) {
        printf("Could not find lightAttenuation\n\n");
    }
    finalPass_spotlightDirection = glGetUniformLocation(finalPassShader, "spotlightDirection");
    if (finalPass_spotlightDirection == -1) {
        printf("Could not find spotlightDirection\n\n");
    }
    finalPass_cosHalfLightAngle = glGetUniformLocation(finalPassShader, "cosHalfLightAngle");
    if (finalPass_cosHalfLightAngle == -1) {
        printf("Could not find cosHalfLightAngle\n\n");
    }
    finalPass_spotlightFalloff = glGetUniformLocation(finalPassShader, "spotlightFalloff");
    if (finalPass_spotlightFalloff == -1) {
        printf("Could not find spotlightFalloff\n\n");
    }
    finalPass_normalMatrix = glGetUniformLocation(finalPassShader, "normalMatrix");
    if (finalPass_normalMatrix == -1) {
        printf("Could not find normalMatrix\n\n");
    }

    printf("Finished compiling final pass shader.\n\n");
}

// Frame buffer that contains depth maps
GLuint depthFrameBuffer;
GLuint sunlightDepthTexture;
Vector<GLuint> spotlightDepthTextures;

// Frame buffer that contains information for deferred rendering
GLuint geometryFrameBuffer;
GLuint depthBuffer; // Need this for depth testing
GLuint sunlightTexture;
Vector<GLuint> spotlightTextures;
Vector<GLuint> pointlightTextures;
GLuint normalTexture;
GLuint matAmbientTexture;
GLuint matDiffuseTexture;
GLuint matSpecularTexture;
GLuint matSpecularExponentTexture;
GLuint viewTexture;

// Fullscreen quad
GLuint fullscreenQuadVAO;

// Matrix for biasing depth map
glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0);

// Clear color
GLuint clearColor[3] = { 0, 0, 0 };

bool Renderer::initialize(const Camera& camera, const Scene& scene, std::string shaderPath)
{
    // Initialize glew
    glewExperimental = TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cout << "glewInit failed, aborting." << std::endl;
        return false;
    }

    initShaders(shaderPath);

    // Loading the models and VAOs
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
                glBindAttribLocation(shadowMapShader, 0, "in_Position");
                glBindAttribLocation(intermediateShader, 0, "in_Position");
                glBindAttribLocation(materialShader, 0, "in_Position");

                glGenBuffers(1, &submesh.normalBuffer);
                glBindBuffer(GL_ARRAY_BUFFER, submesh.normalBuffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f) * submesh.normalArray.size(), &submesh.normalArray[0], GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(1);
                glBindAttribLocation(intermediateShader, 1, "in_Normal");
                glBindAttribLocation(materialShader, 0, "in_Normal");

                if (submesh.vType == Triangle::VertexType::POSITION_TEXCOORD || submesh.vType == Triangle::VertexType::POSITION_TEXCOORD_NORMAL) {
                    glGenBuffers(1, &submesh.texCoordBuffer);
                    glBindBuffer(GL_ARRAY_BUFFER, submesh.texCoordBuffer);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord) * submesh.texCoordArray.size(), &submesh.texCoordArray[0], GL_STATIC_DRAW);
                    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
                    glEnableVertexAttribArray(2);
                    glBindAttribLocation(materialShader, 2, "in_TexCoord");
                }

                glGenBuffers(1, &submesh.indexBuffer);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, submesh.indexBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * submesh.indexArray.size(), &submesh.indexArray[0], GL_STATIC_DRAW);

                mesh.submeshes[i] = submesh;
            }

            if (sm.model->numTextures() > 0) {
                glGenTextures(sm.model->numTextures(), &mesh.textures[0]);
            }

            for (int i = 0; i < sm.model->numTextures(); i++) {
                sf::Image img = sm.model->getTexture(i);
                glBindTexture(GL_TEXTURE_2D, mesh.textures[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getSize().x, img.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }

            std::cout << "Finished loading " << sm.model->getName() << std::endl;
            meshMap.insert({ sm.model->getName(), mesh });
            glBindVertexArray(0);
        }
        else {
            std::cout << "Skipping " << sm.model->getName() << std::endl;
        }
    }

    // Set up full screen quad
    glGenVertexArrays(1, &fullscreenQuadVAO);
    glBindVertexArray(fullscreenQuadVAO);
    static const GLfloat fullscreenQuadVertices[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };

    GLuint fullScreenQuad_VertexArray;
    glGenBuffers(1, &fullScreenQuad_VertexArray);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenQuadVertices), fullscreenQuadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Set up frame buffers

    // Depth map frame buffer
    glGenFramebuffers(1, &depthFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);

    glGenTextures(1, &sunlightDepthTexture);
    glBindTexture(GL_TEXTURE_2D, sunlightDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int numSpotlights = scene.getSpotlights().size();
    spotlightDepthTextures = Vector<GLuint>(numSpotlights);
    if (numSpotlights > 0) {
        glGenTextures(numSpotlights, &spotlightDepthTextures[0]);
    }
    for (int i = 0; i < numSpotlights; i++) {
        glBindTexture(GL_TEXTURE_2D, spotlightDepthTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glDrawBuffer(GL_NONE);

    // Geometry frame buffer
    glGenFramebuffers(1, &geometryFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, geometryFrameBuffer);

    // Need this to enable depth testing
    glGenTextures(1, &depthBuffer);
    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthBuffer, 0);

    glGenTextures(1, &sunlightTexture);
    glBindTexture(GL_TEXTURE_2D, sunlightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    spotlightTextures = Vector<GLuint>(numSpotlights);
    if (numSpotlights > 0) {
        glGenTextures(numSpotlights, &spotlightTextures[0]);
    }
    for (int i = 0; i < numSpotlights; i++) {
        glBindTexture(GL_TEXTURE_2D, spotlightTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    int numPointLights = scene.getPointlights().size();
    pointlightTextures = Vector<GLuint>(numPointLights);
    if (numPointLights > 0) {
        glGenTextures(numPointLights, &pointlightTextures[0]);
    }
    for (int i = 0; i < numPointLights; i++) {
        glBindTexture(GL_TEXTURE_2D, pointlightTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glGenTextures(1, &normalTexture);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &matDiffuseTexture);
    glBindTexture(GL_TEXTURE_2D, matDiffuseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &matAmbientTexture);
    glBindTexture(GL_TEXTURE_2D, matAmbientTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &matSpecularTexture);
    glBindTexture(GL_TEXTURE_2D, matSpecularTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &matSpecularExponentTexture);
    glBindTexture(GL_TEXTURE_2D, matSpecularExponentTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &viewTexture);
    glBindTexture(GL_TEXTURE_2D, viewTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    printf("Finished initializing\n");
	return true;
}

void Renderer::render(const Camera& camera, const Scene& scene) {
    const Vector<StaticModel> models = scene.getModels();

    glEnable(GL_DEPTH_TEST);
    ///*
    // Rendering from sunlight's POV
    glUseProgram(shadowMapShader);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sunlightDepthTexture, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, 1024, 1024);

    Scene::DirectionalLight sunlight = scene.getSunlight();

    glm::mat4 sunlightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 50.0f);

    Vec3 up = Vec3(0, 1, 0);
    if (1 - glm::abs(glm::dot(glm::normalize(sunlight.direction), up)) <= 0.01f) {
        up = Vec3(0, 0, 1);
    }
    glm::mat4 sunlightView = glm::lookAt(glm::vec3(0), sunlight.direction, up);

    for (StaticModel sm : models) {
        auto iter = meshMap.find(sm.model->getName());

        if (iter != meshMap.end()) {
            glm::mat4 modelTransform = glm::mat4();

            Vec3 eulerAngles = sm.orientation;

            modelTransform = glm::translate(modelTransform, sm.position);
            modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.z), Vec3(0, 0, 1)); //roll
            modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.y), Vec3(0, 1, 0)); //yaw
            modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.x), Vec3(1, 0, 0)); //pitch
            modelTransform = glm::scale(modelTransform, sm.scale);

            glm::mat4 mvpMat = sunlightProj * sunlightView * modelTransform;

            ModelInfo mesh = iter->second;

            glUniformMatrix4fv(shadowMapShader_lightMVPMat, 1, GL_FALSE, glm::value_ptr(mvpMat));

            for (SubMesh submesh : mesh.submeshes) {
                glBindVertexArray(submesh.vao);

                glDrawElements(GL_TRIANGLES, submesh.indexArray.size(), GL_UNSIGNED_INT, 0);
            }
        }
    }

    // Render a depth map for each spot light in the scene
    for (int i = 0; i < scene.getSpotlights().size(); i++) {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, spotlightDepthTextures[i], 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 1024, 1024);

        Scene::SpotLight spotlight = scene.getSpotlights()[i];
        glm::mat4 spotlightProj = glm::perspective(spotlight.angle, 1.0f, 0.1f, spotlight.length);

        up = Vec3(0, 1, 0);
        if (1 - glm::abs(glm::dot(glm::normalize(spotlight.direction), up)) <= 0.01f) {
            up = Vec3(0, 0, 1);
        }
        glm::mat4 spotlightView = glm::lookAt(spotlight.position, spotlight.position + spotlight.direction, up);

        for (StaticModel sm : models) {
            auto iter = meshMap.find(sm.model->getName());

            if (iter != meshMap.end()) {
                glm::mat4 modelTransform = glm::mat4();

                Vec3 eulerAngles = sm.orientation;

                modelTransform = glm::translate(modelTransform, sm.position);
                modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.z), Vec3(0, 0, 1)); //roll
                modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.y), Vec3(0, 1, 0)); //yaw
                modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.x), Vec3(1, 0, 0)); //pitch
                modelTransform = glm::scale(modelTransform, sm.scale);

                glm::mat4 mvpMat = spotlightProj * spotlightView * modelTransform;

                ModelInfo mesh = iter->second;

                glUniformMatrix4fv(shadowMapShader_lightMVPMat, 1, GL_FALSE, glm::value_ptr(mvpMat));

                for (SubMesh submesh : mesh.submeshes) {
                    glBindVertexArray(submesh.vao);

                    glDrawElements(GL_TRIANGLES, submesh.indexArray.size(), GL_UNSIGNED_INT, 0);
                }
            }
        }
    }
    //*/

    ///*
    // Render from camera's POV and write information to geometry buffer
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // First create light maps using the intermediate shader
    glUseProgram(intermediateShader);
    glBindFramebuffer(GL_FRAMEBUFFER, geometryFrameBuffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sunlightTexture, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform1i(intermediateShader_lightType, 0); // Sunlight
    glUniform3fv(intermediateShader_lightDirection, 1, glm::value_ptr(-sunlight.direction));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunlightDepthTexture);
    glUniform1i(intermediateShader_shadowMap, 0);

    glm::mat4 cameraProj = camera.getProjectionMatrix();
    glm::mat4 cameraView = camera.getViewMatrix();
    glm::mat4 cameraVPMat = cameraProj * cameraView;

    for (StaticModel sm : models) {
        auto iter = meshMap.find(sm.model->getName());

        if (iter != meshMap.end()) {
            glm::mat4 modelTransform = glm::mat4();

            Vec3 eulerAngles = sm.orientation;

            modelTransform = glm::translate(modelTransform, sm.position);
            modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.z), Vec3(0, 0, 1)); //roll
            modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.y), Vec3(0, 1, 0)); //yaw
            modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.x), Vec3(1, 0, 0)); //pitch
            modelTransform = glm::scale(modelTransform, sm.scale);

            glm::mat4 lightMVPMat = sunlightProj * sunlightView * modelTransform;
            glm::mat4 cameraMVPMat = cameraProj * cameraView * modelTransform;

            ModelInfo mesh = iter->second;

            glUniformMatrix4fv(intermediateShader_lightMVPMat, 1, GL_FALSE, glm::value_ptr(biasMatrix*lightMVPMat));
            glUniformMatrix4fv(intermediateShader_cameraMVPMat, 1, GL_FALSE, glm::value_ptr(cameraMVPMat));

            for (SubMesh submesh : mesh.submeshes) {
                glBindVertexArray(submesh.vao);

                glDrawElements(GL_TRIANGLES, submesh.indexArray.size(), GL_UNSIGNED_INT, 0);
            }
        }
    }

    // Do the above step, except for each spot light in the scene
    for (int i = 0; i < scene.getSpotlights().size(); i++) {
        Scene::SpotLight spotlight = scene.getSpotlights()[i];
        glm::mat4 spotlightProj = glm::perspective(spotlight.angle, 1.0f, 0.1f, spotlight.length);

        up = Vec3(0, 1, 0);
        if (1 - glm::abs(glm::dot(glm::normalize(spotlight.direction), up)) <= 0.01f) {
            up = Vec3(0, 0, 1);
        }
        glm::mat4 spotlightView = glm::lookAt(spotlight.position, spotlight.position + spotlight.direction, up);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, spotlightTextures[i], 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform1i(intermediateShader_lightType, 1); // Spotlight
        glUniform3fv(intermediateShader_lightPosition, 1, glm::value_ptr(spotlight.position));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, spotlightDepthTextures[i]);
        glUniform1i(intermediateShader_shadowMap, 0);

        for (StaticModel sm : models) {
            auto iter = meshMap.find(sm.model->getName());

            if (iter != meshMap.end()) {
                glm::mat4 modelTransform = glm::mat4();

                Vec3 eulerAngles = sm.orientation;

                modelTransform = glm::translate(modelTransform, sm.position);
                modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.z), Vec3(0, 0, 1)); //roll
                modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.y), Vec3(0, 1, 0)); //yaw
                modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.x), Vec3(1, 0, 0)); //pitch
                modelTransform = glm::scale(modelTransform, sm.scale);

                glm::mat4 lightMVPMat = spotlightProj * spotlightView * modelTransform;
                glm::mat4 cameraMVPMat = cameraVPMat * modelTransform;

                ModelInfo mesh = iter->second;

                glUniformMatrix4fv(intermediateShader_lightMVPMat, 1, GL_FALSE, glm::value_ptr(biasMatrix*lightMVPMat));
                glUniformMatrix4fv(intermediateShader_cameraMVPMat, 1, GL_FALSE, glm::value_ptr(cameraMVPMat));
                glUniformMatrix4fv(intermediateShader_modelMat, 1, GL_FALSE, glm::value_ptr(modelTransform));

                for (SubMesh submesh : mesh.submeshes) {
                    glBindVertexArray(submesh.vao);

                    glDrawElements(GL_TRIANGLES, submesh.indexArray.size(), GL_UNSIGNED_INT, 0);
                }
            }
        }
    }

    // Do the above step, except with each point light in the scene
    for (int i = 0; i < scene.getPointlights().size(); i++) {
        Scene::PointLight pointlight = scene.getPointlights()[i];

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pointlightTextures[i], 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform1i(intermediateShader_lightType, 2); // Point light
        glUniform3fv(intermediateShader_lightPosition, 1, glm::value_ptr(pointlight.position));

        for (StaticModel sm : models) {
            auto iter = meshMap.find(sm.model->getName());

            if (iter != meshMap.end()) {
                glm::mat4 modelTransform = glm::mat4();

                Vec3 eulerAngles = sm.orientation;

                modelTransform = glm::translate(modelTransform, sm.position);
                modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.z), Vec3(0, 0, 1)); //roll
                modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.y), Vec3(0, 1, 0)); //yaw
                modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.x), Vec3(1, 0, 0)); //pitch
                modelTransform = glm::scale(modelTransform, sm.scale);

                glm::mat4 cameraMVPMat = cameraProj * cameraView * modelTransform;

                ModelInfo mesh = iter->second;
                glUniformMatrix4fv(intermediateShader_cameraMVPMat, 1, GL_FALSE, glm::value_ptr(cameraMVPMat));
                glUniformMatrix4fv(intermediateShader_modelMat, 1, GL_FALSE, glm::value_ptr(modelTransform));

                for (SubMesh submesh : mesh.submeshes) {
                    glBindVertexArray(submesh.vao);

                    glDrawElements(GL_TRIANGLES, submesh.indexArray.size(), GL_UNSIGNED_INT, 0);
                }
            }
        }
    }

    // Populate the material buffers using the material shader
    glUseProgram(materialShader);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, normalTexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, matAmbientTexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, matDiffuseTexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, matSpecularTexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, matSpecularExponentTexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, viewTexture, 0);

    GLenum buffers[6] = { 
        GL_COLOR_ATTACHMENT0, 
        GL_COLOR_ATTACHMENT1, 
        GL_COLOR_ATTACHMENT2, 
        GL_COLOR_ATTACHMENT3, 
        GL_COLOR_ATTACHMENT4,
        GL_COLOR_ATTACHMENT5
    };
    glDrawBuffers(6, buffers);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (StaticModel sm : models) {
        auto iter = meshMap.find(sm.model->getName());

        if (iter != meshMap.end()) {
            glm::mat4 modelTransform = glm::mat4();

            Vec3 eulerAngles = sm.orientation;

            modelTransform = glm::translate(modelTransform, sm.position);
            modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.z), Vec3(0, 0, 1)); //roll
            modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.y), Vec3(0, 1, 0)); //yaw
            modelTransform = glm::rotate(modelTransform, glm::radians(eulerAngles.x), Vec3(1, 0, 0)); //pitch
            modelTransform = glm::scale(modelTransform, sm.scale);

            glm::mat4 cameraMVMat = cameraView * modelTransform;
            glm::mat4 cameraMVPMat = cameraProj * cameraMVMat;
            glm::mat4 normalMat = glm::transpose(glm::inverse(cameraMVMat));

            ModelInfo mesh = iter->second;

            glUniformMatrix4fv(materialShader_normalMat, 1, GL_FALSE, glm::value_ptr(normalMat));
            glUniformMatrix4fv(materialShader_cameraMVPMat, 1, GL_FALSE, glm::value_ptr(cameraMVPMat));
            glUniformMatrix4fv(materialShader_cameraMVMat, 1, GL_FALSE, glm::value_ptr(cameraMVMat));

            for (SubMesh submesh : mesh.submeshes) {
                glBindVertexArray(submesh.vao);

                ObjModel::ObjMtl material = submesh.material;

                if (material.map_Ka != -1) {
                    GLuint meshAmbientTexture = mesh.textures[material.map_Ka - 1];
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, meshAmbientTexture);
                    glUniform1i(materialShader_ambientTexture, 0);
                    glUniform1i(materialShader_hasAmbientTexture, true);
                }
                else {
                    glUniform1i(materialShader_hasAmbientTexture, false);
                }

                if (material.map_Kd != -1) {
                    GLuint meshDiffuseTexture = mesh.textures[material.map_Kd - 1];
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, meshDiffuseTexture);
                    glUniform1i(materialShader_diffuseTexture, 1);
                    glUniform1i(materialShader_hasDiffuseTexture, true);
                }
                else {
                    glUniform1i(materialShader_hasDiffuseTexture, false);
                }

                glUniform3fv(materialShader_ambientColor, 1, glm::value_ptr(material.Ka));
                glUniform3fv(materialShader_diffuseColor, 1, glm::value_ptr(material.Kd));
                glUniform3fv(materialShader_specularColor, 1, glm::value_ptr(material.Ks));
                glUniform1f(materialShader_specularExponent, material.Ns);

                glDrawElements(GL_TRIANGLES, submesh.indexArray.size(), GL_UNSIGNED_INT, 0);
            }
        }
    }
    //*/

    ///*
    // Render quad to the screen
    glUseProgram(finalPassShader);
    glDisable(GL_DEPTH_TEST); // Need to disable this for blending to work
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE); // This blend function adds to the current color on screen

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // For transforming light directions from world space to view space
    glUniformMatrix4fv(finalPass_normalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(cameraView))));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glUniform1i(finalPass_normalTexture, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, matAmbientTexture);
    glUniform1i(finalPass_ambientTexture, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, matDiffuseTexture);
    glUniform1i(finalPass_diffuseTexture, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, matSpecularTexture);
    glUniform1i(finalPass_specularTexture, 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, matSpecularExponentTexture);
    glUniform1i(finalPass_specularExponentTexture, 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, viewTexture);
    glUniform1i(finalPass_viewTexture, 5);

    glBindVertexArray(fullscreenQuadVAO);

    //Sunlight
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, sunlightTexture);
    glUniform1i(finalPass_lightMap, 6);
    glUniform1f(finalPass_ambientLight, sunlight.ambient);
    glUniform3fv(finalPass_lightColor, 1, glm::value_ptr(sunlight.color));
    glUniform3f(finalPass_lightAttenuation, 1, 0, 0);
    glUniform1i(finalPass_lightType, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //Set ambient light to 0 for rest of lights
    glUniform1f(finalPass_ambientLight, 0);

    glUniform1i(finalPass_lightType, 1);
    for (int i = 0; i < scene.getSpotlights().size(); i++) {
        Scene::SpotLight spotlight = scene.getSpotlights()[i];

        glBindTexture(GL_TEXTURE_2D, spotlightTextures[i]);
        glUniform1i(finalPass_lightMap, 6);
        glUniform3fv(finalPass_lightColor, 1, glm::value_ptr(spotlight.color));
        glUniform3f(finalPass_lightAttenuation, spotlight.Kc, spotlight.Kl, spotlight.Kq);

        glUniform3fv(finalPass_spotlightDirection, 1, glm::value_ptr(spotlight.direction));
        glUniform1f(finalPass_cosHalfLightAngle, (float)glm::cos(glm::radians(spotlight.angle / 2)));
        glUniform1f(finalPass_spotlightFalloff, spotlight.exponent);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glUniform1i(finalPass_lightType, 2);
    for (int i = 0; i < scene.getPointlights().size(); i++) {
        Scene::PointLight pointlight = scene.getPointlights()[i];

        glBindTexture(GL_TEXTURE_2D, pointlightTextures[i]);
        glUniform1i(finalPass_lightMap, 6);
        glUniform3fv(finalPass_lightColor, 1, glm::value_ptr(pointlight.color));
        glUniform3f(finalPass_lightAttenuation, pointlight.Kc, pointlight.Kl, pointlight.Kq);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glDisable(GL_BLEND);

    glBindVertexArray(0);
}

void Renderer::release()
{
    glDisable(GL_DEPTH_TEST);
}