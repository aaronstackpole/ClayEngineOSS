# ClayEngineOSS

Simple rendering system application framework, based on DirectX 11 Toolkit https://github.com/microsoft/DirectXTK

Running the application should result in a standard 1080 HD window (assuming your screen is large enough) and a standard text console.

Trying to full-screen or minimize the application will crash it, the feature is currently blocked on the swap chain but not the msg pump.

--Aaron

January 2022 Update

There has been a lot of work on this since I first published this project. Overall, this isn't even the same actual solution, I started
again with a blank project and built everything here from scratch. There are still some artifacts from the import of some content, and
it is possible that not all of the project configurations will work, there might be some paths that you'd need to update in the project
or solution properties.

This version of the engine has most of the application framework built. We've got the beginnings of the UI system being worked on 
(terribly fragmented POC code in Sensorium.h), the 2D rendering system is fully functional, there's a custom window message processing
system that's capturing typed input into buffers, and a good set of system services. We've implemented a simple JSON parser to load 
2D Sprite and SpriteFont content, and a basic application state system is in place. Finally, there's a custom threaded game ticker
system built that allows you to hook update and draw functions to just about anything with a standard function callback.

This has been a huge amount of work on my part, and I hope you find some value in this code. As we continue to work on the engine, you
can expect to see updates fairly infrequently, but there will be updates :)

EDIT: We should note that this version does not include any of the networking implementation from before, we are currently assessing ASIO

--Aaron
