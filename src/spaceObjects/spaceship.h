#ifndef SPACESHIP_H
#define SPACESHIP_H
#include <i18n.h>

#include <array>
#include <optional>

#include "shipTemplateBasedObject.h"
#include "spaceStation.h"
#include "spaceshipParts/beamWeapon.h"
#include "spaceshipParts/weaponTube.h"
#include "tween.h"


enum EMainScreenSetting
{
    MSS_Front = 0,
    MSS_Back,
    MSS_Left,
    MSS_Right,
    MSS_Target,
    MSS_Tactical,
    MSS_LongRange
};
template<> void convert<EMainScreenSetting>::param(lua_State* L, int& idx, EMainScreenSetting& mss);

enum EMainScreenOverlay
{
    MSO_HideComms = 0,
    MSO_ShowComms
};
template<> void convert<EMainScreenOverlay>::param(lua_State* L, int& idx, EMainScreenOverlay& mso);

enum EDockingState
{
    DS_NotDocking = 0,
    DS_Docking,
    DS_Docked
};

struct Speeds
{
    float forward;
    float reverse;
};
template<> int convert<Speeds>::returnType(lua_State* L, const Speeds &speeds);


class ShipSystem
{
public:
    static constexpr float power_factor_rate = 0.08f;
    static constexpr float default_heat_rate_per_second = 0.05f;
    static constexpr float default_power_rate_per_second = 0.3f;
    static constexpr float default_coolant_rate_per_second = 1.2f;
    float health; //1.0-0.0, where 0.0 is fully broken.
    float health_max; //1.0-0.0, where 0.0 is fully broken.
    float power_level; //0.0-3.0, default 1.0
    float power_request;
    float heat_level; //0.0-1.0, system will damage at 1.0
    float coolant_level; //0.0-10.0
    float coolant_request;
    float hacked_level; //0.0-1.0
    float power_factor;
    float coolant_rate_per_second{};
    float heat_rate_per_second{};
    float power_rate_per_second{};

    float getHeatingDelta() const
    {
        return powf(1.7f, power_level - 1.0f) - (1.01f + coolant_level * 0.1f);
    }

    float getPowerUserFactor() const
    {
        return power_factor * power_factor_rate;
    }
};

class SpaceShip : public ShipTemplateBasedObject
{
public:
    constexpr static int max_frequency = 20;
    constexpr static float combat_maneuver_charge_time = 20.0f; /*< Amount of time it takes to fully charge the combat maneuver system */
    constexpr static float combat_maneuver_boost_max_time = 3.0f; /*< Amount of time we can boost with a fully charged combat maneuver system */
    constexpr static float combat_maneuver_strafe_max_time = 3.0f; /*< Amount of time we can strafe with a fully charged combat maneuver system */
    constexpr static float warp_charge_time = 4.0f;
    constexpr static float warp_decharge_time = 2.0f;
    constexpr static float jump_drive_charge_time = 90.0f;   /*<Total charge time for the jump drive after a max range jump */
    constexpr static float jump_drive_energy_per_km_charge = 4.0f;
    constexpr static float jump_drive_heat_per_jump = 0.35f;
    constexpr static float heat_per_combat_maneuver_boost = 0.2f;
    constexpr static float heat_per_combat_maneuver_strafe = 0.2f;
    constexpr static float heat_per_warp = 0.02f;
    constexpr static float unhack_time = 180.0f; //It takes this amount of time to go from 100% hacked to 0% hacked for systems.

    float energy_level;
    float max_energy_level;
    ShipSystem systems[SYS_COUNT];
    static std::array<float, SYS_COUNT> default_system_power_factors;
    /*!
     *[input] Ship will try to aim to this rotation. (degrees)
     */
    float target_rotation;

    /*!
     *[input] Ship will rotate in this velocity. ([-1,1], overrides target_rotation)
     */
    float turnSpeed;

    /*!
     * [input] Amount of impulse requested from the user (-1.0 to 1.0)
     */
    float impulse_request;

    /*!
     * [output] Amount of actual impulse from the engines (-1.0 to 1.0)
     */
    float current_impulse;

