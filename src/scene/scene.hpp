#ifndef _SCENE_H_
#define _SCENE_H_

#include <SFML/System/String.hpp>
#include <scene/objmodel.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Scene {
public:

	struct StaticModel
	{
		glm::vec3 position;
		glm::vec3 orientation; // you may want to change this to a rotation matrix or quaternion
		                       // if so, you can edit the scene loader to do the conversion up-front
		                       // look through glm's docs to see what options are available!
							   // keep in mind the values from the scene file are in degrees!
		glm::vec3 scale;

		// you may want to change this when you build meshes
		const ObjModel * model;
	
        StaticModel() : scale(glm::vec3(1.0f, 1.0f, 1.0f))
        {
        };
    };

	struct DirectionalLight
	{
		glm::vec3 direction;
		glm::vec3 color;
		float ambient;
	};

	struct SpotLight
    {
        glm::vec3 initialPosition;
		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 color;
		float exponent;
		float angle;
        float length;
        float velocity;
		float Kc, Kl, Kq; // attenuation constants

		SpotLight() : position( glm::vec3( 0.0f, 0.0f, 0.0f ) ),
			          direction( glm::vec3( 0.0f, 0.0f, 0.0f ) ),
			          color( glm::vec3( 0.0f, 0.0f, 0.0f ) ),
			          exponent( 0.0f ),
			          angle( 0.0f ),
			          length( 0.0f ),
                      velocity( 0.0f ),
			          Kc( 0.0f ), Kl( 0.0f ), Kq( 0.0f )
		{
		};
	};

	struct PointLight
	{
        glm::vec3 initialPosition;
		glm::vec3 position;
		glm::vec3 color;
		float velocity;
		float Kc, Kl, Kq;
	};

private:
	std::unordered_map<std::string, ObjModel> objmodels;
	std::vector<StaticModel> models;
	DirectionalLight sunlight;
	std::vector<SpotLight> spotlights;
	std::vector<PointLight> pointlights;
	
public:
	Scene();
	bool loadFromFile( std::string filename );

    const std::unordered_map<std::string, ObjModel> getObjModels() const;
    const std::vector<StaticModel> getModels() const;
    const DirectionalLight getSunlight() const;
    const std::vector<SpotLight> getSpotlights() const;
    const std::vector<PointLight> getPointlights() const;

    void update(float deltaTime);

	~Scene();
};

#endif // #ifndef _SCENE_H_