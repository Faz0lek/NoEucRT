/**
 * @file LambertianShader.cpp
 * @author Martin Kostelník (xkoste12@stud.fit.vutbr.cz)
 * @brief Implementation file of the LambertianShader class, which is derived from Shader class.
 * @version 1.0
 * @date 2021-05-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "LambertianShader.hpp"

#include <glm/gtx/norm.hpp>

LambertianShader::LambertianShader()
{
}

glm::vec3 LambertianShader::getColor(const Ray& ray, const Scene& scene, const glm::vec4& hitPoint, const Model& hitModel, const Triangle& hitTriangle, const float u, const float v) const
{
    glm::vec3 hitColor(0.0f);

    // Calculate edge functions
    const glm::vec3 edge1 = { hitTriangle.v2.x - hitTriangle.v1.x, hitTriangle.v2.y - hitTriangle.v1.y, hitTriangle.v2.z - hitTriangle.v1.z };
    const glm::vec3 edge2 = { hitTriangle.v3.x - hitTriangle.v1.x, hitTriangle.v3.y - hitTriangle.v1.y, hitTriangle.v3.z - hitTriangle.v1.z };

    // Calculate hit normal
    const glm::vec4 hitNormal = { glm::normalize(glm::cross(edge1, edge2)), 0.0f };

    glm::vec3 albedoColor(0.0f);

    // Texture mapping
    if (!hitModel.textureCoordinateMapping.empty() && hitModel.texture.getSize().x != 0)
    {
        const glm::vec2 textureCoordinates = (*hitModel.textureCoordinateMapping.at(&hitTriangle.v1)) * (1 - u - v)
                                           + (*hitModel.textureCoordinateMapping.at(&hitTriangle.v2)) * u
                                           + (*hitModel.textureCoordinateMapping.at(&hitTriangle.v3)) * v;

        sf::Color color = hitModel.texture.getPixel(textureCoordinates.x * (hitModel.texture.getSize().x - 1), textureCoordinates.y * (hitModel.texture.getSize().y - 1));
        albedoColor = glm::vec3(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);
    }
    else
    {
        albedoColor = hitModel.material.albedo;
    }

    #pragma omp parallel for
    for (int i = 0; i < scene.lights.size(); i++)
    {
        glm::vec4 lightDirection;
        glm::vec3 lightAmount;

        switch (scene.lights[i].type)
        {
        case Light::Type::Distant:
            lightDirection = -scene.lights[i].position;
            lightAmount = scene.lights[i].intensity * scene.lights[i].color * std::max(0.0f, glm::dot(hitNormal, glm::normalize(lightDirection)));
            break;
            
        case Light::Type::Point:
            lightDirection = scene.lights[i].position - hitPoint;
            lightAmount = scene.lights[i].intensity * scene.lights[i].color * 2e4f / (4 * glm::pi<float>() * glm::length2(lightDirection));
            lightAmount *= std::max(0.0f, glm::dot(hitNormal, glm::normalize(lightDirection)));
            break;
        }

        const float bias = 0.001f;
        Ray shadowRay(hitPoint + hitNormal * bias, glm::normalize(lightDirection));

        hitColor += float(shadowRay.seesLight(scene.lights[i], scene)) * albedoColor * lightAmount;
    }

    return glm::clamp(hitColor, 0.0f, 255.0f);
}