    /*!
     * [config] Speed of rotation, in deg/second
     */
    float turn_speed;

    /*!
     * [config] Max speed of the impulse engines, in m/s
     */
    float impulse_max_speed;

    /*!
     * [config] Max speed of the reverse impulse engines, in m/s
     */
    float impulse_max_reverse_speed;

    /*!
     * [config] Impulse engine acceleration, in (m/s)/s
     */
    float impulse_acceleration;

    /*!
     * [config] Impulse engine acceleration in reverse, in (m/s)/s
     */
    float impulse_reverse_acceleration;

    /*!
     * [config] True if we have a warpdrive.
     */
    bool has_warp_drive;

    /*!
     * [input] Level of warp requested, from 0 to 4
     */
    int8_t warp_request;

    /*!
     * [output] Current active warp amount, from 0.0 to 4.0
     */
    float current_warp;

    /*!
     * [config] Amount of speed per warp level, in m/s
     */
    float warp_speed_per_warp_level;

    /*!
     * [output] How much charge there is in the combat maneuvering system (0.0-1.0)
     */
    float combat_maneuver_charge;
    /*!
     * [input] How much boost we want at this moment (0.0-1.0)
     */
    float combat_maneuver_boost_request;
    float combat_maneuver_boost_active;

    float combat_maneuver_strafe_request;
    float combat_maneuver_strafe_active;

    float combat_maneuver_boost_speed; /*< [config] Speed to indicate how fast we will fly forwards with a full boost */
    float combat_maneuver_strafe_speed; /*< [config] Speed to indicate how fast we will fly sideways with a full strafe */

    bool has_jump_drive;      //[config]
    float jump_drive_charge; //[output]
    float jump_distance;     //[output]
    float jump_delay;        //[output]
    float jump_drive_min_distance; //[config]
    float jump_drive_max_distance; //[config]
    float wormhole_alpha;    //Used for displaying the Warp-postprocessor

    int weapon_storage[MW_Count];
    int weapon_storage_max[MW_Count];
    int8_t weapon_tube_count;
    WeaponTube weapon_tube[max_weapon_tubes];

    /*!
     * [output] Frequency of beam weapons
     */
    int beam_frequency;
    ESystem beam_system_target;
    BeamWeapon beam_weapons[max_beam_weapons];

    /**
     * Frequency setting of the shields.
     */
    int shield_frequency;

    /// MultiplayerObjectID of the targeted object, or -1 when no target is selected.
    int32_t target_id;

    EDockingState docking_state;
    P<SpaceObject> docking_target; //Server only
    glm::vec2 docking_offset{0, 0}; //Server only

    SpaceShip(string multiplayerClassName, float multiplayer_significant_range=-1);
    virtual ~SpaceShip();

    virtual void draw3DTransparent() override;
    /*!
     * Get this ship's radar signature dynamically modified by the state of its
     * systems and current activity.
     */
    virtual RawRadarSignatureInfo getDynamicRadarSignatureInfo();
    float getDynamicRadarSignatureGravity() { return getDynamicRadarSignatureInfo().gravity; }
    float getDynamicRadarSignatureElectrical() { return getDynamicRadarSignatureInfo().electrical; }
    float getDynamicRadarSignatureBiological() { return getDynamicRadarSignatureInfo().biological; }

    /*!
     * Draw this ship on the radar.
     */
    virtual void drawOnRadar(sp::RenderTarget& renderer, glm::vec2 position, float scale, float rotation, bool long_range) override;
    virtual void drawOnGMRadar(sp::RenderTarget& renderer, glm::vec2 position, float scale, float rotation, bool long_range) override;

    virtual void update(float delta) override;
    virtual float getShieldRechargeRate(int shield_index) override;
    virtual float getShieldDamageFactor(DamageInfo& info, int shield_index) override;
    float getJumpDriveRechargeRate() { return Tween<float>::linear(getSystemEffectiveness(SYS_JumpDrive), 0.0, 1.0, -0.25, 1.0); }

