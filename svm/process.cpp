#include "process.h"

namespace svm
{
    Process::Process(
            process_id_type id,
            Memory::ram_size_type memory_start_position,
            Memory::ram_size_type memory_end_position)
        : id(id),
          registers(),
          state(Ready),
          priority(0),
        memory_start_position(memory_start_position),
        memory_end_position(memory_end_position)
    {
        registers.ip =
            memory_start_position;

        sequential_instruction_count =
            (memory_end_position - memory_start_position) / 2;
    }

    bool Process::operator<(const Process &another_process) const {
        return priority < another_process.priority;
    }
}
