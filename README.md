# SinkHole-Attack-using-NS3-Simulation

This project demonstrates a Sinkhole Attack simulation using the ns-3 network simulator. The simulation highlights the impact of sinkhole attacks in a wireless ad hoc network.
![image](https://github.com/user-attachments/assets/1e04ed70-f36f-4afd-b514-d034634c5aa8)

Prerequisites
-------------
1. ns-3 installed on your system.
2. Basic knowledge of C++ and ns-3.
3. NetAnim for visualization.

**Steps to Run the Simulation**
-------------------------------
1. Save the C++ CodeSave your C++ code as sinkhole_final.cc in the scratch directory of your ns-3 installation.
2. Navigate to the ns-3 DirectoryOpen a terminal and navigate to your ns-3 directory. For example:
3. cd /path-to-ns3/ns-3.36.1
4. Run the SimulationExecute the following command to run the simulation:
    ./ns3 run scratch/sinkhole_final.cc
5. Visualize the Simulation in NetAnimOpen the generated NetAnim XML file for visualization:
    ./NetAnim
6. Load the file Sinkhole_attack_example.xml in the NetAnim GUI to view the packet transmissions.

**Outputs**
----------

Energy Log File: Energy consumption of nodes is logged in energy_log.csv.
NetAnim XML File: Packet transmission is visualized in Sinkhole_attack_example.xml.
Simulation Statistics: Total packets sent and received are displayed in the terminal.
![Screenshot from 2025-01-05 12-09-32](https://github.com/user-attachments/assets/6edea112-24ab-499d-887b-dd930167da7c)
![Screenshot from 2025-01-05 12-10-05](https://github.com/user-attachments/assets/cb55f3cc-3475-490f-bff7-dac5baa7e6e7)



**Explanation**
--------------
The simulation consists of 22 nodes: 20 regular nodes, 1 start node, and 1 sink node.
Two nodes act as sinkhole attackers, disrupting the network by drawing traffic.
The remaining nodes communicate using AODV routing.
The simulation tracks energy usage and visualizes packet flows in NetAnim.
