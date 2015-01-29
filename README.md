BlockTracker
============

Wrapper over [scanmem](https://github.com/coolwanglu/scanmem) to provide simple visuals for the [Tetris: The Grandmaster](https://en.wikipedia.org/wiki/Tetris:_The_Grand_Master) series.

Usage:

    # blocktracker <pid> <level_address>

Example:

    # blocktracker $(pidof Texmaster2009) 083e2a50

Pid is the process id for the application we wish to scan.

Use scanmem to probe for the address in a similar fashion that you would use [Cheat Engine](http://www.cheatengine.org/).

This is all rough around the edges so far, but currently, BlockTracker can keep track of buttons pressed per level, a level graph (per section of 100 levels), and a simple button display.

Rough roadmap:

1. Simpler method to get the level address. Right now I have a VERY ugly python wrapper over scanmem attempts to automatically probe for it.

2. Cleanup the code.

3. Prettify everything. While it _works_, it's __really__ ugly right now.
