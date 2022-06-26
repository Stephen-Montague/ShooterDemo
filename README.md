# ShooterDemo  
#### 3'rd-person shooter demo written in C++ and powered by Unreal 4.26
Will be posting video clips of this project here:   

https://www.youtube.com/channel/UCSR2NmoF5DqlLigxub62pVw

This is a customized recreation of Stephen Ulibarri's course project on Udemy:

https://www.udemy.com/course/unreal-engine-the-ultimate-shooter-course/ 

### Features implemented:

Character creation and movement

- Input for PC and console controllers

- Smoothly run, jump, aim, crouch

- Support turn-in-place, aim offsets, strafing 

- Lean and turn hips for natural running

- Crouch with resized collision capsule 


Animations via Blueprints and C++ Anim Instances

- Using State Machines, Blendspaces, Montage Slots

- Using Anim Notifies to connect animations to Blueprints and C++

- Using Animation Curves, Inverse Kinematics

Blending animations by enum, per bone, by bool, and via slots 

- Play different animations on different body parts at the same time

- Reload weapon animation with internal ammunition update 

- Weapon fires with adjustible recoil animation

- Weapon muzzle flash, impact, smoke trails, bullet-shell ejection particles 

- Sound and visual effects for footsteps, footprints, water splashing 

- Dynamic character footstep audio changes depending on ground surface type or if underwater

Retargeting single animations or entire Animation Blueprints 

- Sharing animations between different character models

Equip and Use Weapons 

- Submachine gun, assault rifle (to add pistol soon)

- Automatic gunfire (continuous firing via button press & hold)

- Moving gun parts during reload animation (rifle magazine, to add pistol slide-action next)

- Aiming via Camera Zoom 

- Dynamic crosshairs : change size based on character state : velocity, jumping, aiming, firing

- Different crosshairs per weapon

Data Tables in BP and C++ to setup:

- Item inventory, character components

- Animated HUD, game world popup UI widgets

- Item names, types, count, rarity

Material shader creation and customization

- Post-process materials for outline FX, underwater FX, darkness FX 

- Dynamic material instances to update materials at runtime

Using various data structures as required:

 - struct, class, enum, array, map, etc.

Efficient actor communication via C++ delegates / Blueprint event dispatchers

Level prototyping with hundreds of objects via organized folders

---

Related:

On another Unreal project... (in a private repo) I've also used Unreal Blueprints to code a fairly fun little science-fiction arcade game.  It uses an overhead twin-stick / mouse-keyboard shooter style, but many of the concepts used here in C++ I developed in Blueprints and then built those into "Nativized" C++ files for performance.  If you'd like to play the [game](https://stephen-montague.itch.io/going-home), please do and feel free to give ideas for further development!  🙂
