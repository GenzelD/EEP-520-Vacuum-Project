#ifndef __DIRT_AGENT__H
#define __DIRT_AGENT__H 

#include "enviro.h"

using namespace enviro;

class dirtController : public Process, public AgentInterface {

    public:
    dirtController() : Process(), AgentInterface() {}

    void init() {}
    void start() {}
    void update() {}
    void stop() {}

};

class dirt : public Agent {
    public:
    dirt(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
    private:
    dirtController c;
};

DECLARE_INTERFACE(dirt)

#endif