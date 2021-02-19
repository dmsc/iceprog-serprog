iceprog-serprog
===

A Programmer for iCE40 FPGA boards using flashrom serprog protocol.

![spi-flasher](https://raw.githubusercontent.com/florolf/spi-flasher/master/img/front.jpg)

**Requirements**

* mingw-w64, git and make (via apt Command Line Tools)

**Compiling for Windows**

After installing the necessary tools, building `iceprog` is as simple as running the included script:

```
./build.sh
```

After successful compilation, the target executable file will be generated in the `build` folder.

**Compiling for Linux**

After installing the necessary tools, building `iceprog` is as simple as running the command:

```
make
```

After successful compilation, the target executable file will be generated in the current folder.

**Usage**

Using `iceprog --help` is straightforward.

**Author**

Originally written by [dmsc](https://github.com/dmsc/iceprog-serprog) and released under the terms of the GNU GPL, version 2, or later. 

**License**

This is free software: you can redistribute it and/or modify it under the terms of
the latest GNU General Public License as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
