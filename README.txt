What are they?
--------------

Programs included with this README are rudimentary implementations of the shell,
each phase closer to the functionality of the original.



runCommand-specific Notes
-------------------------

runCommand is the first phase program that reads in command line arguments in a
manner similar to the original shell. It treats the first argument as the name
of a program to be executed and all subsequent arguments as the arguments to
the first.

Its mechanics is similar to that of the shell; it creates a parent process to be
perserved and a child process to handle the arguments. One notable purpose of
the parent process is to print statistics about the child process's execution.

It is important to note that while the program is capable of handling most
inputs, it fails to execute <cd> command. It also does not have the
functionality to be terminated with the command line arguments.



shell-specific Notes
--------------------

shell is the second phase program where the concept of changing working
directory is implemented to the previous phase. Thus, a functionality is added
to handle <cd> command. Also, it is modified to be able to exit with the
command line argument <exit>.

When the program detects <cd> command, the subsequent argument is stored as an
address to the new working directory. After confirming the the directory exists,
the program will change its working directory.

In the shell, some limitations are set regarding the command line inputs so
that the number of arguments does not exceed 32 and total characters 128. The
shell will reject such inputs, but it does not terminate.



shell2-specific Notes
---------------------

shell2 is the third and final phase program where background execution
functionality is added onto the previous phase.

When a command to execute the child process as a background task is detected,
the parent process is set not to block and ask for the user input as the task
is being executed. Also, a job struct is created with information of the chlid
process. A list of job is maintained by a listofjobs struct containing a pointer
to the job and a pointer to the next listofjob struct.

Functions are declared to append a job to the last of the listofjobs struct by
have the last point to the new last struct, and to remove a job by linking the
previous listofjob struct with the next, thus skipping the listofjob in the
middle.



How did we test the programs?
-----------------------------

While in development, various arguments, both valid and invalid, are passed
into the programs individually in order to validate the functionalities of the
programs. After development, arguments are passed in as a group, a text file
including possible arguments, and the output file is saved in the same
directory. The output file is examined to validate the programs.



Contacts
--------

If you:
1) need instructions for running the programs, or
2) have a concrete bug report for the programs
contact the developers below:

	- Krzysztof Borowicz
	<kaborowicz@wpi.edu>

	- Hyunsoo Kim
	<hkim@wpi.edu>

	- Jimmy Tran
	<jntran@wpi.edu>