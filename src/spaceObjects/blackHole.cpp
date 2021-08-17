#include <GL/glew.h> 

#include "blackHole.h"
#include "pathPlanner.h"
#include "main.h"
#include <SFML/OpenGL.hpp>

#include "scriptInterface.h"
#include "glObjects.h"
#include "shaderRegistry.h"


#if FEATURE_3D_RENDERING
struct VertexAndTexCoords
{
    glm::vec3 vertex;
    glm::vec2 texcoords;
};
#endif

/// A blackhole has a 5km radius where it pulls in all near objects. At the center of the black hole everything gets a lot of damage.
/// Which will lead to the eventual destruction of said object.
REGISTER_SCRIPT_SUBCLASS(BlackHole, SpaceObject)
{
}

REGISTER_MULTIPLAYER_CLASS(BlackHole, "BlackHole");
BlackHole::BlackHole()
: SpaceObject(5000, "BlackHole")
{
    update_delta = 0.0;
    PathPlannerManager::getInstance()->addAvoidObject(this, 7000);
    setRadarSignatureInfo(0.9, 0, 0);
}

void BlackHole::update(float delta)
{
    update_delta = delta;
}

#if FEATURE_3D_RENDERING
void BlackHole::draw3DTransparent()
{
    static std::array<VertexAndTexCoords, 4> quad{
        glm::vec3{}, {0.f, 1.f},
        glm::vec3{}, {1.f, 1.f},
        glm::vec3{}, {1.f, 0.f},
        glm::vec3{}, {0.f, 0.f}
    };

    glBindTexture(GL_TEXTURE_2D, textureManager.getTexture("blackHole3d.png")->getNativeHandle());
    ShaderRegistry::ScopedShader shader(ShaderRegistry::Shaders::Billboard);

    glUniform4f(shader.get().uniform(ShaderRegistry::Uniforms::Color), 1.f, 1.f, 1.f, 5000.f);
    gl::ScopedVertexAttribArray positions(shader.get().attribute(ShaderRegistry::Attributes::Position));
    gl::ScopedVertexAttribArray texcoords(shader.get().attribute(ShaderRegistry::Attributes::Texcoords));

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glVertexAttribPointer(positions.get(), 3, GL_FLOAT, GL_FALSE, sizeof(VertexAndTexCoords), (GLvoid*)quad.data());
    glVertexAttribPointer(texcoords.get(), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAndTexCoords), (GLvoid*)((char*)quad.data() + sizeof(glm::vec3)));

    std::initializer_list<uint8_t> indices = { 0, 2, 1, 0, 3, 2 };
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, std::begin(indices));
    glBlendFunc(GL_ONE, GL_ONE);
}
#endif

void BlackHole::drawOnRadar(sp::RenderTarget& renderer, glm::vec2 position, float scale, float rotation, bool long_range)
{
    float size = getRadius() * scale * 2;
    renderer.drawSprite("radar/blackHole.png", position, size, glm::u8vec4(64, 64, 255, 255));
    renderer.drawSprite("radar/blackHole.png", position, size, glm::u8vec4(0, 0, 0, 255));
}

void BlackHole::collide(Collisionable* target, float collision_force)
{
    if (update_delta == 0.0)
        return;

    P<SpaceObject> obj = P<Collisionable>(target);
    if (!obj) return;
    if (!obj->hasWeight()) { return; } // the object is not affected by gravitation

    auto diff = getPosition() - target->getPosition();
    float distance = glm::length(diff);
    float force = (getRadius() * getRadius() * 50.0f) / (distance * distance);
    DamageInfo info(NULL, DT_Kinetic, getPosition());
    if (force > 10000.0)
    {
        force = 10000.0;
        if (isServer())
        {
            obj->takeDamage(100000.0, info); //try to destroy the object by inflicting a huge amount of damage
            if (target)
            {
                target->destroy();
                return;
            }
        }
    }
    if (force > 100.0 && isServer())
    {
        obj->takeDamage(force * update_delta / 10.0f, info);
    }
    if (!obj) {return;}
    obj->setPosition(obj->getPosition() + diff / distance * update_delta * force);
}
