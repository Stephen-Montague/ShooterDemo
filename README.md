# ShooterDemo  
#### 3'rd-person shooter demo written in C++ and powered by Unreal 4.26
#### A customized recreation of Stephen Ulibarri's course project on Udemy:
#### https://www.udemy.com/course/unreal-engine-the-ultimate-shooter-course/ 

### Features implemented:

Character creation and movement

- Input for PC and console controllers

- Smoothly run, jump, aim, crouch

- Support turn-in-place, aim offsets, strafing 

- Lean and turn hips for natural running

- Crouch with resized collision capsule 


Animations via Blueprints and C++ Anim Instances

- Using State Machines, Blendspaces, Montage Slots, Animation Curves, Inverse Kinematics

- Reload weapon animation with UI and internal model updates 

- Weapon fire with recoil animation

- Weapon muzzle flash, impact particles, smoke trails, bullet shell ejection particles 

- Sound and visual effect cues (footstep, footprints, splash, etc.) via Anim Notifies to BP or C++

- Dynamic character footsteps change depending on ground surface type (or if underwater, etc.)

Blending animations by enum, per bone, by bool and via montage slots 

- Play different animations on different body parts at the same time

Equip and Use Weapons 

- Submachine gun, assault rifle (to add pistol soon).

- Automatic gunfire (continuous firing via button press & hold).

- Moving gun parts during reload animation (rifle magazine, to add pistol slide-action next)

- Aiming via Camera Zoom 

- Dynamic crosshairs : change size based on character state : velocity, jumping, aiming, firing

- Different crosshairs per weapon

Data Tables in BP and C++ to setup:

- Item inventory, character components

- Animated HUD, game world popup UI widgets

- Item names, types, count, rarity

Material Shader creation and customization, including:

- Post-process materials for outline FX, underwater FX, darkness FX 

- Dynamic material instances to change materials at runtime

Retargeting single animations or entire Animation Blueprints 

- Sharing animations between different character models. 

Using various data structures as required:

 - struct, class, enum, array, map

Efficient actor communication via C++ delegates / Blueprint event dispatchers

Level prototyping with hundreds of objects via organized folders
