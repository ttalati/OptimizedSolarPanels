# OptimizedSolarPanels (Sunflower Solar Panels)
Code to arduino project, which was an innovative optimazation to solar panels

Project was inspired by the sunflower and its ability to track the sun and optimize the amount of light it hits. 

Often solar panels lack this capability and this project was made to address the issue.

The hardware consisted of a "control" solar panel unit, which contains solar panels on each end. Every few moments (time is controlled by the user's preference in the hardware), data from the solar panel. Each solar panel sends the amount of energy produced, which is later converted to voltage. Using 2 2-dimensional planes, to make a 3D one, the optimal angle that solar panels should face is calculated. Then this angle is what all the stepper motors, which the rest of the larger solar panels are connected to, change to. Hence the angle and the amount of energy produced can be optimized under any condition. 

Possible Optimizations:
- The main optimization is change in the way the angle is calculated. Ideally tangent inverse would be used, but because of the limitations of the software a simple linear line is used to find the angle, which does create slight under and over approximations at certain points. 
- In the future a raspberry pi could be used to have this optimized. 
