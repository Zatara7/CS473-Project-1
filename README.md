Original Project Writeup:

CMPSC 473, Project 1

Posted Jan. 20, 2014.  Due Feb. 6, 2014.  50 points.
Feb. 3, some more examples added
This project should be done by each student individually, without assistance from others in the class or elsewhere.  If you did get assistance from anyone else, specify the name of the other person, what you did, what the other person did, and how much of the work was done by you.  For example, it's allowed to have someone else explain fork() to you, or to explain fork()-related bugs in general, but not to explain where the fork()-related bugs in your program are and how to fix them.

There is an ANGEL Dropbox available.  Please turn in a file named pr1.c that is a modified version of the file pr1pipe.c provided here; this should be an expanded version of the program, with additional comments describing your changes.  Please turn in a second file named pr1.txt that is a plain Unix or DOS text file; this should contain any discussion that you didn't put into the program as a comment.  Don't forget to put your name, email address and PSU Access ID at the top of each file.

Please, no zip files, etc., just two files named pr1.c and pr1.txt, and the second one is optional.


The code provided here should compile and run without change.  If it does not, send information to the instructor, and we'll investigate.

Two files are provided:
Makefile is for use with the Unix make program.  There are separate entries for compiling with Solaris, Linux and Mac OS X.
pr1pipe.c is the initial version of the code, in C.  There are a few comments to help explain how the program is constructed, and what it is trying to do.
The program creates two processes connected through a Unix pipe, by first calling pipe() and then fork().  Some data is sent from the first process to the second, and then the program ends.  Extra printing is used to help explain what happens.

For access to a Solaris system, use eru.cse.psu.edu; this is an old system which has not been updated in several years.  For access to a current Linux system, use ladon.cse.psu.edu, or one of the workstations in 218 IST.  For access to a Mac OS X system, use one of the PSU student labs.  Your own Linux or Mac, or Linux in a virtual machine on Windows, should present no problems.


A general discussion and examples of Unix process manipulation can be found in OSC Sec. 3.3.  A general discussion of pipes can be found in OSC Sec. 3.6.3.

From the CMPSC 311 lecture notes, 
For a discussion of fork(), see Process-Control-Signals; the most relevant part starts after "Now it's time to create some more processes.", about halfway through.
For more about pipe(), see Interprocess-Communication, in the section "Interprocess Pipes with pipe()".
You may need to review the discussion of file descriptors, read() and write() in System-Level-I/O.


Here is what you should do.  Create three processes that are connected by two pipes, effectively P1 | P2 | P3 in Unix command shell syntax.  P1 reads from a first file and writes on the first pipe (to P2); P2 reads from the first pipe (from P1) and writes on the second pipe (to P3); P3 reads from the second pipe (from P2) and writes to a second file.  Note that P2 really only needs to know about the two pipes, and not about processes P1 and P3.

The names of the two files are given on the command line with options -i and -o (short for in and out); the default file name for each is /dev/null .  (Recall, the file /dev/null is always empty, so reading from /dev/null always yields end-of-file.)

