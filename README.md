# Metronome_ARM_mbed
A digital metronome, IoT Solution
A metronome is a device used by musicians to have a context of the beats and also make sure they are maintaining a particular bpm.
Traditional metronomes includes like something similar to pendulums and there also exist digital ones. 

My Solution implements digital metronome which works on button press on the FRDM-K64F board and also on browser. 
The Light blinks on FRDM-K64F board as well the web browser to indicate beats. 

There are two modes implemented on the board. 
1. Play Mode
2. Learn Mode

1-> In play mode the green led blinks repeatedly, at a previously specified tempo. On startup default tempo is 0. So the LED will be off always.
2-> In learn mode, the red led pulses everytime user presses a button, and the time of the tap is recorded. I have coded the board so that it, stores the last 
4 recent taps and sets the BPM accordingly. Atleast 4 taps are required to create a valid BPM measurement. 
Coding on board is done on C++.

To make metronome a thing in IoT.
I have coded the board to expose the following REST end points (using Client M2M objects), under a frequency device(3318).
1. Set point value(5900) : The current metronome BPM is made readable through GET and writable through PUT.
2. Min/Max measured value(5601/5602): The highest and lowest BPM set by the user(through the BUTTON press on board or through REST API) is made readable.
3. Reset Min/Max measured value (5605): Reset Min/Max values through POST REQUEST.

Implemented the project by working on an existing code: https://github.com/wquist/metronome
So to work on the project, import this repo on mbed compiler and then make changes to the metronome.hpp and main.cpp files.
The above two files I am uploading in this repo. 

Then, the node.js server is used along with front end web-page. Here I use mbed connector-api-node, which is ARM's library for 
communicating with the Device Connector. 

The web Application does the following: 
1. Updates if the BPM on the board changes, either through button press or through REST API PUT request.
2. Set the BPM on the board
3. Views/Resets the Max/Min values on the board. 
In addition it also provides the following features:
4. Play clicking sound at speed specified by the BPM.
5. Shows blinking animation.

That's pretty much the apllication. Thank you. 
