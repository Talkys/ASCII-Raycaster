# ASCII Raycaster

This is a simple raycaster that outputs to ascii in a posix compliant terminal.

It uses ncurses as the only non standard dependecy and can be compiled with just:

> g++ -o raycaster raycaster.cpp -lncurses

As ncurses input can't handle multiple inputs at the same time, you can't turn and walk, but as you don't have enemies this is not a problem. 

Maps should be edited in the source, but is easy to do. Just remember that "." is floor and "#" is wall.