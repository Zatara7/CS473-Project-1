/* CMPSC 473, Project 1, starter kit
 *
 * Sample program for a pipe
 *
 * See http://www.cse.psu.edu/~dheller/cmpsc311/Lectures/Interprocess-Communication.html
 * and http://www.cse.psu.edu/~dheller/cmpsc311/Lectures/Files-Directories.html
 * for some more information, examples, and references to the CMPSC 311 textbooks.
 */

//--------------------------------------------------------------------------------

/* 	Student: 	Hayder Sharhan
 * 	Last Modified: 	Sun Feb 02
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


// This makes Solaris and Linux happy about waitpid(); it is not required on Mac OS X.
#include <sys/wait.h>

//--------------------------------------------------------------------------------

void err_sys(char *msg);				// print message and quit

// In a pipe, the parent is upstream, and the child is downstream, connected by
// 	a pair of open file descriptors.

void parent_actions(int argc, char *argv[], int fd);	// write to fd
void child_actions(int argc, char *argv[], int fd);	// read from fd
// fd = file descriptor, opened by pipe()
// treat fd as if it had come from open()

#define BUFFER_SIZE 4096

//--------------------------------------------------------------------------------

int main( int argc, char *argv[] ) {
	printf("Demonstration of pipe() and fork()\n");

	int fd[2];					// pipe endpoints
	pid_t child_pid;

	if (pipe(fd) < 0) {				// Open Pipe
		err_sys("pipe error");
	}

	if ( ( child_pid = fork() ) < 0 ) {		// Fork For P2
	
		err_sys("fork error");
	
	} else if ( child_pid > 0 ) {			// this is the parent (P1)
	
		close( fd[0] );				// read from fd[0]
		parent_actions( argc, argv, fd[1] );	// write to fd[1]
		
		if ( waitpid( child_pid, NULL, 0 ) < 0 )// wait for child
	 		err_sys( "waitpid error" );
	
	} else {					// this is the child (P2)
	
		close( fd[1] );				// write to fd[1]
		child_actions( argc, argv, fd[0] );	// read from fd[0]
	
	}

	return( 0 );
}

//--------------------------------------------------------------------------------

// print message and quit

void err_sys( char *msg ) {
	printf( "error: PID %d, %s\n", getpid(), msg );
	exit( 0 );
}

//--------------------------------------------------------------------------------

// write to fd

void parent_actions( int argc, char *argv[], int fd ) {

	char *in_file = "/dev/null";
	int byte_count = 0;
	int ch;

	while ((ch = getopt(argc, argv, "i:o:m:")) != -1 ) { 	// Only need to check for input file

		switch (ch) {
		case 'i':
			in_file = argv[optind-1];
			break;
		}

	}

	FILE *first_file = fopen(in_file, "r");			// Open File Handle
	if (first_file == NULL) {
		err_sys("opening first file didnt work");
	}
	
	FILE *fp = fdopen(fd, "w");				// use fp as if it had come from fopen()
	if (fp == NULL)
		err_sys( "fdopen(w) error" );

	char dummy;

	// Count number of bytes and write to pipe while reading from file !
	while(1) {
		dummy = getc(first_file); 
		if (dummy == EOF)
			break;
		byte_count++;
		putc(dummy, fp);
	}

	// Print message
	printf( "P1: file %s, bytes %i\n", in_file, byte_count );

	// Close things we opened
	fclose( first_file );
	fclose( fp );
	close( fd );

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
	
	char *out_file = "/dev/null";
	int ch;
	int count = 0;
	char *flags[argc];

	while ((ch = getopt(argc, argv, "i:o:m:")) != -1 ) {	// Only need to look for:

		switch (ch) {
		case 'o':					// Output file
			out_file = argv[optind-1];
			break;
		
		case 'm':					// And substrings -> will store in flags[]
			flags[count] = argv[optind-1];
			count++;
	
		}
	}
	
	FILE *fp = fdopen(fd, "r");
	if (fp == NULL)
		err_sys("fdopen(r) error");

	static char buffer[BUFFER_SIZE];

	int ret = setvbuf(fp, buffer, _IOLBF, BUFFER_SIZE);
	if (ret != 0)
		err_sys("setvbuf error (child)");

	int fd2[2];						// Pipe endpoints
	pid_t child_pid;					

	if (pipe(fd2) < 0) {					// Open second pipe
		err_sys("pipe error");
	}

	if ( ( child_pid = fork() ) < 0 ) {			// For for new process (P3)
	
		err_sys("fork error");

	} else if (child_pid > 0) {				// This is the parent (P2)
	
		close(fd2[0]);

		char line[BUFFER_SIZE];
		FILE* fp2w = fdopen(fd2[1], "w");		// Open second pipe for writing

		int occurs[count];
		int lines[count];
		
		// Must initialize to 0 to not get grabage
		memset(occurs, 0, sizeof(occurs));
		memset(lines, 0, sizeof(lines));

		int i = 0;	
		char *line_dummy;				// To read things in without distroying data
		int printme = 0;				// Indicator if line should be written to pipe
		int count_me_in = 0;				// Indicator if lines should be incremented


		while (fgets(line, BUFFER_SIZE, fp) != NULL) {	// Reading from pipe1 line by line
			printme = 0;

			for(i = 0; i < count; i++) {			// Looping through all -m subscripts (if any)
				count_me_in = 0;
				line_dummy = strdup(line);		// Backup out data
				// This next for loop uses strstr to search for the flags[] inside of the lines if more than one occurs,
				for((line_dummy = strstr(line_dummy, flags[i])); line_dummy; line_dummy = strstr(line_dummy + strlen(flags[i]), flags[i])) {
					occurs[i]++;			// This will get incremented
					printme = 1;
					count_me_in = 1;
				}

				if (count_me_in) {			// If we had an occurence
					lines[i]++;			// And update the line occurence counter
				}
			}
			
			if (printme) {
				fputs(line, fp2w);			// Write line to pipe2
			}	
		}
		

		for (i = 0; i < count; i++) {			// For each flag print this statement
			printf("P2: string %s, lines %i, matches %i\n", flags[i], lines[i], occurs[i]);
		}

		// Close many things we opened
		fclose(fp);
		fclose(fp2w);
		close(fd);
		close(fd2[1]);
		fflush(stdout);

		if ( waitpid( child_pid, NULL, 0 ) < 0 )	// Wait for child
	 		err_sys("waitpid error");
	
	} else {						// This is the child (P3)
	
		close(fd2[1]);

		char line2[BUFFER_SIZE];

		FILE *FHR2 = fdopen(fd2[0], "r");		// Open pipe2 for reading
		FILE *OH = fopen(out_file, "w");		// Open the output file
		int lines = 0;

		// Read from pipe2 and write to output file line by line
		while (fgets(line2, BUFFER_SIZE, FHR2) != NULL) {
			fputs(line2, OH);
			lines++;
		}
		printf("P3: file %s, lines %i\n", out_file, lines);

		// Close things we opened
		fclose(FHR2);
		fclose(OH);
		close(fd2[0]);
	}
}

//--------------------------------------------------------------------------------

