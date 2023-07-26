# FC-simulator
The project implements a flight controller to simulate the drone's behaviour in 'Angle' mode, which includes axis stabilisation and manual throttle control.

The setpoint values are stored in global variables (using mutex) and updated when new values are received from the process simulating the hardware. The current attitude values are read by a separate thread, which sends a UDP query after being triggered by the cyclic thread (barrier mechanism). The computed control outputs from the shared memory are sent to the Model process.

The Model process implements an approximate mathematical model of the drone, taking into account the influence of axis attitudes on each other. New angle values are stored in global variables (using rwlock mechanism), allowing their retrieval by the thread transmitting them to the FC process (for visualization and logging purposes). Additionally, the Model process includes a thread simulating wind gusts. In this process, data is logged using queues, and values are sent for visualization.


![image](https://github.com/Tarsq/FC-simulator/assets/140641302/597e3845-1a40-421a-b611-15a275dddf00)
