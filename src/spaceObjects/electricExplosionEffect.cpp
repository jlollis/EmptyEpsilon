#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include "main.h"
#include "electricExplosionEffect.h"
#include "glObjects.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#if FEATURE_3D_RENDERING
sf::Shader* ElectricExplosionEffect::basicShader = nullptr;
sf::Shader* ElectricExplosionEffect::particlesShader = nullptr;
uint32_t ElectricExplosionEffect::particlesShaderPositionAttribute = 0;
uint32_t ElectricExplosionEffect::particlesShaderTexCoordsAttribute = 0;

int32_t ElectricExplosionEffect::basicShaderModelLocation = -1;
int32_t ElectricExplosionEffect::basicShaderColorLocation = -1;
int32_t ElectricExplosionEffect::particlesShaderModelLocation = -1;

struct VertexAndTexCoords
{
    sf::Vector3f vertex;
    sf::Vector2f texcoords;
};
#endif

/// ElectricExplosionEffect is a visible electrical explosion, as seen from EMP missiles
/// Example: ElectricExplosionEffect():setPosition(500,5000):setSize(20)
REGISTER_SCRIPT_SUBCLASS(ElectricExplosionEffect, SpaceObject)
{
    REGISTER_SCRIPT_CLASS_FUNCTION(ElectricExplosionEffect, setSize);
    REGISTER_SCRIPT_CLASS_FUNCTION(ElectricExplosionEffect, setOnRadar);
}

REGISTER_MULTIPLAYER_CLASS(ElectricExplosionEffect, "ElectricExplosionEffect");
ElectricExplosionEffect::ElectricExplosionEffect()
: SpaceObject(1000.0, "ElectricExplosionEffect")
{
    has_weight = false;
    on_radar = false;
    size = 1.0;

    setCollisionRadius(1.0);
    lifetime = maxLifetime;
    for(int n=0; n<particleCount; n++)
        particleDirections[n] = sf::normalize(sf::Vector3f(random(-1, 1), random(-1, 1), random(-1, 1))) * random(0.8, 1.2);

    registerMemberReplication(&size);
    registerMemberReplication(&on_radar);
#if FEATURE_3D_RENDERING
    if (!particlesShader && gl::isAvailable())
    {
        particlesShader = ShaderManager::getShader("shaders/billboard");
        particlesShaderModelLocation = glGetUniformLocation(particlesShader->getNativeHandle(), "model");
        particlesShaderPositionAttribute = glGetAttribLocation(particlesShader->getNativeHandle(), "position");
        particlesShaderTexCoordsAttribute = glGetAttribLocation(particlesShader->getNativeHandle(), "texcoords");

        basicShader = ShaderManager::getShader("shaders/basicShader");
        basicShaderModelLocation = glGetUniformLocation(basicShader->getNativeHandle(), "model");
        basicShaderColorLocation = glGetUniformLocation(basicShader->getNativeHandle(), "color");
    }
#endif
}

//due to a suspected compiler bug this deconstructor needs to be explicitly defined
ElectricExplosionEffect::~ElectricExplosionEffect()
{
}

#if FEATURE_3D_RENDERING
void ElectricExplosionEffect::draw3DTransparent()
{
    float f = (1.0f - (lifetime / maxLifetime));
    float scale;
    float alpha = 0.5;
    if (f < 0.2f)
    {
        scale = (f / 0.2f) * 0.8;
    }else{
        scale = Tween<float>::easeOutQuad(f, 0.2, 1.0, 0.8f, 1.0f);
        alpha = Tween<float>::easeInQuad(f, 0.2, 1.0, 0.5f, 0.0f);
    }

    auto model_matrix = getModelMatrix();
    auto explosion_matrix = glm::scale(model_matrix, glm::vec3(scale * size));

    basicShader->setUniform("textureMap", *textureManager.getTexture("electric_sphere_texture.png"));
    sf::Shader::bind(basicShader);
    glUniform4fv(basicShaderColorLocation, 1, glm::value_ptr(glm::vec4(alpha, alpha, alpha, 1.f)));
    glUniformMatrix4fv(basicShaderModelLocation, 1, GL_FALSE, glm::value_ptr(explosion_matrix));
    Mesh* m = Mesh::getMesh("sphere.obj");
    m->render();

    glUniformMatrix4fv(basicShaderModelLocation, 1, GL_FALSE, glm::value_ptr(glm::scale(explosion_matrix, glm::vec3(.5f))));
    m->render();

    scale = Tween<float>::easeInCubic(f, 0.0, 1.0, 0.3f, 3.0f);
    float r = Tween<float>::easeOutQuad(f, 0.0, 1.0, 1.0f, 0.0f);
    float g = Tween<float>::easeOutQuad(f, 0.0, 1.0, 1.0f, 0.0f);
    float b = Tween<float>::easeInQuad(f, 0.0, 1.0, 1.0f, 0.0f);

    constexpr size_t quad_count = 10;
    std::array<VertexAndTexCoords, 4 * quad_count> quads;

    // Initialize texcoords per quad.
    for (auto i = 0; i < quads.size(); i += 4)
    {
        quads[i + 0].texcoords = { 0.f, 0.f };
        quads[i + 1].texcoords = { 1.f, 0.f };
        quads[i + 2].texcoords = { 1.f, 1.f };
        quads[i + 3].texcoords = { 0.f, 1.f };
    }

    particlesShader->setUniform("textureMap", *textureManager.getTexture("particle.png"));

    sf::Shader::bind(particlesShader);
    glUniformMatrix4fv(particlesShaderModelLocation, 1, GL_FALSE, glm::value_ptr(model_matrix));
    particlesShader->setUniform("color", sf::Glsl::Vec4(r, g, b, size / 32.0f));
    gl::ScopedVertexAttribArray positions(particlesShaderPositionAttribute);
    gl::ScopedVertexAttribArray texcoords(particlesShaderTexCoordsAttribute);

    // We're drawing particles `quad_count` at a time.
    for (size_t n = 0; n < particleCount;)
    {
        auto active_quads = std::min(quad_count, particleCount - n);
        // setup quads
        for (auto p = 0; p < active_quads; ++p)
        {
            sf::Vector3f v = particleDirections[n + p] * scale * size;
            quads[4 * p + 0].vertex = v;
            quads[4 * p + 1].vertex = v;
            quads[4 * p + 2].vertex = v;
            quads[4 * p + 3].vertex = v;
        }

        glVertexAttribPointer(positions.get(), 3, GL_FLOAT, GL_FALSE, sizeof(VertexAndTexCoords), (GLvoid*)quads.data());
        glVertexAttribPointer(texcoords.get(), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAndTexCoords), (GLvoid*)((char*)quads.data() + sizeof(sf::Vector3f)));
        glDrawArrays(GL_QUADS, 0, 4 * active_quads);
        n += active_quads;
    }
}
#endif//FEATURE_3D_RENDERING

void ElectricExplosionEffect::drawOnRadar(sf::RenderTarget& window, sf::Vector2f position, float scale, float rotation, bool long_range)
{
    if (!on_radar)
        return;
    if (long_range)
        return;

    sf::CircleShape circle(size * scale);
    circle.setOrigin(size * scale, size * scale);
    circle.setPosition(position);
    circle.setFillColor(sf::Color(0, 0, 255, 64 * (lifetime / maxLifetime)));
    window.draw(circle);
}

void ElectricExplosionEffect::update(float delta)
{
    if (delta > 0 && lifetime == maxLifetime)
        soundManager->playSound("sfx/emp_explosion.wav", getPosition(), size * 2, 60.0);
    lifetime -= delta;
    if (lifetime < 0)
        destroy();
}
