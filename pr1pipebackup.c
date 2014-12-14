/* CMPSC 473, Project 1, starter kit
 *
 * Sample program for a pipe
 *
 * See http://www.cse.psu.edu/~dheller/cmpsc311/Lectures/Interprocess-Communication.html
 * and http://www.cse.psu.edu/~dheller/cmpsc311/Lectures/Files-Directories.html
 * for some more information, examples, and references to the CMPSC 311 textbooks.
 */

//--------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


// This makes Solaris and Linux happy about waitpid(); it is not required on Mac OS X.
#include <sys/wait.h>

//--------------------------------------------------------------------------------

void err_sys(char *msg);								// print message and quit

// In a pipe, the parent is upstream, and the child is downstream, connected by
// 	a pair of open file descriptors.

void parent_actions(int argc, char *argv[], int fd);	// write to fd
void child_actions(int argc, char *argv[], int fd);		// read from fd
// fd = file descriptor, opened by pipe()
// treat fd as if it had come from open()

#define BUFFER_SIZE 4096

//--------------------------------------------------------------------------------

int main( int argc, char *argv[] ) {
	printf("Demonstration of pipe() and fork()\n");

	printf(" 1: PID, PPID: %d %d\n", getpid(), getppid());	// make some noise

	int fd[2];					// pipe endpoints
	pid_t child_pid;

	if (pipe(fd) < 0) {
		err_sys("pipe error");
	}

	if ( ( child_pid = fork() ) < 0 ) {
		err_sys("fork error");
	} else if (child_pid > 0) {	// this is the parent
		close(fd[0]);								// read from fd[0]
		parent_actions( argc, argv, fd[1] );		// write to fd[1]
		if ( waitpid( child_pid, NULL, 0 ) < 0 )	// wait for child
	 		err_sys("waitpid error");
	} else {					// this is the child
		close(fd[1]);								// write to fd[1]
		child_actions(argc, argv, fd[0]);			// read from fd[0]
	}

	printf(" 2: PID, PPID: %d %d\n", getpid(), getppid());	// make some noise

	return 0;
}

//--------------------------------------------------------------------------------

// print message and quit

void err_sys(char *msg) {
	printf("error: PID %d, %s\n", getpid(), msg);
	exit(0);
}

//--------------------------------------------------------------------------------

// write to fd

void parent_actions(int argc, char *argv[], int fd) {
	/* If you want to use read() and write(), this works:
	*
	* write(fd, "hello world\n", 12);
	*/

	int byte_count = 0;

	// open first file to read from
	FILE *first_file = fopen(argv[2], "r");
	if (first_file == NULL) {
		err_sys("opening first file didnt work");
	}
	
	FILE *fp = fdopen(fd, "w");		// use fp as if it had come from fopen()
	if (fp == NULL)
		err_sys( "fdopen(w) error" );

	char dummy;

	// count number of bytes and write to
	while(1) {
		dummy = getc(first_file); 
		if (dummy == EOF)
			break;
		byte_count++;
		putc(dummy, fp);
	}

	// The following is so we don't need to call fflush(fp) after each fprintf().
	// The default for a pipe-opened stream is full buffering, so we switch to line
	//   buffering.
	// But, we need to be careful not to exceed BUFFER_SIZE characters per output
	//   line, including the newline and null terminator.

	static char buffer[BUFFER_SIZE];	// off the stack, always allocated
	int ret = setvbuf(fp, buffer, _IOLBF, BUFFER_SIZE);	// set fp to line-buffering
	if (ret != 0)
		err_sys("setvbuf error (parent)");

	// use text from command line, with default
	fprintf(fp, "[printed to pipe by %d] %s\n", getpid(), (argc > 1) ? argv[2] : "greetings");
	fflush(fp);
}

//--------------------------------------------------------------------------------

// read from fd

void child_actions(int argc, char *argv[], int fd) {
	/* If you want to use read() and write(), this works:
	*
	* char line[BUFFER_SIZE];
	*
	* int n = read(fd, line, BUFFER_SIZE);
	* write(STDOUT_FILENO, line, n);
	*/

	// see parent_actions() for similar code, with comments
	FILE *fp = fdopen(fd, "r");
	if (fp == NULL)
		err_sys("fdopen(r) error");

	static char buffer[BUFFER_SIZE];
	int ret = setvbuf(fp, buffer, _IOLBF, BUFFER_SIZE);
	if (ret != 0)
		err_sys("setvbuf error (child)");

	int fd2[2];					// pipe endpoints
	pid_t child_pid;

	if (pipe(fd2) < 0) {
		err_sys("pipe error");
	}

	if ( ( child_pid = fork() ) < 0 ) {
		err_sys("fork error");
	} else if (child_pid > 0) {	// this is the parent
		char line[BUFFER_SIZE];

		read(fd, line, BUFFER_SIZE);
		
		fprintf(stdout, "[printed to stdout by %d] %s", getpid(), line);
		fclose(fp);
		fflush(stdout);
	
		write(fd2[1], line, BUFFER_SIZE);

		if ( waitpid( child_pid, NULL, 0 ) < 0 )// wait for child
	 		err_sys("waitpid error");
	} else {			// this is the child		
		char line2[BUFFER_SIZE];

		read( fd2[0], line2, BUFFER_SIZE );

		//printf(strlen(line2)+1);

		write( open(argv[4], O_WRONLY | O_CREAT, S_IRGRP | S_IRUSR | S_IROTH), 
				line2, strlen(line2));
	}


}

//--------------------------------------------------------------------------------