    /*!
     * Check if the ship can be targeted.
     */
    virtual bool canBeTargetedBy(P<SpaceObject> other) override { return true; }

    /*!
     * didAnOffensiveAction is called whenever this ship does something offesive towards an other object
     * this can identify the ship as friend or foe.
     */
    void didAnOffensiveAction();

    /*!
     * Spaceship takes damage directly on hull.
     * This is used when shields are down or by weapons that ignore shields.
     * \param damage_amount Damage to be delt.
     * \param info Information about damage type (usefull for damage reduction, etc)
     */
    virtual void takeHullDamage(float damage_amount, DamageInfo& info) override;

    /*!
     * Spaceship is destroyed by damage.
     * \param info Information about damage type
     */
    virtual void destroyedByDamage(DamageInfo& info) override;

    /*!
     * Jump in current direction
     * \param distance Distance to jump in meters)
     */
    virtual void executeJump(float distance);

    /*!
     * Check if object can dock with this ship.
     * \param object Object that wants to dock.
     */
    virtual bool canBeDockedBy(P<SpaceObject> obj) override;

    virtual void collide(Collisionable* other, float force) override;

    /*!
     * Start the jumping procedure.
     */
    void initializeJump(float distance);

    /*!
     * Request to dock with target.
     */
    void requestDock(P<SpaceObject> target);

    /*!
     * Request undock with current docked object
     */
    void requestUndock();

    /*!
     * Abort the current dock request
     */
    void abortDock();

    /// Dummy virtual function to use energy. Only player ships currently model energy use.
    virtual bool useEnergy(float amount) { return true; }

    /// Dummy virtual function to add heat on a system. The player ship class has an actual implementation of this as only player ships model heat right now.
    virtual void addHeat(ESystem system, float amount) {}

    virtual bool canBeScannedBy(P<SpaceObject> other) override { return getScannedStateFor(other) != SS_FullScan; }
    virtual int scanningComplexity(P<SpaceObject> other) override;
    virtual int scanningChannelDepth(P<SpaceObject> other) override;
    virtual void scannedBy(P<SpaceObject> other) override;
    void setScanState(EScannedState scanned);
    void setScanStateByFaction(string faction_name, EScannedState scanned);

    bool isFriendOrFoeIdentified();//[DEPRICATED]
    bool isFullyScanned();//[DEPRICATED]
    bool isFriendOrFoeIdentifiedBy(P<SpaceObject> other);
    bool isFullyScannedBy(P<SpaceObject> other);
    bool isFriendOrFoeIdentifiedByFaction(int faction_id);
    bool isFullyScannedByFaction(int faction_id);

    virtual bool canBeHackedBy(P<SpaceObject> other) override;
    virtual std::vector<std::pair<ESystem, float> > getHackingTargets() override;
    virtual void hackFinished(P<SpaceObject> source, string target) override;

    /*!
     * Check if ship has certain system
     */
    bool hasSystem(ESystem system);

    /*!
     * Check effectiveness of system.
     * If system has more / less power or is damages, this can influence the effectiveness.
     * \return float 0. to 1.
     */
    float getSystemEffectiveness(ESystem system);

    virtual void applyTemplateValues() override;

    P<SpaceObject> getTarget();

    virtual std::unordered_map<string, string> getGMInfo() override;

