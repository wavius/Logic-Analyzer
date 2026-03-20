rem Command as per Hannah
rem asy -V -f pdf Media_FPGA_Audio_Address_Map.asy -gs="C:\MyPrograms\gs\gs9.23\bin\gswin64.exe"
asy Media_FPGA_Audio_Address_Map.asy
asy Media_FPGA_Audio_Address_Map_Out_Only.asy

del *.tex
del texput.aux
del texput.log
