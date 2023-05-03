# END-Class

This repository contains the implementation of an interactive 3D graphics application. The implementation uses the provided renderer as a base and allows users to simulate particles on a grid with different colors and behaviors. The implementation also includes the implementation of Look-At, Turn-To, and Mouse-Look algorithms from the lecture.

<h3>Installation</h3>
To use the implementation, you will need to have a C++ compiler and Visual Studio installed on your computer.
<ul>
<li>Clone the repository using git clone https://github.com/EthanStanks/END-Class.git</li>
<li>Open the project in Visual Studio.</li>
<li>Build and run the project.</li>
</ul>

<h3>Camera Controls</h3>
<ul>
<li>To move the camera, hold down the right mouse button to unlock the mouse cursor. Then, move the mouse to rotate the camera.</li>
<li>To move the camera forward, press the 'W' key. To move the camera backward, press the 'S' key.</li>
<li>To move the camera to the right, press the 'D' key. To move the camera to the left, press the 'A' key.</li>
<li>To move the camera up, press the 'Space' key. To move the camera down, press the 'Shift' key.</li>
</ul>
<h3>Player Controls</h3>
<ul>
<li>To move the player forward, press the 'Up' arrow key. To move the player backward, press the 'Down' arrow key.</li>
<li>To turn the player to the right, press the 'Right' arrow key. To turn the player to the left, press the 'Left' arrow key.</li>
</ul>
Note: The speed of camera and player movement is affected by the dt variable.

<h3>The simulation also supports the following behaviors:</h3>
<ul>
<li>Update functionality: Updates the velocity of the particles, spawns particles, moves the camera, collects input, color update changes, etc.</li>
<li>Debug Renderer: Renders a grid (simulating a floor), changes the colors of the grid based on time, and allows users to cycle through the colors of the grid.</li>
<li>Pools: Allows users to spawn simple particles, create a sorted pool and a free pool, and use them to spawn particles. Users can also use multiple emitters that share the pool to spawn particles.</li>
<li>Simple Particle behavior: Allows users to spawn particles a little at a time, to have a constant spray, not fire them all at once.</li>
</ul>
  
<h3>Credits</h3>
This implementation was created by Ethan Stanks. It is based on the provided renderer and the Look-At, Turn-To, and Mouse-Look algorithms from the lecture.
