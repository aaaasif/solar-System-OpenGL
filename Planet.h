// Planet.h
#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "Sphere.h"

class Planet {
public:
    Planet(const std::string& name, float radius, float orbitSpeed, float rotationSpeed, const std::string& texturePath);
    void Update(float deltaTime);
    void Draw(Shader& shader);

    glm::vec3 GetPosition() const;

private:
    std::string name;
    float radius;
    float orbitSpeed;
    float rotationSpeed;
    glm::vec3 position;
    Sphere sphere;
    unsigned int textureID;
    glm::mat4 modelMatrix;
};

// In main function
std::vector<Planet> planets;
planets.emplace_back("Mercury", 10.0f, 47.87f, 0.05f, "resources/planets/2k_mercury.jpg");
planets.emplace_back("Venus", 12.0f, 35.02f, 0.012f, "resources/planets/2k_venus.jpg");
// Add other planets similarly

while (!glfwWindowShouldClose(window)) {
    // Update and draw planets
    for (Planet& planet : planets) {
        planet.Update(deltaTime);
        planet.Draw(SimpleShader);
    }

    // Other rendering code...
}
