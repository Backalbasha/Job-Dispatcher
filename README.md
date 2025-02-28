# Job-Dispatcher

This program simulates a server cluster with a main server that acts as a job dispatcher, distributing client requests to worker servers. The main server reads commands from a command file and sends them to worker computers using MPI. To boost efficiency, OpenMP is used to parallelize tasks. The command and command2 text files contain examples of the types of commands the system accepts, and the m1 and m2 files include matrices for the addition and multiplication commands. For larger matrices, the workload is distributed across multiple machines to ensure better performance.
