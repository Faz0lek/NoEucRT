#include "PBRShader.hpp"
#include <glm/gtx/norm.hpp>

PBRShader::PBRShader()
{
}

glm::vec3 PBRShader::getColor(const Ray& primaryRay, const Scene& scene, const Model& hitModel, const Triangle& hitTriangle, const float& distance) const
{
    const glm::vec3 hitPoint = primaryRay.origin + glm::normalize(primaryRay.direction) * distance;

    const glm::vec3 edge1 = { hitTriangle.v2.x - hitTriangle.v1.x, hitTriangle.v2.y - hitTriangle.v1.y, hitTriangle.v2.z - hitTriangle.v1.z };
    const glm::vec3 edge2 = { hitTriangle.v3.x - hitTriangle.v1.x, hitTriangle.v3.y - hitTriangle.v1.y, hitTriangle.v3.z - hitTriangle.v1.z };
    
    const glm::vec3 hitNormal = glm::normalize(glm::cross(edge1, edge2));
    
    glm::vec3 hitColor(0.0f);

    #pragma omp parallel for
    for (int i = 0; i < scene.lights.size(); i++)
    {
        glm::vec3 lightDirection;
        glm::vec3 lightAmount;

        switch (scene.lights[i].type)
        {
        case Light::Type::Distant:
            lightDirection = -scene.lights[i].position;
            lightAmount = scene.lights[i].intensity * scene.lights[i].color * std::max(0.0f, glm::dot(hitNormal, glm::normalize(lightDirection)));
            break;
            
        case Light::Type::Point:
            lightDirection = scene.lights[i].position - hitPoint;
            lightAmount = scene.lights[i].intensity * scene.lights[i].color * 1e4f / (4 * glm::pi<float>() * glm::length2(lightDirection)) * std::max(0.0f, glm::dot(hitNormal, glm::normalize(lightDirection)));
            break;
        }
        Ray shadowRay({ hitPoint, 1.0f }, { lightDirection, 0.0f });

        hitColor += float(shadowRay.seesLight(scene.lights[i], scene)) * hitModel.material.albedo * lightAmount;
    }

    return glm::clamp(hitColor, 0.0f, 255.0f);
}