    bool isDocked(P<SpaceObject> target) { return docking_state == DS_Docked && docking_target == target; }
    P<SpaceObject> getDockedWith() { if (docking_state == DS_Docked) return docking_target; return NULL; }
    bool canStartDocking() { return current_warp <= 0.0f && (!has_jump_drive || jump_delay <= 0.0f); }
    EDockingState getDockingState() { return docking_state; }
    int getWeaponStorage(EMissileWeapons weapon) { if (weapon == MW_None) return 0; return weapon_storage[weapon]; }
    int getWeaponStorageMax(EMissileWeapons weapon) { if (weapon == MW_None) return 0; return weapon_storage_max[weapon]; }
    void setWeaponStorage(EMissileWeapons weapon, int amount) { if (weapon == MW_None) return; weapon_storage[weapon] = amount; }
    void setWeaponStorageMax(EMissileWeapons weapon, int amount) { if (weapon == MW_None) return; weapon_storage_max[weapon] = amount; weapon_storage[weapon] = std::min(int(weapon_storage[weapon]), amount); }
    float getMaxEnergy() { return max_energy_level; }
    void setMaxEnergy(float amount) { if (amount > 0.0f) { max_energy_level = amount;} }
    float getEnergy() { return energy_level; }
    void setEnergy(float amount) { if ( (amount > 0.0f) && (amount <= max_energy_level)) { energy_level = amount; } }
    float getSystemHackedLevel(ESystem system) { if (system >= SYS_COUNT) return 0.0; if (system <= SYS_None) return 0.0; return systems[system].hacked_level; }
    void setSystemHackedLevel(ESystem system, float hacked_level) { if (system >= SYS_COUNT) return; if (system <= SYS_None) return; systems[system].hacked_level = std::min(1.0f, std::max(0.0f, hacked_level)); }
    float getSystemHealth(ESystem system) { if (system >= SYS_COUNT) return 0.0; if (system <= SYS_None) return 0.0; return systems[system].health; }
    void setSystemHealth(ESystem system, float health) { if (system >= SYS_COUNT) return; if (system <= SYS_None) return; systems[system].health = std::min(1.0f, std::max(-1.0f, health)); }
    float getSystemHealthMax(ESystem system) { if (system >= SYS_COUNT) return 0.0; if (system <= SYS_None) return 0.0; return systems[system].health_max; }
    void setSystemHealthMax(ESystem system, float health_max) { if (system >= SYS_COUNT) return; if (system <= SYS_None) return; systems[system].health_max = std::min(1.0f, std::max(-1.0f, health_max)); }
    float getSystemHeat(ESystem system) { if (system >= SYS_COUNT) return 0.0; if (system <= SYS_None) return 0.0; return systems[system].heat_level; }
    void setSystemHeat(ESystem system, float heat) { if (system >= SYS_COUNT) return; if (system <= SYS_None) return; systems[system].heat_level = std::min(1.0f, std::max(0.0f, heat)); }
    float getSystemHeatRate(ESystem system) const { if (system >= SYS_COUNT) return 0.f; if (system <= SYS_None) return 0.f; return systems[system].heat_rate_per_second; }
    void setSystemHeatRate(ESystem system, float rate) { if (system >= SYS_COUNT) return; if (system <= SYS_None) return; systems[system].heat_rate_per_second = rate; }

