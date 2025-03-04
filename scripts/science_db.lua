--[[
	Everything in the science database files is just readable data for the science officer.
	This data does not affect the game in any other way and just contributes to the lore.
--]]
space_objects = ScienceDatabase():setName(_('Natural'))
item = space_objects:addEntry(_('Asteroid'))
item:setLongDescription(_([[Asteroids are minor planets, usually smaller than a few kilometers. Larger variants are sometimes refered to as planetoids.]]))
item:setImage("redicule2.png")
item = space_objects:addEntry(_('Nebula'))
item:setLongDescription(_([[Nebulae are the birthing places of new stars. These gas fields, usually created by the death of an old star, slowly form new stars due to the gravitational pull of its gas molecules. Because of the ever-changing nature of gas nebulae, most radar and scanning technologies are unable to penetrate them. Science officers are therefore advised to rely on probes and visual observations.]]))

item = space_objects:addEntry(_('Black hole'))
item:setLongDescription(_([[A black hole is a point of supercondensed mass with a gravitational pull so powerful that not even light can escape it. It has no locally detectable features, and can only be seen indirectly by blocking the view and distorting its surroundings, creating a strange circular mirror image of the galaxy. The black disc in the middle marks the event horizon, the boundary where even light can't escape it anymore. 
	
On the sensors, a black hole appears as a disc indicating the zone where the gravitational pull is getting dangerous, and soon will be stronger then the ship's impulse engines. An object that crosses a black hole is drawn toward its center and quickly ripped apart by the gravitational forces.]]))

item = space_objects:addEntry(_('Wormhole'))
item:setLongDescription(_([[A wormhole, also known as an Einstein-Rosen bridge, is a phenomena that connects two points of spacetime. Jump drives operate in a similar fashion, but instead of being created at will, a wormhole occupies a specific location in space. Objects that enter a wormhole instantaneously emerge from the other end, which might be anywhere from a few feet to thousands of light years away. 

Wormholes are rare, and most can move objects in only one direction. Traversable wormholes, which are stable and allow for movement in both directions, are even rarer. All wormholes generate tremendous sensor activity, which an astute science officer can detect even through disruptions such as nebulae.]]))

weapons = ScienceDatabase():setName(_('Weapons'))
weapons:setLongDescription(_([[This database only covers the basic versions of the missiles used throughout the galaxy.

It has been reported that some battleships started using larger variations of those missiles. Small fighters and even frigates should not have too much trouble dodging them, but space captains of bigger ships should be wary of their doubled damage potential.

Smaller variations of these missiles have become common in the galaxy, too. Fighter pilots praise their speed and maneuverability, because it gives them an edge against small and fast-moving targets. They only deal half the damage of their basic counterparts, but what good is a missile if it does not hit its target.]]))

item = weapons:addEntry(_('Homing missile'))
item:addKeyValue(_('Range'), '5.4u')
item:addKeyValue(_('Damage'), '35')
item:setLongDescription(_([[This target-seeking missile is the workhorse of many space combat arsenals. It's compact enough to be fitted on frigates, and packs enough punch to be used on larger ships, though usually in more than a single missile tube.]]))

item = weapons:addEntry(_('Nuke'))
item:addKeyValue(_('Range'), '5.4u')
item:addKeyValue(_('Blast radius'), '1u')
item:addKeyValue(_('Damage at center'), '160')
item:addKeyValue(_('Damage at edge'), '30')
item:setLongDescription(_([[A nuclear missile is similar to a homing missile in that it can seek a target, but it moves and turns more slowly and explodes a greatly increased payload. Its nuclear explosion spans 1U of space and can take out multiple ships in a single shot.

Some captains oppose the use of nuclear weapons because their large explosions can lead to 'fragging', or unintentional friendly fire. Shields should protect crews from harmful radiation, but because these weapons are often used in the thick of battle, there's no way of knowing if hull plating or shields can provide enough protection.]]))

item = weapons:addEntry(_('Mine'))
item:addKeyValue(_('Drop distance'), '1u')
item:addKeyValue(_('Trigger distance'), '0.8u')
item:addKeyValue(_('Blast radius'), '1u')
item:addKeyValue(_('Damage at center'), '160')
item:addKeyValue(_('Damage at edge'), '30')
item:setLongDescription(_([[Mines are often placed in defensive perimeters around stations. There are also old minefields scattered around the galaxy from older wars.

Some fearless captains use mines as offensive weapons, but their delayed detonation and blast radius make this use risky at best.]]))

item = weapons:addEntry(_('EMP'))
item:addKeyValue(_('Range'), '5.4u')
item:addKeyValue(_('Blast radius'), '1u')
item:addKeyValue(_('Damage at center'), '160')
item:addKeyValue(_('Damage at edge'), '30')
item:setLongDescription(_([[The electromagnetic pulse missile (EMP) reproduces the disruptive effects of a nuclear explosion, but without the destructive properties. This causes it to only affect shields within its blast radius, leaving their hulls intact. The EMP missile is also smaller and easier to store than heavy nukes. Many captains (and pirates) prefer EMPs over nukes for these reasons, and use them to knock out targets' shields before closing to disable them with focused beam fire.]]))

item = weapons:addEntry(_('HVLI'))
item:addKeyValue(_('Range'), '5.4u')
item:addKeyValue(_('Damage'), '6 each, 30 total')
item:addKeyValue(_('Burst'), '5')
item:setLongDescription(_([[A high-velocity lead impactor (HVLI) fires a simple slug of lead at a high velocity. This weapon is usually found in simpler ships since it does not require guidance computers. This also means its projectiles fly in a straight line from its tube and can't pursue a target.

Each shot from an HVLI fires a burst of 5 projectiles, which increases the chance to hit but requires precision aiming to be effective. It reaches its full damage potential at a range of 2u.]]))
