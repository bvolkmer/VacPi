/**
 * @file VacPi.h
 * @brief VacPi main library with abstract logic
 * @author Benedikt Volkmer
 * @version 0.1
 * @date 2018-03-09
 */
#ifndef VACPI_H
#define VACPI_H

namespace VacPi {
    enum Direction { Forward, Backward };
    enum Turn { LEFT, RIGHT };
    enum Speed {Slow, Fast};
    struct Obstruction {
        bool dustLevel;
        bool left;
        bool right;
        bool center;
        bool front;
    };

    /**
     * @brief Abstract Class of movements to be implemented with platform-dependent engine instructions.
     */
    class Movements {
        public:
            /**
             * @brief Stop all engines
             */
            virtual void stopAll() = 0;
            /**
             * @brief Start all engines used as vacuum (i.e. vacuum itself and brushes)
             */
            virtual void startVacuum() = 0;
            /**
             * @brief Stop all engines used as vacuum (i.e. vacuum itself and brushes)
             */
            virtual void stopVacuum() = 0;
            /**
             * @brief Start engines to move a straight line
             *
             * @param direction Direction to move
             */
            virtual void moveStraight(Direction direction) = 0;
            /**
             * @brief Start engines and wait to rotate in place 45 degrees
             *
             * @param turn Direction to turn
             */
            virtual void rotate45(Turn turn) = 0;
            /**
             * @brief Move in a spiral fully covering a circle area
             *
             * @param turn Direction to turn
             * @param timer progression timer of spiral
             */
            virtual void spiral(Turn turn, int timer) = 0;
            /**
             * @brief Start Engines to move a curved line
             *
             * @param direction Direction to move
             * @param turn Direction to turn
             * @param speed Speed of turn
             */
            virtual void curve(Direction direction, Turn turn, Speed speed) = 0;
    };

    namespace States {
        class State {
            public:
                virtual ~State() {};
                virtual State* run(bool obstruction) = 0;
            protected:
                State(Movements* movements) {
                    this->movements = movements;
                };
                Movements* movements;
                int timer = 0;
        };
        class EdgeRun: public State {
            public:
                EdgeRun (Movements* movements);
                State* run(bool obstruction);
            private:
                const int timerThreshold = 1000;
                const int straightTimerThreshold = 5;
                int straightTimer = 0;
                const int obstructionTimerThreshold = 50;
                int obstructionTimer = 0;
        };
        class LinearRun: public State {
            public:
                LinearRun (Movements* movements);
                State* run(bool obstruction);
            private:
                const int timerThreshold = 100;
        };
        class Circling: public State {
            public:
                Circling (Movements* movements);
                State* run(bool obstruction);
            private:
                const int timerThreshold = 100;
        };
    } /* States */

    /**
     * @brief Main Class used to run in loop
     */
    class Looper{
        public:
            Looper (Movements* movements);
            ~Looper ();
            /**
             * @brief Function to be run in loop
             *
             * @param obstruction Obstructions detected at start of the loop
             */
            void loop(Obstruction obstruction);

        private:
            States::State* currentState;
            Movements* movements;
    };

} /* VacPi */
#endif /* end of include guard: VACPI_H */