    float getSystemPower(ESystem system) { if (system >= SYS_COUNT) return 0.0; if (system <= SYS_None) return 0.0; return systems[system].power_level; }
    void setSystemPower(ESystem system, float power) { if (system >= SYS_COUNT) return; if (system <= SYS_None) return; systems[system].power_level = std::min(3.0f, std::max(0.0f, power)); }
    float getSystemPowerRate(ESystem system) const { if (system >= SYS_COUNT) return 0.f; if (system <= SYS_None) return 0.f; return systems[system].power_rate_per_second; }
    void setSystemPowerRate(ESystem system, float rate) { if (system >= SYS_COUNT) return; if (system <= SYS_None) return; systems[system].power_rate_per_second = rate; }
    float getSystemPowerUserFactor(ESystem system) { if (system >= SYS_COUNT) return 0.f; if (system <= SYS_None) return 0.f; return systems[system].getPowerUserFactor(); }
    float getSystemPowerFactor(ESystem system) { if (system >= SYS_COUNT) return 0.f; if (system <= SYS_None) return 0.f; return systems[system].power_factor; }
    void setSystemPowerFactor(ESystem system, float factor) { if (system >= SYS_COUNT) return; if (system <= SYS_None) return; systems[system].power_factor = factor; }
    float getSystemCoolant(ESystem system) { if (system >= SYS_COUNT) return 0.0; if (system <= SYS_None) return 0.0; return systems[system].coolant_level; }
    void setSystemCoolant(ESystem system, float coolant) { if (system >= SYS_COUNT) return; if (system <= SYS_None) return; systems[system].coolant_level = std::min(1.0f, std::max(0.0f, coolant)); }
    Speeds getImpulseMaxSpeed() {return {impulse_max_speed, impulse_max_reverse_speed};}
    void setImpulseMaxSpeed(float forward_speed, std::optional<float> reverse_speed) 
    { 
        impulse_max_speed = forward_speed; 
        impulse_max_reverse_speed = reverse_speed.value_or(forward_speed);
    }
    float getSystemCoolantRate(ESystem system) const { if (system >= SYS_COUNT) return 0.f; if (system <= SYS_None) return 0.f; return systems[system].coolant_rate_per_second; }
    void setSystemCoolantRate(ESystem system, float rate) { if (system >= SYS_COUNT) return; if (system <= SYS_None) return; systems[system].coolant_rate_per_second = rate; }
    float getRotationMaxSpeed() { return turn_speed; }
    void setRotationMaxSpeed(float speed) { turn_speed = speed; }
    Speeds getAcceleration() { return {impulse_acceleration, impulse_reverse_acceleration};}
    void setAcceleration(float acceleration, std::optional<float> reverse_acceleration) 
    { 
        impulse_acceleration = acceleration; 
        impulse_reverse_acceleration = reverse_acceleration.value_or(acceleration);
    }
    void setCombatManeuver(float boost, float strafe) { combat_maneuver_boost_speed = boost; combat_maneuver_strafe_speed = strafe; }
    bool hasJumpDrive() { return has_jump_drive; }
    void setJumpDrive(bool has_jump) { has_jump_drive = has_jump; }
    void setJumpDriveRange(float min, float max) { jump_drive_min_distance = min; jump_drive_max_distance = max; }
    bool hasWarpDrive() { return has_warp_drive; }
    void setWarpDrive(bool has_warp)
    {
        has_warp_drive = has_warp;
        if (has_warp_drive)
        {
            if (warp_speed_per_warp_level < 100.0f)
                warp_speed_per_warp_level = 1000.0f;
        }else{
            warp_request = 0;
            warp_speed_per_warp_level = 0;
        }
    }
    void setWarpSpeed(float speed) { warp_speed_per_warp_level = std::max(0.0f, speed); }
    float getWarpSpeed() {
        if (has_warp_drive) {
            return warp_speed_per_warp_level;
        } else {
            return 0.0f;
        }
     }
    float getJumpDriveCharge() { return jump_drive_charge; }
    void setJumpDriveCharge(float charge) { jump_drive_charge = charge; }
    float getJumpDelay() { return jump_delay; }

    float getBeamWeaponArc(int index) { if (index < 0 || index >= max_beam_weapons) return 0.0; return beam_weapons[index].getArc(); }
    float getBeamWeaponDirection(int index) { if (index < 0 || index >= max_beam_weapons) return 0.0; return beam_weapons[index].getDirection(); }
    float getBeamWeaponRange(int index) { if (index < 0 || index >= max_beam_weapons) return 0.0; return beam_weapons[index].getRange(); }

    float getBeamWeaponTurretArc(int index)
    {
        if (index < 0 || index >= max_beam_weapons)
            return 0.0;
        return beam_weapons[index].getTurretArc();
    }

    float getBeamWeaponTurretDirection(int index)
    {
        if (index < 0 || index >= max_beam_weapons)
            return 0.0;
        return beam_weapons[index].getTurretDirection();
    }

    float getBeamWeaponTurretRotationRate(int index)
    {
        if (index < 0 || index >= max_beam_weapons)
            return 0.0;
        return beam_weapons[index].getTurretRotationRate();
    }

    float getBeamWeaponCycleTime(int index) { if (index < 0 || index >= max_beam_weapons) return 0.0; return beam_weapons[index].getCycleTime(); }
    float getBeamWeaponDamage(int index) { if (index < 0 || index >= max_beam_weapons) return 0.0; return beam_weapons[index].getDamage(); }
    float getBeamWeaponEnergyPerFire(int index) { if (index < 0 || index >= max_beam_weapons) return 0.0; return beam_weapons[index].getEnergyPerFire(); }
    float getBeamWeaponHeatPerFire(int index) { if (index < 0 || index >= max_beam_weapons) return 0.0; return beam_weapons[index].getHeatPerFire(); }

