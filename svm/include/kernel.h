#ifndef KERNEL_H
#define KERNEL_H

#include <deque>
#include <queue>
#include <string>

#include "board.h"
#include "process.h"

namespace svm
{
    // OS Kernel
    class Kernel
    {
        public:
            enum Scheduler
            {
                Undefined,
                FirstComeFirstServed,
                ShortestJob,
                RoundRobin,
                Priority
            };

            typedef std::deque<Process> process_list_type;
            //changed from priority queue to deque, because priority queue does not have pop_front method
            typedef std::deque<Process> process_priorities_type;

            Board board;

            process_list_type processes;
            process_priorities_type priorities;

            Scheduler scheduler;

            // Kernel boot process (setup ISRs, create processes, etc.)
            Kernel(
                Scheduler scheduler,
                std::vector<Memory::ram_type> executables_paths
            );

            virtual ~Kernel();

            // Creates a new PCB, places the executable image into memory
            void CreateProcess(Memory::ram_type &executable);

        private:
            static const unsigned int _MAX_CYCLES_BEFORE_PREEMPTION = 3;

            Process::process_id_type _last_issued_process_id;
            Memory::ram_type::size_type _last_ram_position;

            unsigned int _cycles_passed_after_preemption;
            process_list_type::size_type _current_process_index;
    };
}

#endif
