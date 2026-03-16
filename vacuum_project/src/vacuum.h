#ifndef __VACUUM_AGENT__H
#define __VACUUM_AGENT__H 

#include <string>
#include <math.h>
#include "enviro.h"

int g_battery_level = 25; // global variable for battery due to dependency issue (see README)

namespace
{
    using namespace enviro;
    class VacuumController;

    /**
     * @class Rotating
     * @brief This is the class for the state that makes the robot turn.
     *
     * This class inherits the State and AgentInterface class. The class uses
     * the State class form of having an entry(), during(), and exit(). The entry
     * code will run when the State is first entered and will loop through during 
     * for the rest of the time.
     * 
     * The class will rotate the robot by using track_velocity() from the AgentInterface class.
     * Each turn will be a random selection of either a right of left turn and will last 10 executions
     * of the during() loop. 
     * 
     */
    class Rotating : public State, public AgentInterface
    {
    private:
        /** 
         * angular_speed: This will be used to choose a negative or positive speed that
         *                will determine if the robot will turn left or right.
         * count: This will be used to keep track of how many times the code loop has executed
         *        it will be utilized as a "timer"
        */
        double angular_speed;
        int count;

    public:
        /**
         * The entry code will initiate the count to be 0 and will determine if the turn is left or right
         */
        void entry(const Event &e) 
        {
            track_velocity(0,0);
            count = 0;
            angular_speed = rand() % 2 == 0 ? 5 : -5; // taken from wanderers (rate code)
            std::cout << "State: Rotate\n";
            std::cout << "Angular Speed is " << angular_speed << std::endl;
        }

