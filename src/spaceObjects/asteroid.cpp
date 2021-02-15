#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "asteroid.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "explosionEffect.h"
#include "main.h"
#include "pathPlanner.h"

#include "scriptInterface.h"
#include "glObjects.h"

#if FEATURE_3D_RENDERING
sf::Shader* Asteroid::shader = nullptr;
int32_t Asteroid::shader_model_location = -1;
int32_t Asteroid::shader_model_normal_location = -1;
#endif

/// An asteroid in space. Which you can fly into and hit. Will do damage.
REGISTER_SCRIPT_SUBCLASS(Asteroid, SpaceObject)
{
    /// Set the radius of this asteroid
    /// The default radius for an asteroid is between 110 and 130
    /// Example: Asteroid():setSize(50)
    REGISTER_SCRIPT_CLASS_FUNCTION(Asteroid, setSize);
    /// Gets the current radius of this asteroid
    /// Example: local size=Asteroid():getSize()
    REGISTER_SCRIPT_CLASS_FUNCTION(Asteroid, getSize);
}

REGISTER_MULTIPLAYER_CLASS(Asteroid, "Asteroid");
Asteroid::Asteroid()
: SpaceObject(random(110, 130), "Asteroid")
{
    setRotation(random(0, 360));
    rotation_speed = random(0.1, 0.8);
    z = random(-50, 50);
    size = getRadius();
    model_number = irandom(1, 10);
    setRadarSignatureInfo(0.05, 0, 0);

    registerMemberReplication(&z);
    registerMemberReplication(&size);

    PathPlannerManager::getInstance()->addAvoidObject(this, 300);

#if FEATURE_3D_RENDERING
    if (!shader && gl::isAvailable())
    {
        shader = ShaderManager::getShader("shaders/objectShaderBS");
        shader_model_location = glGetUniformLocation(shader->getNativeHandle(), "model");
        shader_model_normal_location = glGetUniformLocation(shader->getNativeHandle(), "model_normal");
    }
#endif
}

void Asteroid::draw3D()
{
#if FEATURE_3D_RENDERING
    if (size != getRadius())
        setRadius(size);

    shader->setUniform("baseMap", *textureManager.getTexture("Astroid_" + string(model_number) + "_d.png"));
    shader->setUniform("specularMap", *textureManager.getTexture("Astroid_" + string(model_number) + "_s.png"));
    sf::Shader::bind(shader);
    glUniformMatrix4fv(shader_model_location, 1, GL_FALSE, glm::value_ptr(getModelMatrix()));
    glUniformMatrix3fv(shader_model_normal_location, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(getModelMatrix())))));
    Mesh* m = Mesh::getMesh("Astroid_" + string(model_number) + ".model");
    m->render();
#endif//FEATURE_3D_RENDERING
}

void Asteroid::drawOnRadar(sf::RenderTarget& window, sf::Vector2f position, float scale, float rotation, bool long_range)
{
    if (size != getRadius())
        setRadius(size);

    sf::Sprite object_sprite;
    textureManager.setTexture(object_sprite, "RadarBlip.png");
    object_sprite.setRotation(getRotation());
    object_sprite.setPosition(position);
    object_sprite.setColor(sf::Color(255, 200, 100));
    float size = getRadius() * scale / object_sprite.getTextureRect().width * 2;
    if (size < 0.2)
        size = 0.2;
    object_sprite.setScale(size, size);
    window.draw(object_sprite);
}

void Asteroid::collide(Collisionable* target, float force)
{
    if (!isServer())
        return;
    P<SpaceObject> hit_object = P<Collisionable>(target);
    if (!hit_object || !hit_object->canBeTargetedBy(nullptr))
        return;

    DamageInfo info(nullptr, DT_Kinetic, getPosition());
    hit_object->takeDamage(35, info);

    P<ExplosionEffect> e = new ExplosionEffect();
    e->setSize(getRadius());
    e->setPosition(getPosition());
    e->setRadarSignatureInfo(0.0, 0.1, 0.2);
    destroy();
}

