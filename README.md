# Introduction
Using pipes and threads to create a competition between two clients to determine who can react the fastest. When an opponent connects to
the server, it  acknowledges them by printing out their process ID(PID). When both opponents connect, the server send messages to the opponents 
to get them to react as fast as the can by just pressing enter. A score board is printed by the server after 3 rounds of fastest finger first
and a message is sent to both client saying how well they performed.

Both clients and the server must be ran on the same machine because the pipes and the the PID used to implement FFF

# Setup
1. use the make file to generate the executables be running ``make`` on the command line
2. Run the server executable first by running  ``./server``
3. Run the client executable next ``./client``