P1, P2 and P3 should ignore standard input, should write only a few diagnostic or progress messages to standard output, and should send error messages to standard error.  (Note that pr1pipe.c doesn't follow this rule; it sends error messages to stdout.)

P1 should copy bytes from the input file to its output pipe, without any changes.  When the end-of-input is reached, P1 should print to stdout the name of the input file, and the number of bytes that were read.  Make it clear on the output that it came from P1.  For example, you could use  printf("P1: file %s, bytes %d\n", ...);

P2 should read bytes from its input pipe and write text lines to its output pipe; the text lines should each end with a newline character, Unix-style.  Essentially, P2 acts as a recognizer and filter for character strings that appear in the input.  The command line option -m indicates which string to look for; this option can be repeated without limit.  (There is a system-imposed limit on the length of a command line, so there is a practical limit to the -m options, but your program should not be designed to take advantage of this fact.)  For example, with the option  -m foo , and the input substring  Food! food! give me food!  there are two matches.  With the option  -m oo , and the same input substring, there are three matches.  It's up to you to decide if  -m oo  matches fooood two or three times; two is probably easier to implement, and agrees with most searches in editors and browsers.  When the end-of-input is reached, P2 should print to stdout the number of lines that matched, one line per -m option that was supplied.  Make it clear on the output that it came from P2.  For example, you could use  printf("P2: string %s, matches %d\n", ...);

P3 should copy text lines from its input pipe to its output file, without any changes, although the ordering of the text lines is allowed to vary from one run to the next.  When the end-of-input is reached, P3 should print to stdout the name of the output file and the number of lines that were read.  Make it clear on the output that it came from P3.  For example, you could use  printf("P3: file %s, lines %d\n", ...);


A really slick version of P2 would use a separate thread for each -m option.  That will be the essence of Project 2.  There are a number of problems with the multi-thread version that will require synchronization between the threads, which we haven't covered in class yet, so it's probably better to wait until later to attempt this. 


For grading purposes, your program should compile and run on one of the Linux systems in 218 IST, or on the Linux server ladon.cse.psu.edu.  Fewer warnings from GCC will get you a higher grade.  If the program won't compile using the obvious modification of the Makefile provided, to rename the source and object files, then we will attempt to read it, but we won't attempt to rewrite it.  If the program compiles and runs but doesn't terminate in a reasonable amount of time, we will kill it; this may cause buffered output to be lost.  Don't assume that the testing will be done manually at a terminal window; it will be done with a shell script.


Here is an example that may help clarify what the program should do.  The file x has 62 bytes, and it's in Unix text format, a single newline character at the end of each line.  % is the command-line prompt.  The commands entered are highlighted.  The executable program is a.out .

% cat x
one
two
three
twenty-one
twenty-two
Food, food, foood, fooood

% ./a.out -i x -o y -m one -m ood -m oo
P1: file x, bytes 62
P2: string oo, lines 1, matches 5
P2: string ood, lines 1, matches 4
P2: string one, lines 2, matches 2
P3: file y, lines 3

% cat y
one
twenty-one
Food, food, foood, fooood

% ./a.out -i x -o y -m blat
P1: file x, bytes 62
P2: string blat, lines 0, matches 0
P3: file y, lines 0

% ./a.out -i x -o y
warning: no -m option was supplied
P1: file x, bytes 62
P3: file y, lines 0

% ./a.out
warning: no -m option was supplied
P1: file /dev/null, bytes 0
P3: file /dev/null, lines 0

The ordering of lines printed by P2 is arbitrary; yours can differ and still be considered correct.


Examples added

Here is the original version, pr1pipe.c.
% ./pr1
Demonstration of pipe() and fork()
 1: PID, PPID: 47969 29124
[printed to stdout by 47970] [printed to pipe by 47969] greetings
 2: PID, PPID: 47970 47969
 2: PID, PPID: 47969 29124

% ./pr1 foo
Demonstration of pipe() and fork()
 1: PID, PPID: 47971 29124
[printed to stdout by 47972] [printed to pipe by 47971] foo
 2: PID, PPID: 47972 47971
 2: PID, PPID: 47971 29124
Starting from pr1pipe.c, now renamed pr1.1.c, get the third process connected to the second pipe.  The command-line options are not there yet, so the -o option defaults to stdout, as in pr1pipe.c.
% ./pr1
Demonstration of pipe() and fork()
 1: PID, PPID: 47982 29124
[printed to file by 47984] [printed to pipe by 47983] [printed to pipe by 47982] greetings
 2: PID, PPID: 47984 47983
 2: PID, PPID: 47983 47982
 2: PID, PPID: 47982 29124

% ./pr1 foo
Demonstration of pipe() and fork()
 1: PID, PPID: 47986 29124
[printed to file by 47988] [printed to pipe by 47987] [printed to pipe by 47986] foo
 2: PID, PPID: 47988 47987
 2: PID, PPID: 47987 47986
 2: PID, PPID: 47986 29124
Next, in pr1.2.c, the command-line options -i and -o are used in main().  We also used the Unix convention that filename "-" stands for standard input with -i, or standard output with -o.  This helps with debugging, even though the project description says not to do it.
% cat inf
input 1
input 2

% ./pr1 -i inf -o outf
Demonstration of pipe() and fork()
 1: PID, PPID: 47999 29124
 2: PID, PPID: 48001 48000
 2: PID, PPID: 48000 47999
 2: PID, PPID: 47999 29124

% cat outf
[printed to file by 48001] [printed to pipe by 48000] [printed to pipe by 47999] input 1

% echo foo | ./pr1 -i - -o -
Demonstration of pipe() and fork()
 1: PID, PPID: 48006 29124
[printed to file by 48008] [printed to pipe by 48007] [printed to pipe by 48006] foo
 2: PID, PPID: 48008 48007
 2: PID, PPID: 48007 48006
 2: PID, PPID: 48006 29124
Next, in pr1.3.c, each process reads its entire input, and P1 and P3 do their counting.  The string matching in P2 is not attempted yet.  The wc command reports lines, words and bytes.
% wc inf
       2       4      16 inf

% ./pr1 -i inf -o outf
Demonstration of pipe() and fork()
 1: PID, PPID: 48599 29124
P1: file inf, bytes 16
P2: finished
P3: file outf, lines 2
 2: PID, PPID: 48601 48600
 2: PID, PPID: 48600 48599
 2: PID, PPID: 48599 29124

% cat outf
input 1
input 2
Next, in pr1.4.c, the string matching is installed.
% cat x
one
two
three
twenty-one
twenty-two
Food, food, foood, fooood

% ./pr1 -i x -o y -m one -m ood -m oo
Demonstration of pipe() and fork()
 1: PID, PPID: 49800 29124
P1: file x, bytes 62
P2: string one, lines 2, matches 2
P2: string ood, lines 1, matches 4
P2: string oo, lines 1, matches 5
P3: file y, lines 3
 2: PID, PPID: 49803 49802
 2: PID, PPID: 49802 49800
 2: PID, PPID: 49800 29124

% cat y
one
twenty-one
Food, food, foood, fooood

% ./pr1 -i x -o y -m blat
Demonstration of pipe() and fork()
 1: PID, PPID: 49805 29124
P1: file x, bytes 62
P2: string blat, lines 0, matches 0
P3: file y, lines 0
 2: PID, PPID: 49807 49806
 2: PID, PPID: 49806 49805
 2: PID, PPID: 49805 29124

% ./pr1 -i x -o y 
Demonstration of pipe() and fork()
 1: PID, PPID: 49808 29124
P1: file x, bytes 62
P3: file y, lines 0
 2: PID, PPID: 49810 49809
 2: PID, PPID: 49809 49808
 2: PID, PPID: 49808 29124

% ./pr1 
Demonstration of pipe() and fork()
 1: PID, PPID: 49811 29124
P1: file /dev/null, bytes 0
P3: file /dev/null, lines 0
 2: PID, PPID: 49813 49812
 2: PID, PPID: 49812 49811
 2: PID, PPID: 49811 29124
After some cleanup, we're done.  The string match output ordering differs from the previous example, but that's not a problem.


Last revised, 20 January 2014
3 Feb., examples added