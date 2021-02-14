#ifndef ASTEROID_H
#define ASTEROID_H

#include "spaceObject.h"

class Asteroid : public SpaceObject
{
#if FEATURE_3D_RENDERING
    static sf::Shader* shader;
    static int32_t shader_model_location;
#endif
public:
    float rotation_speed;
    float z;
    float size;
    int model_number;

    Asteroid();

    virtual void draw3D() override;

    virtual void drawOnRadar(sf::RenderTarget& window, sf::Vector2f position, float scale, float rotation, bool long_range) override;

    virtual void collide(Collisionable* target, float force) override;

    void setSize(float size);
    float getSize();

    virtual string getExportLine() override { return "Asteroid():setPosition(" + string(getPosition().x, 0) + ", " + string(getPosition().y, 0) + ")" + ":setSize(" + string(getSize(),0) + ")"; }

private:
    glm::mat4 getModelMatrix() const override;
};

class VisualAsteroid : public SpaceObject
{
#if FEATURE_3D_RENDERING
    static sf::Shader* shader;
    static int32_t shader_model_location;
#endif
public:
    float rotation_speed;
    float z;
    float size;
    int model_number;

    VisualAsteroid();

    virtual void draw3D() override;

    void setSize(float size);
    float getSize();

    virtual string getExportLine() override { return "VisualAsteroid():setPosition(" + string(getPosition().x, 0) + ", " + string(getPosition().y, 0) + ")" ":setSize(" + string(getSize(),0) + ")"; }

private:
    glm::mat4 getModelMatrix() const override;

};

#endif//ASTEROID_H
