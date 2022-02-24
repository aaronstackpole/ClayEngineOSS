# ClayEngineOSS

Simple rendering system application framework, based on DirectX 11 Toolkit https://github.com/microsoft/DirectXTK

Running the application should result in a standard 1080 HD window (assuming your screen is large enough) and a standard text console.

Trying to full-screen or minimize the application will crash it, the feature is currently blocked on the swap chain but not the msg pump.

--Aaron

February 2022 Update

Major refactoring complete, engine now split into three projects, client, server, and library. The client and server applications 
currently need just one class to access the library, the client has a demo game "SquareChase" showcasing how to set up the client. You 
can create an empty project and apply the ClayEngineCore.props file to it and you'll be ready to go.

The window class has been replaced with a new static WindowSystem class, it has hookups for callbacks for OS messages, but they are not
yet hooked up, so you still can't change the size of the window, but it doesn't crash when you try now ;)

Did a lot of cleanup, a huge amount of prototoype and proof of concept code was removed. I removed the copy of the DirectX toolkit we
were using. To get this to work, you do need to get DirectX Toolkit for DirectX 11. This version has had the VoxelFarm code removed as 
well as that's not something we can share.

This update represents a major step towards Milestone 0.0.2, it has an updated input buffer that fixes all of the RC scrollback bugs, 
and has the network stack implemented. Note that the ClientCoreSystem currently has the network module connect funtion on the client 
side commented out so it doesn't actually try to connect. The network code for the client and server purely sets up a connect server 
and a client to connect to it. Once that's done, nothing else happens, there are no send/recv calls implemented. If you want to see it 
you can run the ClayEngineServer.exe and compile the client with the connect line uncommented and you can see the connection happen.

EDIT: Actually, you'd also need to change the IP address of the server to connect to. This currently points at my router... :/

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
