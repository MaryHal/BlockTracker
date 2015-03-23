BlockTracker
============

Wrapper over [scanmem](https://github.com/coolwanglu/scanmem) to provide simple visuals for the [TGM](https://en.wikipedia.org/wiki/Tetris:_The_Grand_Master) series.

This is all rough around the edges so far, but currently, BlockTracker can keep track of buttons pressed per level, a level graph (per section of 100 levels), and a simple button display.

Usage:

    # blocktracker <pid> <level_address>

Example:

    # blocktracker $(pidof Texmaster2009) 083e2a50

Pid is the process id for the application we wish to scan.

Use scanmem to probe for the address in a similar fashion that you would use [Cheat Engine](http://www.cheatengine.org/).


Rough Roadmap
=============

1. Simpler method to automatically get the level address. Right now I have a VERY ugly python wrapper over scanmem attempts to probe for it by sending scripted keypresses.

2. Prettify the interface. While everything _works_ right now, it's __really__ ugly right now.

3. This code is the worst code I've ever written ever. I adopted the speedy gamedev mentality where you just try to get something that works while disregarding the elegance of the solution.
