#ifndef __VACUUM_AGENT__H
#define __VACUUM_AGENT__H 

#include <string>
#include <math.h>
#include "enviro.h"

int g_battery_level = 25;

namespace
{

    using namespace enviro;
    class VacuumController;


    class Rotating : public State, public AgentInterface
    {
    private:
        double target_angle;
        double angular_speed;
        int count;

    public:
        void entry(const Event &e) 
        {
            track_velocity(0,0);
            count = 0;
            // target_angle = angle() + (M_PI/2 - .06);
            angular_speed = rand() % 2 == 0 ? 5 : -5; // taken from wanderers (rate code)
            std::cout << "State: Rotate\n";
            std::cout << "Angular Speed is " << angular_speed << std::endl;
        }
        void during()
        {
            track_velocity(0, angular_speed);
            //if (angle() >= target_angle) {
            if (count == 10) {
                track_velocity(0, 0);
                if (g_battery_level < 20) {
                    emit(Event("lb_return"));
                } else {
                    emit(Event("rotated"));
                }
            }
            count++;
        }
        void exit(const Event &e) {}

    };

    class Avoid : public State, public AgentInterface
    {
    public:
        void entry(const Event &e) 
        {
            std::cout << "State: Avoid\n";
            track_velocity(0,0);
            emit(Event("turning"));
        }
        void during(){}
        void exit(const Event &e) {}
    };

    class Clean : public State, public AgentInterface
    {
    private:
        int counter;
    
    public:
        void entry(const Event &e) 
        {
            counter = 0;
            std::cout << "State: Clean\n";
            notice_collisions_with("dirt", [&](Event& e) {
                int dirt_id = e.value()["id"];
                if (agent_exists(dirt_id)) {
                    remove_agent(dirt_id);
            }
            });
        }
        void during()
        {
            VacuumController& controller = (VacuumController&) state_machine();
            track_velocity(10, 0);
            // std::cout << "sensor_value(0) < 30: " << (sensor_value(0) < 30) << std::endl;
            // std::cout << "reflection type: " << sensor_reflection_type(0) << std::endl;
            if (sensor_value(0) < 30 && sensor_reflection_type(0) != "dirt") 
            {
                emit(Event("obj_detected_cleaning"));
            }

            if (rand() % 1000 < 5) {
                track_velocity(0,0);
                std::cout << "random turn tirggered\n";
                emit(Event("random_turning"));
            }

            if (g_battery_level < 20) {
                emit(Event("low_battery"));
            }

            if (counter % 20 == 0) {
                g_battery_level--;
                std::cout << "Battery Level is " << g_battery_level << std::endl;
            }

            counter++;

        }
        void exit(const Event &e) {}
    };

    class Charging : public State, public AgentInterface
    {
    public:
        void entry(const Event &e) 
        {
            track_velocity(0,0);
            std::cout << "State: Charging\n";
        }
        void during()
        {
            track_velocity(0,0);
            g_battery_level++;
            std::cout << "Battery Level is " << g_battery_level << std::endl;
            if (g_battery_level >= 100) {
                emit(Event("charged"));
            }
        }
        void exit(const Event &e) {}
    };

    class ReturnToDock : public State, public AgentInterface
    {
    public:
        void entry(const Event &e) 
        {
            std::cout << "State: Returning to Dock\n";
            counter = 0;
        }
        void during()
        {
            move_toward(-275,-125);
            if (sensor_value(0) < 30 || sensor_value(1) < 10 || sensor_value(2) < 10)
            {
                emit(Event("lb_avoid"));
            }
            if ((int)position().x == -275 && (int)position().y == -125) 
            {
                emit(Event("docked"));
            }

            if (counter % 20 == 0) {
                g_battery_level--;
                std::cout << "Battery Level is " << g_battery_level << std::endl;
            }
            counter++;
        }
        void exit(const Event &e) {}

    private:
        int counter;
    };

    class Forward : public State, public AgentInterface
    {
    public:
        void entry(const Event &e) 
        {
            std::cout << "State: Moving Forward\n";

            counter = 0;
        }
        void during()
        {
            track_velocity(5,0);

            std::cout << "vx: " << vx() << ", vy: " << vy() << std::endl;
            if (counter == 10) {
                
                if (std::abs(vx()) < 0.5 && std::abs(vy()) < 0.5 ) {
                    emit(Event("backing up"));
                }
                emit(Event("back on return"));
            }
            counter++;
        }
        void exit(const Event &e) {}

    private:
        int counter;
    };

    class Backward : public State, public AgentInterface
    {
    public:
        void entry(const Event &e) 
        {
            std::cout << "State: Moving Backwards\n";
            counter = 0;
        }
        void during()
        {
            track_velocity(-5,0);
            std::cout << "Counter " << counter << std::endl;
            if (counter == 5) {
                if (g_battery_level < 20){
                    emit(Event("back on return backwards"));
                } else {
                    emit(Event("back to cleaning"));
                }
            }
            counter++;
        }
        void exit(const Event &e) {}

    private:
        int counter;
    };

    class VacuumController : public StateMachine, public AgentInterface
    {

    public:

        VacuumController() : StateMachine()
        {

            set_initial(cleaning);
            add_transition("obj_detected_cleaning", cleaning, avoiding);
            add_transition("turning", avoiding, rotating);
            add_transition("random_turning", cleaning, rotating);
            add_transition("rotated", rotating, cleaning);
            add_transition("low_battery", cleaning, returning);
            add_transition("lb_avoid", returning, avoiding);
            add_transition("lb_return", rotating, forward);
            add_transition("docked", returning, charging);
            add_transition("charged", charging, cleaning);
            add_transition("back on return", forward, returning);

            add_transition("back to cleaning", backward, cleaning);
            add_transition("back on return backwards", backward, returning);
            add_transition("backing up", forward, backward);
        }
        

    private:
        Avoid avoiding;
        Rotating rotating;
        Clean cleaning;
        Charging charging;
        ReturnToDock returning;
        Forward forward;
        Backward backward;

    };


    class Vacuum : public Agent {
        public:
        Vacuum(json spec, World& world) : Agent(spec, world) {
            add_process(c);
        }
        private:
        VacuumController c;
    };

    DECLARE_INTERFACE(Vacuum)

}

#endif