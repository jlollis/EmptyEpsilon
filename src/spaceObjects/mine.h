#ifndef MINE_H
#define MINE_H

#include "spaceObject.h"
#include "modelData.h"

class Mine : public SpaceObject, public Updatable
{
    constexpr static float blastRange = 1000.0f;
    constexpr static float trigger_range = 600.0f;
    constexpr static float triggerDelay = 1.0f;
    constexpr static float damageAtCenter = 160.0f;
    constexpr static float damageAtEdge = 30.0f;

    ScriptSimpleCallback on_destruction;

public:
    P<SpaceObject> owner;
    bool triggered;       //Only valid on server.
    float triggerTimeout; //Only valid on server.
    float ejectTimeout;   //Only valid on server.

    Mine();
    virtual ~Mine();

    virtual void draw3D() override;
    virtual void draw3DTransparent() override;
    virtual void drawOnRadar(sp::RenderTarget& renderer, glm::vec2 position, float scale, float rotation, bool long_range) override;
    virtual void drawOnGMRadar(sp::RenderTarget& renderer, glm::vec2 position, float scale, float rotation, bool long_range) override;
    virtual void update(float delta) override;

    virtual void collide(Collisionable* target, float force) override;
    void eject();
    void explode();
    void onDestruction(ScriptSimpleCallback callback);
    

    P<SpaceObject> getOwner();
    virtual std::unordered_map<string, string> getGMInfo() override;
    virtual string getExportLine() override { return "Mine():setPosition(" + string(getPosition().x, 0) + ", " + string(getPosition().y, 0) + ")"; }
    void setLightsColor(const glm::vec3& color);
    void setLightsPattern(float off_seconds, float on_seconds);

private:
    P<ModelData> model_data;
    const MissileWeaponData& data;
    float lights_off_seconds{ 1.f };
    float lights_timer_seconds{ 0.f };
    float lights_on_seconds{ 0.5f };
    glm::vec3 lights_color_modulation{1.f, 0.f, 0.f};
};

#endif//MINE_H
