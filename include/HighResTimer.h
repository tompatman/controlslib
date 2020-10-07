//
// Created by service on 10/7/16.
//

#pragma once

#include <Poco/Types.h>

namespace PbPoco {
    namespace Core {
        class HighResTimer {

        public:
            HighResTimer();

            ~HighResTimer();

            /**
             * Start measuring time
             */
            void start();

            /**
             * Stop measuring time
             */
            void stop();

            /**
             * Reset the timer
             */
            void reset();

            /**
             * Read the elapsed time in nano seconds.
             * @return
             */
            __syscall_slong_t getElapsedMs();

            bool isRunning()
            {
              return !_bStop;
            }
            /**
             * Return the whole timespec struct.
             * @return
             */
            struct timespec getElapsedTime();

        private:
            struct timespec _requestStart;
            struct timespec _requestEnd;
            struct timespec _elapsedTime;
            bool _bStop;
            __syscall_slong_t _elapsedMs;
        };
    }
}

