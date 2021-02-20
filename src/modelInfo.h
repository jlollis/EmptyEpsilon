#ifndef MODEL_INFO_H
#define MODEL_INFO_H

#include "modelData.h"

#include <glm/mat4x4.hpp>

class ModelInfo : sf::NonCopyable
{
private:
    P<ModelData> data;
    float last_engine_particle_time;
    float last_warp_particle_time;
public:
    ModelInfo();

    float engine_scale;
    float warp_scale;

    void render(sf::Vector2f position, float rotation, const glm::mat4& model_matrix);
    void renderOverlay(sf::Texture* texture, float alpha, const glm::mat4& model_matrix);
    void renderShield(float alpha, const glm::mat4& model_matrix);
    void renderShield(float alpha, float angle, const glm::mat4& model_matrix);

    void setData(P<ModelData> data) { this->data = data; }
    void setData(string name);
};

#endif//MODEL_INFO_H
