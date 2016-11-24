# STITCH
A prototype shell programmed in C 
by Dustin Lapierre and Albert Sebastian

Simply run the stitch executable from the main folder to access the shell

For a list of commands and their functions type "help" from within the shell

# How to compile

Go to source folder and run these commands

gcc cd.c fatSupport.o helperFunctions.o -o cd
gcc cat.c fatSupport.o helperFunctions.o -o cat
gcc ls.c fatSupport.o helperFunctions.o -o ls
gcc touch.c fatSupport.o helperFunctions.o -o touch
gcc rm.c fatSupport.o helperFunctions.o -o rm
gcc pfe.c fatSupport.o -o pfe
gcc pbs.c fatSupport.o -o pbs
gcc df.c fatSupport.o -o df
gcc pwd.c -o pwd
mkdir and rmdir not yet implemented

place executables in the commands folder

From main folder

gcc stitch.c -o stitch

