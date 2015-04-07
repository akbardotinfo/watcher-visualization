### Introduction ###
Allows researchers to visualize test runs on their MANET testbed. Supports MANE and EMANE emulators. Shows geographic location and real-time annotations of nodes and connections. Captures live test runs for "TiVO" style playback (fast-forward, rewind, etc). Live and playback runs contain layers which can be toggled in real-time regardless of whether the researcher is viewing a playback or a live test run.

### How it works: ###
  * run a handful of daemons on your test nodes
  * daemons talk to a centralized watcherd daemon
  * centralized daemon writes events to a local database
  * GUI(s) connect to the centralized daemon and get fed events from the database based on the playback rate and direction the GUIs specify
  * GUIs specify new playback rates and directions as needed

<img src='http://i.imgur.com/jvsKA.png' width='65%' title='Watcher Arch' />

### Features: ###
  * view locations of nodes in real time or post test run
  * capture all node messages and play them back "TiVO" style, rewinding, fast forwarding, etc.
  * replay captured tests moving around in time via fast-forward, rewind, etc.
  * multiple GUI(s) and views into any test run
  * GUIs can share synchronized streams, if one GUI changes the playback, all others sycn'd to that stream, see the change.
  * all test node message captured to sqlite "flat-file" database
  * somewhat documented! ('make doc' in ./src builds Doxygen, man pages, and User's Guide)
  * information is layered for ease of visualization
  * Existing GUIs:
    * OpenGL based GUI
      * layers are dynamic and easily generated in real time
      * visual characteristics of any layer is intialized at run time via flat configuration file or updated via the GUI itself in real time
      * changes to GUI saved as they happen
    * Experimental OGRE GUI
    * Google Earth plugin GUI
    * Non-geographic GUI for displaying realtime numeric node data from any and all nodes on the testbed.
  * GNU Autotools support, yum package support
  * Written in C++ using Boost (may not be a feature)

### Note ###
Prepared through collaborative participation in the Communications and Networks Consortium sponsored by the U. S. Army Research Laboratory under the Collaborative technology Alliance Program, Cooperative Agreement DAAD19-01-2-0011. The U.S. Government is authorized to reproduce and distribute reprints for Government purposes notwithstanding any copyright notation thereon. The views and conclusions contained in this document are those of the authors and should not be interpreted as representing the official policies, either expressed or implied, of the Army Research Laboratory or the U. S. Government.

### Licence ###
Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
This file is part of WATCHER.
WATCHER is free software: you can redistribute it and/or modify it under the terms of the
GNU Affero General Public License as published by the Free Software Foundation, either version
3 of the License, or (at your option) any later version.
WATCHER is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with Watcher.
If not, see http://www.gnu.org/licenses.

### Screenshots ###
  * EarthWatcher, the Google Earth based GUI
<img src='http://i.imgur.com/tBNO1.jpg' width='75%' title='Earth Watcher Image' />

<img src='http://i.imgur.com/eWg8P.jpg' width='75%' title='Earth Watcher Image' />

  * "Legacy" Watcher, OpenGL-based GUI

<img src='http://i.imgur.com/o05aZ.jpg' width='75%' title='Legacy Watcher' />

<img src='http://i.imgur.com/JBlcz.jpg' width='75%' title='Legacy Watcher' />

<img src='http://i.imgur.com/0o5Zo.png' width='75%' title='Legacy Watcher' />

  * Data Watcher

<img src='http://i.imgur.com/l6coT.png' width='75%' title='Data Watcher' />

**Sync'd Streams**

<img src='http://i.imgur.com/OVtgU.png' width='75%' title='Data Watcher' />

**Look and feel of visualization can be modified while runnning**

<img src='http://i.imgur.com/iwa4K.png' width='50%' title='Visual Configuration' />
<img src='http://i.imgur.com/aLT7n.png' width='75%' title='Visual Configuration' />
