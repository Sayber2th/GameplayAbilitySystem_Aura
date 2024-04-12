//* WORK IN PROGRESS BUILD *//
This project best demonstrates my current skill level.

NOTE - 
1) This project is being made by taking an advanced course on Udemy on the vast and complex Gameplay Ability System within Unreal Engine.
2) This project uses a combination of C++ and Unreal Engine's visual scripting language - Blueprint. All complex functionality is done in C++, while some minor functionality is implemented within Blueprint.
3) All C++ code is available in this repository.
4) This project is being built with multiplayer support in mind.
5) At this point there is no way to exit or restart the game build other than by using the 'Alt + F4' shortcut.


Explanation of what you see in the execution build and the showcase video:-

There are several pick-up items on the map. There are two kinds of pickups(potions and crystals) for health and mana. The potions offer a one-off instant addition to the health/mana reserves, whereas the crystals offer a smaller periodic increase to the health/mana reserves over a certain amount of time.

There are several kinds of enemies on the map. There are two kinds of goblins, one attacks close range with a spear, and another attacks from a long range with a slingshot. There is another enemy with the same model as the goblin but with a mask, named the shaman. This enemy is slower and launches fireballs like the player character.

There is a ghoul, the bigger blue model with the exposed brain. This enemy is a melee attacker with attacks for both left and right hands.

There is a red enemy model with horns named the demon. This enemy is also a melee attacker, but attacks with his tail using a spin attack. This enemy is also slightly faster than others.



//* Controls *//

Movement - Tap left-click on floors to auto-move the character to location; hold left-click to manually move the character in the direction of the mouse cursor.
Shoot fireball - While aiming at and highlighting an enemy, left-click to shoot a fireball targeted at the highlighted enemy. To shoot manually without the need for a highlighted enemy, hold left shift and left-click.



//* Objective *//

The primary objective of this project is to learn about the Gameplay Ability System of Unreal Engine in the context of a multiplayer game. A secondary goal of this project was to improve my overall coding skills and specifically write clean code that is easy to expand upon and easy to understand.



//* Challenges *//

Understanding and setting up expandable code for the multiplayer replication of attributes and tags in Unreal Engine was a big challenge for this project. This in tandem with learning the Gameplay Ability System turned out to be the most challenging project for me so far.
I was learning two completely new concepts with this project. Setting up a game project for multiplayer support, as well as the gameplay ability system in Unreal Engine. The combination of these two concepts both being new was quite a challenge for me. I had to take my time with each lecture that dealt with either or both of these concepts and sometimes replay some sections of the course multiple times.

I faced several technical issues during this project. 

One example would be - While working on this project I had to port over all the project files and the Unreal Engine editor files onto an external SSD from the internal SSD of my laptop. This was to free up some space on the internal SSD of my laptop because it was slowing down my device. During this process, I had to solve an issue that had popped up due to the file pathways being outdated in the '.workspace' file of Visual Studio Code IDE. I had figured out that the issue had something to do with the pathways being outdated, but needed to figure out where to go and switch out the old file pathways and input new ones.



//* Lessons learnt *//

This project/course has taught me many new game development concepts and improved my existing skills. They are as follows:-
1) Learning to debug code.
2) Writing clean and scalable code.
3) Getting better at solving technical issues that may arise while working with coding software.
4) Learning what it takes to set up a game for multiplayer.
5) Improving my workflow for writing code.
6) Learning to organise C++ scripts in appropriate folders for better useability.
