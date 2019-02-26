# Jose-Graphics-Learnings-of-America-for-Make-Benefit-Glorious-Nation-of-Bolivia

CONTROLS

-Move with WASD  
-Mouse to move ecamera  

Settings:  
-Press +/- (on numpad) or up/down arrows to increase or decrease a setting  
-Select settings using by pressing:  
	-1 or Numpad1 = Exposure  
	-2 or Numpad2 = Black and white filter  
	-3 or Numpad3 = Bloom intensity  
	-4 or Numpad4 = Fog intensity  
	-5 or Numpad5 = Depth of field start  
	-6 or Numpad6 = FOV  
	-7 or Numpad7 = Adjust near clip   
	-8 or Numpad8 = Adjust far clip  
	-9 or Numpad9 = Rotate directional light   
	
-Toggle  
	-Q = Fog  
	-Z = Depth of Field  
	-F = Spotlight/flashlight  
	-X = Screen warp effect  
	-C = Align Instanced objects to camera  
	-V = Randomize Instanced Objects  
	-B = Toggle emissive and subsurface scattering effects on Flesh Tree  
	-N = Toggle Look at Flesh Tree  
	-M = Activate Minimap  
	
	
-Hold  
	-O toggle procedural debug sphere on Flesh Tree  
	-L To Render current scene to flag (Front is default scene, Back is inverted)  
	-G = Create point light on current camera's location. Release to detach from camera. Up to 5 point lights.  
	
Notes:  
	-Near Clip : Unless you are very close to the ground, you will have to press it for a long time to notice effects  
	-Panning sine time based pixel shader is on the glowing emissive of the flesh tree. The glow changes through time and flows through the flesh tree.  
	-Terrain uses baked shadows for performance, so only dynamic object shadows will update when you move the directional light  
	-Minimap is not compatible with Post Processing effects   
	-I couldn't fix dynamic tesselation jittering at certain distances, but Larri said it doesn't really matter for the points  
	-Layered transparency is showcased in the cloud billboards. At times they are layered at more than 10 on top of each other without breaking.  
	-Reflective cubemap is on the Flag's pole. Look closer to see reflection better.  
	-Terrain alignment uses compute shader. It reads from the heightmap and the current instance's location   
	