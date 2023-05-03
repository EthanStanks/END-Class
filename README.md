# END-Class

This repository contains the implementation of an interactive 3D graphics application. The implementation uses the provided renderer as a base and allows users to simulate particles on a grid with different colors and behaviors. The implementation also includes the implementation of Look-At, Turn-To, and Mouse-Look algorithms from the lecture.

<h3>Installation</h3>
To use the implementation, you will need to have a C++ compiler and Visual Studio installed on your computer.

Clone the repository using git clone https://github.com/EthanStanks/END-Class.git
Open the project in Visual Studio.
Build and run the project.

<h3>Usage</h3>
The implementation simulates particles on a grid and allows users to use Look-At, Turn-To, and Mouse-Look algorithms. Users can control the movement of the particles and the camera using the following keys:

Arrow keys: Move the camera
W: Move the camera up
S: Move the camera down
A: Move the camera to the left
D: Move the camera to the right
Q: Move the camera forward
E: Move the camera backward
F: Toggle the debug renderer

<h3>The simulation also supports the following behaviors:</h3>
Update functionality: Updates the velocity of the particles, spawns particles, moves the camera, collects input, color update changes, etc.
Debug Renderer: Renders a grid (simulating a floor), changes the colors of the grid based on time, and allows users to cycle through the colors of the grid.
Pools: Allows users to spawn simple particles, create a sorted pool and a free pool, and use them to spawn particles. Users can also use multiple emitters that share the pool to spawn particles.
Simple Particle behavior: Allows users to spawn particles a little at a time, to have a constant spray, not fire them all at once.
  
<h3>Credits</h3>
This implementation was created by [your-name]. It is based on the provided renderer and the Look-At, Turn-To, and Mouse-Look algorithms from the lecture.
