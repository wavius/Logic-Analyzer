# Computer Systems

We have designed Computer Systems specifically for the DE-series boards. Each board has one or more systems depending on the included processor(s) within the system. In addition to one or more processors, the systems include memory ports, basic input/output ports (for switches, lights, etc.), and multimedia ports (for video, audio, and the like). Accompanying the systems is a set of sample programs. To start using the Computer Systems, please download them from the [Design Examples releases](https://github.com/fpgacademy/Design_Examples/releases) page and read the associated user manual for the target board and processor.

## Repository Contents

This repository contains the source code for the Computer Systems, the associated documentation, and the sample programs with the following directory structure:
    
    Computer_Systems/
    +-- <Board Name>/
    │   +-- doc/
    │   │       - Documentation for the Computer Systems of the given board 
    │   +-- scripts/
    │   │       - Makefile and TCL scripts for building and compiling the various Computer Systems for the given board
    │   +-- software/
    │   │       - Software header files with the address map of all the system components. These are used by the sample programs.
    │   +-- src/
    │           - Source files required by the scripts, such as the Quartus software project files, Verilog code, etc.
    +-- common/
    │   +-- doc/
    │   │       - System component description used by multiple systems/boards 
    │   +-- figs/
    │   │       - Contains figures and diagrams used in the documents of multiple systems/boards 
    │   +-- scripts/
    │           - Contains generic Makefile and TCL scripts used by multiple systems/boards 
    +-- sample_programs/
        +-- <Processor Type>
            +-- <Programming Language>
                    -Sample program demostrating how to write software for the given processor and programming language

## Compiling or Modifying the Computer Systems

The Computer Systems are build using a set of TCL scripts that run within Altera's Platform Designer software. Complete each of the following sections to modify and update the Computer Systems. Note that you may require licenses for some of Altera's Intellectual Property (IP) core to compile the Computer Systems. Altera University Program members can request IP licenses via the Members section of the Altera University Program website.

### Source Code

Download a copy of this _Design Example Repository_ to your computer, using a tool of your choice such as _Command-Line Git_, _GitHub Desktop_, etc.

### Quartus Prime Software

Download and install an appropriate version of the Quartus Prime software. Links to the download pages for the various versions of the software are found at https://www.altera.com/products/development-tools/quartus-prime. 
We recommend using the latest version of the *Quartus Prime **Pro** Edition* software when targetting the DE25-Standard, DE25-Nano or DE23-Lite boards. We recommend using the latest version of the *Quartus Prime **Standard** Edition* software when targetting the DE10-Standard, DE10-Nano, DE10-Lite or DE1-SoC boards, unless you are using the Nios II processor. In this case, we recommend using version 23.1 of the *Quartus Prime **Standard** Edition* software (Nios II is not included in releases after 23.1).

### IP Cores

The Computer Systems require a set of IP Core components that can be obtained from their GitHub repository, at https://github.com/fpgacademy/IP_Cores. follow the instructions provided in that repository to install these IP Cores.

### Nios II Command Shell

The Tcl scipts that create the Computer Systems are designed to run within the Nios II Command Shell. To use this shell on your operating system:
- Windows:
    1. If not already done, enable Windows Subsystem for Linux (WSL)
    2. Open a command prompt window (such as the Windows Terminal)
    3. Navigate to the *\<altera install directory\>/nios2eds/* directory
    4. Execute the *\"Nios II Command Shell.bat\"* script
- Linux:
    1. Open a terminal window
    2. Navigate to the *\<altera install directory\>/nios2eds/* directory
    3. Execute the *nios2_command_shell.sh* script

### Makefiles

Finally, within the *Nios II Command Shell* navigate to the scripts directory for your choosen board. In this scripts directory, you'll find one or more Makefile, named *.mk, depending on how many systems exist for the board. For example, the DE1-SoC has 3 systems; one for the ARM and Nios II processors (ARM_NiosII.mk), one for the Nios V/g processor (NiosVg.mk) and one for the Nios V/m processor (NiosVm.mk). The Makefile can be run using the *\"make -f \<system name\>.mk\"*, such as *\"make -f ARM_NiosII.mk\"*. The Makefiles have several targets. The main targets are:

- Default: Create the Platform Designer system
- continue: Generates the system's HDL description in Platform Designer and then compiles the circuit using the Quartus Prime software.
- all: Runs both the Default and continue targets

As an example, to create the Nios Vg Computer System for the DE1-SoC board, first run the command "make -f NiosVg.mk". This command will generate a set of .qsys files in the scripts directory. These files specify all components and their interconnections for the computer system that is being made. (If desired, you could now open the Computer_System.qsys file within the Quartus Platform Designer tool and visually examine the components and connections of the computer system.) Now, you can create the computer system hardware circuit by running the command "make -f NiosVg.mk continue". This command will make a folder called "out" in the filesystem location ..\ (the parent folder of the scripts folder). The makefile will copy into the "out" folder the .qsys files and the top-level Verilog and Quartus project files for the computer system. Then, the makefile will run the Platform Designer software to generate the Verilog code for the computer system (cooresponding to the components and connections in the .qsys files), and then compile this Verilog code by using the Quartus Prime software. The compilation process will create an FPGA programming file such as \<system name\>.sof, which can be downloaded into your FPGA board. 