    int getShieldsFrequency(void){ return shield_frequency; }
    void setShieldsFrequency(int freq) { if ((freq > SpaceShip::max_frequency) || (freq < 0)) return; shield_frequency = freq;}

    int getBeamFrequency(){ return beam_frequency; }

    void setBeamWeapon(int index, float arc, float direction, float range, float cycle_time, float damage)
    {
        if (index < 0 || index >= max_beam_weapons)
            return;
        beam_weapons[index].setArc(arc);
        beam_weapons[index].setDirection(direction);
        beam_weapons[index].setRange(range);
        beam_weapons[index].setCycleTime(cycle_time);
        beam_weapons[index].setDamage(damage);
    }

    void setBeamWeaponTurret(int index, float arc, float direction, float rotation_rate)
    {
        if (index < 0 || index >= max_beam_weapons)
            return;
        beam_weapons[index].setTurretArc(arc);
        beam_weapons[index].setTurretDirection(direction);
        beam_weapons[index].setTurretRotationRate(rotation_rate);
    }

    void setBeamWeaponTexture(int index, string texture)
    {
        if (index < 0 || index >= max_beam_weapons)
            return;
        beam_weapons[index].setBeamTexture(texture);
    }

    void setBeamWeaponEnergyPerFire(int index, float energy) { if (index < 0 || index >= max_beam_weapons) return; beam_weapons[index].setEnergyPerFire(energy); }
    void setBeamWeaponHeatPerFire(int index, float heat) { if (index < 0 || index >= max_beam_weapons) return; beam_weapons[index].setHeatPerFire(heat); }
    void setBeamWeaponArcColor(int index, float r, float g, float b, float fire_r, float fire_g, float fire_b) {
        if (index < 0 || index >= max_beam_weapons) return;
        beam_weapons[index].setArcColor(glm::u8vec4(r * 255, g * 255, b * 255, 128));
        beam_weapons[index].setArcFireColor(glm::u8vec4(fire_r * 255, fire_g * 255, fire_b * 255, 255));
    }
    void setBeamWeaponDamageType(int index, EDamageType type) { if (index < 0 || index >= max_beam_weapons) return; beam_weapons[index].setDamageType(type); }

    void setWeaponTubeCount(int amount);
    int getWeaponTubeCount();
    EMissileWeapons getWeaponTubeLoadType(int index);

    void weaponTubeAllowMissle(int index, EMissileWeapons type);
    void weaponTubeDisallowMissle(int index, EMissileWeapons type);
    void setWeaponTubeExclusiveFor(int index, EMissileWeapons type);
    void setWeaponTubeDirection(int index, float direction);
    void setTubeSize(int index, EMissileSizes size);
    EMissileSizes getTubeSize(int index);
    void setTubeLoadTime(int index, float time);
    float getTubeLoadTime(int index);

    void setRadarTrace(string trace) { radar_trace = "radar/" + trace; }

    void addBroadcast(int threshold, string message);

    // Return a string that can be appended to an object create function in the lua scripting.
    // This function is used in getScriptExport calls to adjust for tweaks done in the GM screen.
    string getScriptExportModificationsOnTemplate();
    
};

float frequencyVsFrequencyDamageFactor(int beam_frequency, int shield_frequency);

string getMissileWeaponName(EMissileWeapons missile);
string getLocaleMissileWeaponName(EMissileWeapons missile);
REGISTER_MULTIPLAYER_ENUM(EMissileWeapons);
REGISTER_MULTIPLAYER_ENUM(EWeaponTubeState);
REGISTER_MULTIPLAYER_ENUM(EMainScreenSetting);
REGISTER_MULTIPLAYER_ENUM(EMainScreenOverlay);
REGISTER_MULTIPLAYER_ENUM(EDockingState);
REGISTER_MULTIPLAYER_ENUM(EScannedState);

string frequencyToString(int frequency);

#endif//SPACESHIP_H
