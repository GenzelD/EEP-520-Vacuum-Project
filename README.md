# EEP-520 Vacuum Project

# Project Overview
This project will recreate the older versions of a Roomba by utilizing "random walk" to collect dirt around the arena it is placed in. When the battery level is low, it will move to the dock to charge.
## Goal
The goal of the project is to utilize Enviro and Elma to learn state machines and it's embedded software application by simulating a robot. This code uses Enviro which was developed by Professor Eric Klavins (https://github.com/klavinslab/enviro).
## Directory Structure
Within the "vacuum_project" folder, you will be able to find the code used to create this project. Under the "defs" folder, the agent (interactable entities) definitions will be listed in JSON format. In the "lib" folder, the .so files for each agent will be found. These are the shared object libraries which are created when the code is compiled. Under "src", the source code for the agents will be available. Both header and .cc files will be available, although most of the code will be contained in the header files. Finally, the Makefile is contained here and is the Makefile that will be used to compile and run the code (use this Makefile when running the code, not the ones inside the lower folders).

# Key Challenges and Solutions
Challenges that were encountered in this project will be listed here.

## Environment Configuration
The project heavily relies on Docker and you will need to utilize Docker to run the code. My main issue is that only a certain version of enviro Docker container works on my laptop (v1.01). The latest version at this point (v1.61) was available on my desktop PC. Therefore, most of my work was done on my desktop.

The solution to this issue was to first test all of the versions by pulling that version using 
```bash
docker run -p80:80 -p8765:8765 -v ${PWD}:/source -it klavins/enviro:v1.1 bash
```
replace "v1.1" with the versions you would like to test. The issue was that for most of the versions I tried, elma was not functioning correctly and I got errors specifying that "error: 'elma' is not a namespace-name".

## Vacuum Turning Logic
My first big implementation hardship was figuring out how to turn the robot. There were 3 different ways that I implemented the robot. The first was to purely use the sensor and change states from "Rotating" to "Cleaning" once a minimum distance from the front of the robot to the object was reached. Second was the use of angles specifically the condition "angle() >= target_angle" where target_angle was an angle specifed based on its current angle and angle() is the enviro function that gets the current angle of the robot. The issue here is that is was difficult to implement the condition to change states if the turn is randomly left or right.

My third implementation was the solution I ended up with. I created an "int counter" variable that increments each time the during() code in the Rotating state runs. This give me control on how much I want the robot to turn and the ability to randomly choose left or right turns.

## Dirt agents triggered Avoid state
Another issue I tackled was that the dirt agents were implemented to be destroyed when the robot collided with it. Although, initially, the robot was treating these agents as obstacles. The robot was avoiding the agents it was supposed to be destroying.

To fix this, the front sensor needed to detect that the thing infront of it was dirt and not turn. With the condition (!= "dirt"), this behavior was fixed.

# Install and Build
## Prerequisites
Have the Docker software installed and ready to be used. This will go over how to obtain the container.
## Build Steps
1. Open your "terminal"
2. Navigate to your working directory
3. Use the command below to create and run the container that contains the enviro and elma code (this is for windows powershell):
```bash
docker run -p80:80 -p8765:8765 -v ${PWD}:/source -it klavins/enviro:v1.1 bash
```
4. Once the pulls are complete, your terminal should look something like this:
```bash
root@000000ffffff:/source#
```

# Run and Use
## Run Default Simulation
In this section, the steps of how to run the code in this project will be shown.
## Steps
1. Add the "vacuum_project" folder within your working directory.
2. Enter your enviro container, your terminal should look like this:
```bash
root@000000ffffff:/source#
```
3. Navigate to the vacuum_project and enter that folder. Your "ls" command should print
```bash
Makefile  config.json  defs  lib  src
```
4. enter these commands (one after the other, not together)
```bash
make
enviro
```
5. Open your browser and place `http://localhost`
6. You should see the robot interacting with the environment now, to exit, go back to your terminal and `Shift + C`


# Sources
### Enviro GitHub
https://github.com/klavinslab/enviro
### Elma GitHub
https://github.com/klavinslab/elma
### Lecture READMEs and Code
(specifically Week 8 and Week 9 including wanderers code)
https://github.com/sosper30/520win26/tree/main
### Chipmunk documentation
https://chipmunk-physics.net/documentation.php
### math.h documentation
https://cplusplus.com/reference/cmath/
### Elma documentation
https://klavins.github.io/ECEP520/index.html


# Notes
## Future Improvements
For the future I would like to implement a counter that will count the number of dirt objects so that the vacuum will need to unload once it's full. It would be cool to add and aspect of a "human" walking around via using your mouse to immitate a human and see how the robot reacts to that.
