#ifndef _RENDERER_H_
#define _RENDERER_H_
#define GLEW_STATIC

#include <renderer/camera.hpp>
#include <scene/scene.hpp>

#define Vec2 glm::vec2
#define Vec3 glm::vec3

#define Map std::unordered_map
#define Vector std::vector

#define StaticModel Scene::StaticModel
#define TriangleGroup ObjModel::TriangleGroup
#define Triangle ObjModel::Triangle

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

class Renderer {
public:

    struct Point3f {
        float x;
        float y;
        float z;

        Point3f(float x2, float y2, float z2) {
            setXYZ(x2, y2, z2);
        }

        Point3f(Vec3 vec3) {
            setXYZ(vec3.x, vec3.y, vec3.z);
        }

        void setXYZ(float x2, float y2, float z2) {
            x = x2;
            y = y2;
            z = z2;
        }

        inline bool operator==(const Point3f other) { 
            return other.x == x && other.y == y && other.z == z;
        }
    };

    struct TexCoord {
        float u;
        float v;

        TexCoord(float u2, float v2) {
            setUV(u2, v2);
        }

        TexCoord(Vec2 vec2) {
            setUV(vec2.x, vec2.y);
        }

        void setUV(float u2, float v2) {
            u = u2;
            v = v2;
        }

        inline bool operator==(const TexCoord other) {
            return other.u == u && other.v == v;
        }
    };

    struct SubMesh {
        Vector<Point3f> vertexArray;
        Vector<Point3f> normalArray;
        Vector<TexCoord> texCoordArray;
        Vector<int> indexArray;

        // I only support meshes that have one vertex type per triangle group
        Triangle::VertexType vType;

        // I only support meshes that have one material per triangle group
        ObjModel::ObjMtl material;

        unsigned int vertexBuffer;
        unsigned int normalBuffer;
        unsigned int texCoordBuffer;
        unsigned int indexBuffer;

        unsigned int vao;

        int attemptToFindVertex(Point3f vertex, Point3f normal) const {
            for (int i = 0; i < vertexArray.size(); i++) {
                Point3f v = vertexArray[i];
                Point3f n = normalArray[i];
                if (v == vertex && n == normal) {
                    return i;
                }
            }

            return -1;
        }

        int attemptToFindVertex(Point3f vertex, Point3f normal, TexCoord texCoord) const {
            for (int i = 0; i < vertexArray.size(); i++) {
                Point3f v = vertexArray[i];
                Point3f n = normalArray[i];
                TexCoord t = texCoordArray[i];
                if (v == vertex && n == normal && t == texCoord) {
                    return i;
                }
            }

            return -1;
        }

        SubMesh(TriangleGroup tg, ObjModel obj) {
            Vector<Vec3> vertices = obj.getVertices();
            Vector<Vec3> normals = obj.getNormals();
            Vector<Vec2> texCoords = obj.getTexCoords();

            Vector<Triangle> triangles = tg.triangles;

            vType = triangles[0].vertexType;

            material = obj.getMaterial(triangles[0].materialID);

            printf("Number of triangles in group: %d\n", triangles.size());
            int triangleCount = 0;

            for (Triangle t : triangles) {
                if (++triangleCount % 5000 == 0) {
                    printf("Processing triangle %d / %d\n", triangleCount, triangles.size());
                }

                if (t.vertexType != vType) {
                    printf("Warning: Must have one type of vertex per triangle group! Skipping this triangle...\n");
                    continue;
                }

                switch (vType) {
                case Triangle::VertexType::POSITION_TEXCOORD_NORMAL:
                {
                    for (int i = 0; i < 3; i++) {
                        Point3f vert(vertices[t.vertices[i]]);
                        Point3f norm(normals[t.normals[i]]);
                        TexCoord texc(texCoords[t.texcoords[i]]);

                        int idx = attemptToFindVertex(vert, norm, texc);

                        if (idx == -1) {
                            indexArray.push_back(vertexArray.size());

                            vertexArray.push_back(vert);
                            normalArray.push_back(norm);
                            texCoordArray.push_back(texc);
                        }
                        else {
                            indexArray.push_back(idx);
                        }
                    }
                } break;
                case Triangle::VertexType::POSITION_NORMAL:
                {
                    for (int i = 0; i < 3; i++) {
                        Point3f vert(vertices[t.vertices[i]]);
                        Point3f norm(normals[t.normals[i]]);

                        int idx = attemptToFindVertex(vert, norm);

                        if (idx == -1) {
                            indexArray.push_back(vertexArray.size());

                            vertexArray.push_back(vert);
                            normalArray.push_back(norm);
                        }
                        else {
                            indexArray.push_back(idx);
                        }
                    }
                } break;
                case Triangle::VertexType::POSITION_TEXCOORD:
                {
                    Vec3 edgeA = vertices[t.vertices[1]] - vertices[t.vertices[0]];
                    Vec3 edgeB = vertices[t.vertices[2]] - vertices[t.vertices[0]];

                    Vec3 normVec = glm::normalize(glm::cross(edgeA, edgeB));

                    for (int i = 0; i < 3; i++) {
                        Point3f vert(vertices[t.vertices[i]]);
                        Point3f norm(normVec);
                        TexCoord texc(texCoords[t.texcoords[i]]);

                        int idx = attemptToFindVertex(vert, norm, texc);

                        if (idx == -1) {
                            indexArray.push_back(vertexArray.size());

                            vertexArray.push_back(vert);
                            normalArray.push_back(norm);
                            texCoordArray.push_back(texc);
                        }
                        else {
                            indexArray.push_back(idx);
                        }
                    }
                } break;
                case Triangle::VertexType::POSITION_ONLY:
                {
                    Vec3 edgeA = vertices[t.vertices[1]] - vertices[t.vertices[0]];
                    Vec3 edgeB = vertices[t.vertices[2]] - vertices[t.vertices[0]];

                    Vec3 normVec = glm::normalize(glm::cross(edgeA, edgeB));

                    for (int i = 0; i < 3; i++) {
                        Point3f vert(vertices[t.vertices[i]]);
                        Point3f norm(normVec);

                        int idx = attemptToFindVertex(vert, norm);

                        if (idx == -1) {
                            indexArray.push_back(vertexArray.size());

                            vertexArray.push_back(vert);
                            normalArray.push_back(norm);
                        }
                        else {
                            indexArray.push_back(idx);
                        }
                    }
                } break;
                }
            }
        }
    };

    struct ModelInfo {
        Vector<SubMesh> submeshes;

        Vector<unsigned int> textures;

        ModelInfo(StaticModel sm) {
            ObjModel obj = *sm.model;
            Vector<TriangleGroup> triangleGroups = obj.getGroups();

            textures = Vector<unsigned int>(obj.numTextures());

            for (TriangleGroup tg : triangleGroups) {
                SubMesh submesh = SubMesh(tg, obj);

                submeshes.push_back(submesh);
            }
        }
    };

    Map<std::string, ModelInfo> meshMap;

	// You may want to build some scene-specific OpenGL data before the first frame
	bool initialize(const Camera& camera, const Scene& scene, std::string shaderPath);

	/*
	 * Render a frame to the currently active OpenGL context.
	 * It's best to keep all your OpenGL-specific data in the renderer; keep the Scene class clean.
	 * This function should not modify the scene or camera.
	 */
	void render(const Camera& camera, const Scene& scene);

	// release all OpenGL data and allocated memory
	// you can do this in the destructor instead, but a callable function lets you swap scenes at runtime
	void release();
};

#endif // #ifndef _RENDERER_H_