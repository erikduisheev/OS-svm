#include "kernel.h"

#include <iostream>
#include <algorithm>

// svm/svm /scheduler:priority ./assemblies/change_registers_and_exit.vmexe ./assemblies/change_registers_and_exit.vmexe ./assemblies/change_registers_and_exit.vmexe


namespace svm
{
    Kernel::Kernel(
                Scheduler scheduler,
                std::vector<Memory::ram_type> executables_paths
            )
        : board(),
          processes(),
          priorities(),
          scheduler(scheduler),
          _last_issued_process_id(0),
          _last_ram_position(0),
          _cycles_passed_after_preemption(0),
          _current_process_index(0)
    {
        std::for_each(
            executables_paths.begin(),
            executables_paths.end(),
            [&](Memory::ram_type &executable) {
                CreateProcess(executable);
            }
        );

        if (scheduler == FirstComeFirstServed) {
            
            std::cout << "First come first serve" << std::endl;
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the FCFS
                // Timer inetrrupt is not needed
                std::cout << "Kernel: Allowing the current process " << processes[_current_process_index].id << " to run" << std::endl;
                
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the FCFS
                // Unload the current process
                
                std::cout << "\nKernel: Processing first software interrupt." << std::endl;
                std::cout << "Kernel: Unloading process " << processes[_current_process_index].id << std::endl;
                
                processes.erase(processes.begin() + _current_process_index);
            
                if (processes.empty()) {
                    std::cout << "Kernel: There is no more processes. Machine is stopping" << std::endl;
                    board.Stop();                   
                }
                else {
                    std::cout << "Kernel: Switching the context to process " << processes[_current_process_index].id << std::endl << std::endl;
                    board.cpu.registers = processes[_current_process_index].registers;
                    processes[_current_process_index].state = Process::States::Running;
                }
                
            };
        } else if (scheduler == ShortestJob) {
            
           // for(unsigned int i = 0; i < processes.size(); i++) {
           //     std::cout << "id " << processes[i].id  << ", "
           //               //<< "registers " << processes[i].registers << ", "
           //               << "state " << processes[i].state << ", "
           //               << "priority " << processes[i].priority << ", "
           //               << "seq count " << processes[i].sequential_instruction_count << ", "
           //               << std::endl;
           //  }

            std::cout << "ShortestJob" << std::endl;
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the Shortest
                //  Job scheduler
                // Timer inetrrupt is not needed
                std::cout << "Kernel: Allowing the current process " << processes[_current_process_index].id << " to run" << std::endl;
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the Shortest
                //  Job scheduler

                std::cout << "\nKernel: Processing first software interrupt." << std::endl;
                std::cout << "Kernel: Unloading process " << processes[_current_process_index].id << std::endl;
                
                processes.erase(processes.begin() + _current_process_index);
                
                if (processes.empty()) {
                    std::cout << "Kernel: There is no more processes. Machine is stopping" << std::endl;
                    board.Stop();                   
                }
                else {
                    std::cout << "Kernel: Switching the context to process " << processes[_current_process_index].id << std::endl << std::endl;
                    board.cpu.registers = processes[_current_process_index].registers;
                    processes[_current_process_index].state = Process::States::Running;
                }
        
            };
        } else if (scheduler == RoundRobin) {
            
            // for(unsigned int i = 0; i < processes.size(); i++) {
            //     std::cout << "id " << processes[i].id  << ", "
            //               //<< "registers " << processes[i].registers << ", "
            //               << "state " << processes[i].state << ", "
            //               << "priority " << processes[i].priority << ", "
            //               << "seq count " << processes[i].sequential_instruction_count << ", "
            //               << std::endl;
            // }


            std::cout << "Round Robin" << std::endl;
            
            board.pic.isr_0 = [&]() 
            {
            // ToDo: Process the timer interrupt for the Round Robin
            //  scheduler
        
                std::cout << "Kernel: Processing the timer interrupt." << std::endl;         
                if (processes.size() < 2)
                {
                    std::cout << "Kernel: Only one process remained.\nKernel: Scheduling is not required." << std::endl << std::endl;
                    return;
                }
            
                ++_cycles_passed_after_preemption;
                
                if (_cycles_passed_after_preemption > _MAX_CYCLES_BEFORE_PREEMPTION) 
                {
                    // Switch the context
                    process_list_type::size_type next_process_index = (_current_process_index + 1) % processes.size();  
                    std::cout << "Kernel: Switching from the process " << processes[_current_process_index].id
                        << " to the next process " << processes[next_process_index].id
                        << std::endl;


                    std::cout << "Kernel: Saving information from CPU to PCB of process " << processes[_current_process_index].id << std::endl;
                    std::cout << std::endl;
                    processes[_current_process_index].registers = board.cpu.registers;
                    processes[_current_process_index].state = Process::States::Ready;
                    
                    std::cout << "Kernel: Now current process is process " << processes[next_process_index].id << std::endl;
                    _current_process_index = (_current_process_index + 1) % processes.size();
                    
                    std::cout << "Kernel: Restoring information of previous process from memory to CPU." << std::endl;
                    board.cpu.registers = processes[_current_process_index].registers;
                    processes[_current_process_index].state = Process::States::Running;
                    
                    std::cout << "Kernel: Reseting slice timer" << std::endl;
                    _cycles_passed_after_preemption = 0;
                }
                else {
                    std::cout << "Kernel: Allowing the current process " << processes[_current_process_index].id << " to run" << std::endl;
                }
                std::cout << "Kernel: The current cycle count " << _cycles_passed_after_preemption << std::endl << std::endl;
                
        
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the
                //  Round Robin scheduler

                //Unload the current process
                std::cout << "Kernel: Processing first software interrupt." << std::endl;
                std::cout << "Kernel: Unloading process " << processes[_current_process_index].id << std::endl;
                
                processes.erase(processes.begin() + _current_process_index);
                
                if(processes.empty())
                {
                    std::cout << "Kernel: There is no more processes. Machine is stopping" << std::endl;
                    board.Stop();
                }
                else
                {
                    if (_current_process_index >= processes.size())
                    {
                        // std::cout << "Kernel: current process became first process." << std::endl;
                        _current_process_index = 0;
                    }

                    std::cout << "Kernel: Switching the context to process " << processes[_current_process_index].id << std::endl << std::endl;

                    board.cpu.registers = processes[_current_process_index].registers;
                    processes[_current_process_index].state = Process::States::Running;
                    _cycles_passed_after_preemption = 0;
                }
                
            };
        } else if (scheduler == Priority) {
            
            // std::cout << processes.size() << std::endl;
            // for(unsigned int i = 0; i < processes.size(); i++) {
            //     std::cout << "id " << processes[i].id  << ", "
            //               //<< "registers " << processes[i].registers << ", "
            //               << "state " << processes[i].state << ", "
            //               << "priority " << processes[i].priority << ", "
            //               << "seq count " << processes[i].sequential_instruction_count << ", "
            //               << std::endl;
            // }

            std::cout << "Priority" << std::endl;
            board.pic.isr_0 = [&]() {
                // ToDo: Process the timer interrupt for the Priority Queue
                //  scheduler
                
                std::cout << "Kernel: Processing the timer interrupt." << std::endl;         
                
                for (unsigned int i = 1; i < priorities.size(); ++i){
                    ++priorities[i].priority;
                }
                
                //Switch the context
                process_list_type::size_type next_process_index = (_current_process_index + 1)% priorities.size();

                if (priorities[_current_process_index].priority < priorities[next_process_index].priority) {
                    
                    std::cout << "Kernel: Priority of current process " << priorities[_current_process_index].priority << std::endl;
                    std::cout << "Kernel: Priority of next process " << priorities[next_process_index].priority << std::endl;
                    
                    std::cout << "Kernel: Saving information of process priority from CPU to PCB." << std::endl;
                    std::cout << std::endl;
                    priorities[_current_process_index].registers = board.cpu.registers;
                    priorities[_current_process_index].state = Process::States::Ready;
                    
                    
                    for(unsigned int i = 1; i < priorities.size(); ++i){
                        int j = i - 1;
                        Process _temp = priorities[i];
                        while(j >= 0 && priorities[j].priority < _temp.priority){
                            priorities[ j + 1 ] = priorities[j];
                            j--;
                        }   
                        priorities[ j + 1 ] = _temp;
                    }   
                    
                    std::cout << "Kernel: Restoring information from memory to CPU." << std::endl;
                    board.cpu.registers = priorities[_current_process_index].registers;
                    priorities[_current_process_index].state = Process::States::Running;
                    
                    std::cout << "Kernel: Reseting slice timer" << std::endl;
                    _cycles_passed_after_preemption = 0;
                }
            
            };

            board.pic.isr_3 = [&]() {
                // ToDo: Process the first software interrupt for the Priority
                //  Queue scheduler

                std::cout << "Kernel: Processing first software interrupt." << std::endl;
                
                priorities.pop_front();

                if (priorities.empty()) {
                    std::cout << "Kernel: There is no more processes. Machine is stopping" << std::endl;
                    board.Stop();                   
                }
                else {
                    if (_current_process_index >= priorities.size()) {
                        _current_process_index = 0;
                    }
                    board.cpu.registers = priorities[_current_process_index].registers;
                    priorities[_current_process_index].state = Process::States::Running;
                }
            };
        }

        // ToDo

        // ---

        board.Start();
    }

    Kernel::~Kernel() { }

    void Kernel::CreateProcess(Memory::ram_type &executable)
    {
        std::copy(
            executable.begin(),
            executable.end(),
            board.memory.ram.begin() + _last_ram_position
        );

        Process process(
            _last_issued_process_id++,
            _last_ram_position,
            _last_ram_position + executable.size()
        );

        /* if (processes.size() = 0)
        {
            // set state to running
        }
 */
        _last_ram_position += executable.size();

        // ToDo: add the new process to an appropriate data structure
        

        
        if (scheduler == ShortestJob) {         
            
            for(int i = 1; i < processes.size(); ++i){
                
                int j = i - 1;
                Process temp = processes[i];
                
                while(j >=0 && processes[j].sequential_instruction_count > temp.sequential_instruction_count){
                    processes[j+1] = processes[j];
                    j--;
                }   
                processes[j+1] = temp;
            }       
        }   
        
        if (scheduler != Priority){
            processes.push_back(process);
        } else {
            priorities.push_back(process);
            std::sort (priorities.begin(), priorities.end());
        }
            
            
            
            // ToDo: process the data structure

            // ---
    }
}