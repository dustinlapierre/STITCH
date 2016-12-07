# STITCH
A prototype shell programmed in C 
by Dustin Lapierre and Albert Sebastian

Simply run the stitch executable from the main folder to access the shell

For a list of commands and their functions type "help" from within the shell

# How to compile

Go to source folder and run these commands

gcc cd.c fatSupport.o helperFunctions.o -o cd <br />
gcc cat.c fatSupport.o helperFunctions.o -o cat <br />
gcc ls.c fatSupport.o helperFunctions.o -o ls <br />
gcc touch.c fatSupport.o helperFunctions.o -o touch <br />
gcc rm.c fatSupport.o helperFunctions.o -o rm <br />
gcc mkdir.c fatSupport.o helperFunctions.o -o mkdir <br />
gcc rmdir.c fatSupport.o helperFunctions.o -o rmdir <br />
gcc pfe.c fatSupport.o -o pfe <br />
gcc pbs.c fatSupport.o -o pbs <br />
gcc df.c fatSupport.o -o df <br />
gcc pwd.c -o pwd <br />

place executables in the commands folder

From main folder

gcc stitch.c -o stitch

