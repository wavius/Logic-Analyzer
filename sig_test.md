# Testing the memory mapped signal capture module
1. Navigate to Computer_Systems/DE1-SoC/src/DE1_SoC_NiosVg_Computer in the project directory
2. Copy the DE1_SoC_NiosVg_Computer.sof file
3. Navigate to fpgacademy/ wherever this is on your computer
4. Replace the existing .sof file with the one copied in step 2
- Make sure to rename the file you copied to the name of the existing one
5. Load GDB as normal with the Logic_Analyzer/sig_test.c file as the program
- If the module is working correctly, LED[0] should turn on then all LEDs should turn off
- testing.c simply writes a value to the trigger control register of the signal capture module, reads it back, and checks if the two values match to see if read/write is working