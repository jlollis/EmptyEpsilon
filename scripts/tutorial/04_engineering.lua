-- Name: Engineering
-- Description: [Station Tutorial]
--- -------------------
--- -Goes over controlling the power of each station and repairs.
---
--- [Station Info]
--- -------------------
---Power Management: 
--- -The Engineering officer can route power to systems by selecting a system and moving its power slider. Giving a system more power increases its output. For instance, an overpowered reactor produces more energy, overpowered shields reduce more damage and regenerate faster, and overpowered impulse engines increase its maximum speed. Overpowering a system (above 100%) also increases its heat generation and, except for the reactor, its energy draw. Underpowering a system (below 100%) likewise reduces heat output and energy draw.
---
---Coolant Management: 
--- -By adding coolant to a system, the Engineering officer can reduce its temperature and prevent the system from damaging the ship. The ship has an unlimited reseve of coolant, but a finite amount of coolant can be applied at any given time, so the Engineering officer must budget how much coolant each system can receive. A system's change in temperature is indicated by white arrows in the temperature column. The brighter an arrow is, the larger the trend.
---
---Repairs: 
--- -When systems are damaged by being shot, colliding with space hazards, or overheating, the Engineering officer can dispatch repair crews to the system for repairs. Each systems has a damage state between -100% to 100%. Systems below 100% function suboptimally, in much the same way as if they are underpowered. Once a system is at or below 0%, it completely stops functioning until it is repaired. Systems can be repaired by sending a repair crew to the room containing the system. Hull damage affects the entire ship, and docking at a station can repair it, but hull repairs progress very slowly.
-- Type: Tutorial

require("tutorial/00_all.lua")

function init()
    tutorial_list = {
        engineeringTutorial,
        endOfTutorial
    }
    startTutorial()
end
