#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "particleEffect.h"
#include "modelInfo.h"
#include "featureDefs.h"
#include "main.h"

#include "glObjects.h"

#if FEATURE_3D_RENDERING
sf::Shader* ModelInfo::shader = nullptr;
int32_t ModelInfo::shader_model_location = -1;
int32_t ModelInfo::shader_color_location = -1;
#endif


ModelInfo::ModelInfo()
: last_engine_particle_time(0), last_warp_particle_time(0), engine_scale(0), warp_scale(0.0f)
{
#if FEATURE_3D_RENDERING
    if (!shader && gl::isAvailable())
    {
        shader = ShaderManager::getShader("shaders/basicShader");
        shader_model_location = glGetUniformLocation(shader->getNativeHandle(), "model");
        shader_color_location = glGetUniformLocation(shader->getNativeHandle(), "color");
    }
#endif
}

void ModelInfo::setData(string name)
{
    this->data = ModelData::getModel(name);
    if (!this->data)
    {
        LOG(WARNING) << "Failed to find model data for: " << name;
    }
}

void ModelInfo::render(sf::Vector2f position, float rotation, const glm::mat4& model_matrix)
{
    if (!data)
        return;

    data->render(model_matrix);

    if (engine_scale > 0.0f)
    {
        if (engine->getElapsedTime() - last_engine_particle_time > 0.1)
        {
            for (unsigned int n=0; n<data->engine_emitters.size(); n++)
            {
                sf::Vector3f offset = data->engine_emitters[n].position * data->scale;
                sf::Vector2f pos2d = position + sf::rotateVector(sf::Vector2f(offset.x, offset.y), rotation);
                sf::Vector3f color = data->engine_emitters[n].color;
                sf::Vector3f pos3d = sf::Vector3f(pos2d.x, pos2d.y, offset.z);
                float scale = data->scale * data->engine_emitters[n].scale * engine_scale;
                ParticleEngine::spawn(pos3d, pos3d, color, color, scale, 0.0, 5.0);
            }
            last_engine_particle_time = engine->getElapsedTime();
        }
    }

    if (warp_scale > 0.0f)
    {
        if (engine->getElapsedTime() - last_warp_particle_time > 0.1)
        {
            int count = warp_scale * 10.0f;
            for(int n=0; n<count; n++)
            {
                sf::Vector3f offset = (data->mesh->randomPoint() + data->mesh_offset) * data->scale;
                sf::Vector2f pos2d = position + sf::rotateVector(sf::Vector2f(offset.x, offset.y), rotation);
                sf::Vector3f color = sf::Vector3f(0.6, 0.6, 1);
                sf::Vector3f pos3d = sf::Vector3f(pos2d.x, pos2d.y, offset.z);
                ParticleEngine::spawn(pos3d, pos3d, color, color, data->getRadius() / 15.0f, 0.0, 3.0);
            }
            last_warp_particle_time = engine->getElapsedTime();
        }
    }
}

void ModelInfo::renderOverlay(sf::Texture* texture, float alpha, const glm::mat4& model_matrix)
{
#if FEATURE_3D_RENDERING
    if (!data)
        return;

    auto overlay_matrix = glm::scale(model_matrix, glm::vec3(data->scale));
    overlay_matrix = glm::translate(overlay_matrix, glm::vec3(data->mesh_offset.x, data->mesh_offset.y, data->mesh_offset.z));
    
    glDepthFunc(GL_EQUAL);

    shader->setUniform("textureMap", *texture);
    sf::Shader::bind(shader);
    glUniform4fv(shader_color_location, 1, glm::value_ptr(glm::vec4(alpha, alpha, alpha, 1.f)));
    glUniformMatrix4fv(shader_model_location, 1, GL_FALSE, glm::value_ptr(overlay_matrix));
    data->mesh->render();
    glDepthFunc(GL_LESS);
#endif//FEATURE_3D_RENDERING
}

void ModelInfo::renderShield(float alpha, const glm::mat4& model_matrix)
{
#if FEATURE_3D_RENDERING
    shader->setUniform("textureMap", *textureManager.getTexture("shield_hit_effect.png"));
    
    sf::Shader::bind(shader);
    auto shield_matrix = glm::rotate(model_matrix, glm::radians(engine->getElapsedTime() * 5), glm::vec3(0.f, 0.f, 1.f));
    shield_matrix = glm::scale(shield_matrix, 1.2f * glm::vec3(data->radius));
    glUniformMatrix4fv(shader_model_location, 1, GL_FALSE, glm::value_ptr(shield_matrix));
    glUniform4fv(shader_color_location, 1, glm::value_ptr(glm::vec4(alpha, alpha, alpha, 1.f)));
    Mesh* m = Mesh::getMesh("sphere.obj");
    m->render();
#endif//FEATURE_3D_RENDERING
}

void ModelInfo::renderShield(float alpha, float angle, const glm::mat4& model_matrix)
{
#if FEATURE_3D_RENDERING
    if (!data) return;

    shader->setUniform("textureMap", *textureManager.getTexture("shield_hit_effect.png"));
    sf::Shader::bind(shader);

    auto shield_matrix = glm::rotate(model_matrix, glm::radians(angle), glm::vec3(0.f, 0.f, 1.f));
    shield_matrix = glm::rotate(shield_matrix, glm::radians(engine->getElapsedTime() * 5), glm::vec3(0.f, 0.f, 1.f));
    shield_matrix = glm::scale(shield_matrix, 1.2f * glm::vec3(data->radius));
    glUniformMatrix4fv(shader_model_location, 1, GL_FALSE, glm::value_ptr(shield_matrix));
    glUniform4fv(shader_color_location, 1, glm::value_ptr(glm::vec4(alpha, alpha, alpha, 1.f)));
    Mesh* m = Mesh::getMesh("half_sphere.obj");
    m->render();
#endif//FEATURE_3D_RENDERING
}
