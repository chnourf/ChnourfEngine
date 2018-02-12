# ChnourfEngine
openGL &amp; C++ engine


Note : This is still a work in progress, commits come and go depending on my availability and motivation. Go to <b>Sample Images</b> if you want to see what the engine is currently capable of !

## How does it work ?

The creation of process of the terrain works by steps :
<ol>
  <li>Generate a grid of hexagonal cells that represent the continent (position of the points is warped)</li>
  <li>Generate the "low res" heightfield of the continent with octave Perlin Noise with mask to give a "warped island" shape</li>
  <li>Place mountains near the coast and at the center of the landmass</li>
  <li>Compute and place rivers</li>
  <li>Compute rainfall for the grid by using a propagation algorithm with the mountains acting as a barrier and rivers artificially increasing rainfall in the area (to simulate moisture)</li>
  <li>Generate the "high res" heightfield by adding other octaves on top on the previous noise, discretizing by 128x128m tiles</li>
  <li>Compute erosion with a droplet algorithm for each tile (current bottleneck) and lerping the edges to ensure continuity</li>
  <li>Deduce Biome by sampling rainfall and computing temperature</li>
  <li>Generate LODs for the tiles</li>
  <li>Add grass planes, density and color influenced by biome</li>
  <li>Render !</li>
</ol>

## TODO :
 <ul>
  <li>Speed up erosion algorithm by computing it on the GPU</li>
  <li>Fix the Shadow Map issue when moving the camera (shadow map is thus deactivated for now)</li>
  <li>Add trees and rocks</li>
  <li>Add rivers according to those places on the grid</li>
  <li>Better clouds with rim lighting</li>
  <li>Move to a deferred rendering</li>
  <li>Proper thread system instead of C++ STL ones</li>
  <li>Cascading Shadow Maps</li>
  <li>Adding a quad tree to speed up culling</li>
</ul> 
