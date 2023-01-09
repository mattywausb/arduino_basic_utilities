# encoder_how_to
Small arduino project to elaborate the "ultimate" encoder reading and switch handling


# Motivation
I was still struggling with the best kind of reading encoder input.
* dont miss a step
* dont duplicate steps
* some comfort for using the input in the final code

Also my code for handling button/switch input was a mess. So in this small project I try out some patterns and save it for future projects.

It is not a library but more a source of approved code to copy into upcoming projects.

# Content
The sketch "my_input_code_pattern" contains my new best practices how to handle button input and encoder input

## Basic code structure

I generally use an abstraction layer to measure all input. This hides all circute specific adjustemens and fiddeling, so the main functional code 
can just rely on robust functions. Therefore the code separates in at least two elements:
* the main code
* the conding of the input functions (input.ino)

### the main code
Contains the main setup and the main loop.
Both functions need to call the setup / loop of the input module for initialization and regular scan of changes.

The current states of the input devices are then be requested by the main code via querey functions to implement the functionanlity.

### the input code
The input code does all the device handling and provides query functions, that are named in a way to support best readabilty of the main code.
It must be adapted to the current circuit depending wich devices are connected (Buttons, encoders etc.) and how (digital/analog input, using pullup/pulldown logic).

### the switch class
The "Switch" class implements a general function set to handle interval scanned switches in an efficient way. It is used by the input code to manage multiple switches indiviually including debouncing and the tracking of state changes and state duration. It has a very low data memory footprint of 4 bytes for every switch.