void Asteroid::setSize(float size)
{
    this->size = size;
    setRadius(size);
}

float Asteroid::getSize()
{
    return size;
}

glm::mat4 Asteroid::getModelMatrix() const
{
    auto asteroid_matrix = glm::translate(SpaceObject::getModelMatrix(), glm::vec3(0.f, 0.f, z));
    asteroid_matrix = glm::rotate(asteroid_matrix, glm::radians(engine->getElapsedTime() * rotation_speed), glm::vec3(0.f, 0.f, 1.f));
    return glm::scale(asteroid_matrix, glm::vec3(getRadius()));
}

#if FEATURE_3D_RENDERING
sf::Shader* VisualAsteroid::shader = nullptr;
int32_t VisualAsteroid::shader_model_location = -1;
int32_t VisualAsteroid::shader_model_normal_location = -1;
#endif

/// An asteroid in space. Outside of hit range, just for visuals.
REGISTER_SCRIPT_SUBCLASS(VisualAsteroid, SpaceObject)
{
    /// Set the radius of this asteroid
    /// The default radius for an VisualAsteroid is between 110 and 130
    /// Example: VisualAsteroid():setSize(50)
    REGISTER_SCRIPT_CLASS_FUNCTION(VisualAsteroid, setSize);
    /// Gets the current radius of this asteroid
    /// Example: local size=VisualAsteroid():getSize()
    REGISTER_SCRIPT_CLASS_FUNCTION(VisualAsteroid, getSize);
}

REGISTER_MULTIPLAYER_CLASS(VisualAsteroid, "VisualAsteroid");
VisualAsteroid::VisualAsteroid()
: SpaceObject(random(110, 130), "VisualAsteroid")
{
    setRotation(random(0, 360));
    rotation_speed = random(0.1, 0.8);
    z = random(300, 800);
    if (random(0, 100) < 50)
        z = -z;

    size = getRadius();
    model_number = irandom(1, 10);

    registerMemberReplication(&z);
    registerMemberReplication(&size);
#if FEATURE_3D_RENDERING
    if (!shader && gl::isAvailable())
    {
        shader = ShaderManager::getShader("shaders/objectShaderBS");
        shader_model_location = glGetUniformLocation(shader->getNativeHandle(), "model");
        shader_model_normal_location = glGetUniformLocation(shader->getNativeHandle(), "model_normal");
    }
#endif
}

void VisualAsteroid::draw3D()
{
#if FEATURE_3D_RENDERING
    if (size != getRadius())
        setRadius(size);

    shader->setUniform("baseMap", *textureManager.getTexture("Astroid_" + string(model_number) + "_d.png"));
    shader->setUniform("specularMap", *textureManager.getTexture("Astroid_" + string(model_number) + "_s.png"));
    sf::Shader::bind(shader);
    glUniformMatrix4fv(shader_model_location, 1, GL_FALSE, glm::value_ptr(getModelMatrix()));
    glUniformMatrix3fv(shader_model_normal_location, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(getModelMatrix())))));
    Mesh* m = Mesh::getMesh("Astroid_" + string(model_number) + ".model");
    m->render();
#endif//FEATURE_3D_RENDERING
}

void VisualAsteroid::setSize(float size)
{
    this->size = size;
    setRadius(size);
    while(fabs(z) < size * 2)
        z *= random(1.2, 2.0);
}

float VisualAsteroid::getSize()
{
    return size;
}

glm::mat4 VisualAsteroid::getModelMatrix() const
{
    auto asteroid_matrix = glm::translate(SpaceObject::getModelMatrix(), glm::vec3(0.f, 0.f, z));
    asteroid_matrix = glm::rotate(asteroid_matrix, glm::radians(engine->getElapsedTime() * rotation_speed), glm::vec3(0.f, 0.f, 1.f));
    return glm::scale(asteroid_matrix, glm::vec3(getRadius()));

}
