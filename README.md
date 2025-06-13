# Sidreal Engine
This engine is basically an excuse for me to teach myself graphics programming, practice C++, learn about optimization, and more!

## Update Log
### June 12th, 2025
This is the first log since last year when I first started working on this project again. Since then, I've remade the Visual Studio solution and created two separate projects: one for the engine code and one for the application code. Right now, my work is still mostly on the engine, but having an application layer (and maybe an editor layer as well) will help me think more about how I actually want to use my systems—and possibly allow me to reuse the engine for other things in the future!

I've also added cascades to my shadow mapping and significantly improved the overall quality by snapping the light-space matrix to its texel size and making it square to prevent stretching.

Lastly, I rebuilt the asset loading and rendering system to use an entity-component system. My implementation is very basic, but it's already helping me think in a more data-oriented way when it comes to assets and is giving me new ideas on how to move forward!

![image](https://github.com/user-attachments/assets/68bb1c5f-6d01-4412-8ecf-e850cf2d1e19)