        /**
         * The during code will stop the robot's forward speed and apply the angular speed to make it turn.
         * It will last 10 executions then either move to the Forward state or Cleaning state. Forward state
         * will be triggered if the battery is low. 
         */
        void during()
        {
            track_velocity(0, angular_speed);
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


    /**
     * @class Avoid
     * @brief This is the class for the state that will be entered if the robot detects an obstacle.
     *
     * This class inherits the State and AgentInterface class. The class uses
     * the State class form of having an entry(), during(), and exit(). The entry
     * code will run when the State is first entered and will loop through during 
     * for the rest of the time.
     * 
     * The class will lead the robot into a turn after an object detection. This was implemented to allow future 
     * states to be implemented to trigger different turns, in this implementation, this state is redundant. 
     * 
     */
    class Avoid : public State, public AgentInterface
    {
    public:
        /** 
         * The entry code will print out the state and stop the robot's movement in preperation for the turn.
         * It will transition the robot into the turn state.
        */
        void entry(const Event &e) 
        {
            std::cout << "State: Avoid\n";
            track_velocity(0,0);
            emit(Event("turning"));
        }
        void during(){}
        void exit(const Event &e) {}
    };

    /**
     * @class Clean
     * @brief This is the class will be the default state of the robot, it will move and collect dirt during this state
     *
     * This class inherits the State and AgentInterface class. The class uses
     * the State class form of having an entry(), during(), and exit(). The entry
     * code will run when the State is first entered and will loop through during 
     * for the rest of the time.
     * 
     * While this state is active, the robot battery will drain, the robot will use it's front sensor to detect obstacles
     * but ignore the dirt so that it can "pick it up"
     * 
     */
    class Clean : public State, public AgentInterface
    {
    private:
        /** 
         * counter: This will be used to keep track of how many times the code loop has executed
         *        it will be utilized as a "timer"
        */
        int counter;
    
    public:
        /**
         * The entry code will print the current state. It will also set up the code to tell the robot to notice
         * collisions with the dirt agent in order to "pick it up" aka removing the agent.
         */
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
        /**
         * The during code will handle the deleting of the dirt agents when a collision happens. It will
         * also transition the robot into the Avoiding state if the sensor detects and object that is not 
         * "dirt". The robot will transition into a "Returning" state if the battery is low. Random turns will
         * also be triggered based on a random number to ensure that the robot will cover the area. Battery level
         * will also be draining in this state.
         */
        void during()
        {
            track_velocity(10, 0);
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


    /**
     * @class Charging
     * @brief This is the class for the state that makes the robot stay in place and charge.
     *
     * This class inherits the State and AgentInterface class. The class uses
     * the State class form of having an entry(), during(), and exit(). The entry
     * code will run when the State is first entered and will loop through during 
     * for the rest of the time.
     * 
     * The class will keep the robot on the charging dock and increase it's battery as it stays.
     * 
     */
    class Charging : public State, public AgentInterface
    {
    public:
        /**
         * The state will be printed out
         */
        void entry(const Event &e) 
        {
            track_velocity(0,0);
            std::cout << "State: Charging\n";
        }
        /**
         * The robot will be stopped and the battery level will increase
         * Once the battery is full, it will change into the "Cleaning" state.
         */
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

    /**
     * @class ReturnToDock
     * @brief This is the class for the state that makes the robot prioritize getting back to charge.
     *
     * This class inherits the State and AgentInterface class. The class uses
     * the State class form of having an entry(), during(), and exit(). The entry
     * code will run when the State is first entered and will loop through during 
     * for the rest of the time.
     * 
     * The class will make the robot target the charing dock in order to be charged up. 
     * 
     */
    class ReturnToDock : public State, public AgentInterface
    {
    public:
        /**
         * Sets the counter to 0 and will print out the state
         */
        void entry(const Event &e) 
        {
            std::cout << "State: Returning to Dock\n";
            counter = 0;
        }

        /**
         * The during code will make the robot target the location of the dock. If the robot's 3 sensors detect and obstacle
         * it will transition into the avoid state. If the robot reaches the dock station, it will transition into the charging state.
         * The robot will be draining its battery while in this state.
         */
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
        /** 
         * counter: This will be used to keep track of how many times the code loop has executed
         *        it will be utilized as a "timer"
        */
        int counter;
    };

    /**
     * @class Forward
     * @brief This is the class for the state that makes the robot move forward for a bit to avoid obstacles.
     *
     * This class inherits the State and AgentInterface class. The class uses
     * the State class form of having an entry(), during(), and exit(). The entry
     * code will run when the State is first entered and will loop through during 
     * for the rest of the time.
     * 
     * The class will make the robot move forward for a certain count in order to avoid obstacles before returning
     * to the returning to dock state. 
     * 
     */
    class Forward : public State, public AgentInterface
    {
    public:
        /**
         * Prints out the current state and sets counter to 0
         */
        void entry(const Event &e) 
        {
            std::cout << "State: Moving Forward\n";
            counter = 0;
        }

        /**
         * Sets the robot's velocity to purely forward velocity so that the robot can clear the obstacle
         * before returning to targeting the dock. Will detect if the robot is not moving enough and trigger
         * a backwards transition.
         */
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
        /** 
         * counter: This will be used to keep track of how many times the code loop has executed
         *        it will be utilized as a "timer"
        */
        int counter;
    };

    /**
     * @class Backwards
     * @brief This is the class for the state that makes the robot move backwards.
     *
     * This class inherits the State and AgentInterface class. The class uses
     * the State class form of having an entry(), during(), and exit(). The entry
     * code will run when the State is first entered and will loop through during 
     * for the rest of the time.
     * 
     * The class will make the robot move backwards when it is stuck in the forward state.
     * 
     */
    class Backward : public State, public AgentInterface
    {
    public:
        /**
         * Will print out the current state and set counter to 0.
         */
        void entry(const Event &e) 
        {
            std::cout << "State: Moving Backwards\n";
            counter = 0;
        }
        /**
         * Will set the velocity to a backwards velocity so that the robot can reverse from an obstacle. 
         * Will transition into either a returning to dock or cleaning state based on the battery life.
         */
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
        /** 
         * counter: This will be used to keep track of how many times the code loop has executed
         *        it will be utilized as a "timer"
        */
        int counter;
    };


    /**
     * @class VacuumController
     * @brief This is the class for the state that makes the robot move backwards.
     *
     * This class inherits the StateMachine and AgentInterface class. This is where the
     * transition states will be defined
     * 
     * The class will determine the behavior of the state machine and which state classes
     * will be utilized.
     * 
     */
    class VacuumController : public StateMachine, public AgentInterface
    {

    public:
        /**
         * These determine the state transitions and which strings will initiate the state change.
         * It also determines which state it's transitioning from and to. It will also initiate the 
         * state objects.
         */
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
        /**
         * Initates the state classes to be used.
         */
        Avoid avoiding;
        Rotating rotating;
        Clean cleaning;
        Charging charging;
        ReturnToDock returning;
        Forward forward;
        Backward backward;

    };

    /**
     * @class Vacuum
     * @brief This is the class will create the Vacuum Object.
     *
     * 
     * The class will determine the behavior of the agent in an enviro world.
     * 
     */
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