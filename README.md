1. Gilbert Adomako
   Mo Mohammed

2. adoma016
   moha1859

3. Gilbert: Implemented counter.c
   Mo: Implemented logger.c

4. No lab machine used for testing. Ran our code on personal linux machine running ubuntu. and personal macbooks

5.
The main program opens the specified directory and scans only the files in that particular directory without scanning its subdirectories. 
The entries . and .. are not taken into account. For every regular file detected the main program launches a new child process using fork(). 
This child process executes the separate counter program by calling execl() from counter.c. As an argument to this function the file path is used.
The program opens the particular file, reads it line by line using getline() and counts corrupted characters, using isascii(). 
It counts the following parameters: The number of corrupted characters and the number of lines containing at least one corrupted character.
The program creates a .log file based on the basename of the original file. 
In the first line it writes the number of corrupted characters, in the second line the number of corrupted lines.
Then the main process waits for the end of work of all child processes using waitpid(). 
When all child processes are finished, it reads .log files and outputs their contents and the total number of corrupted characters and corrupted lines

One challenge was creating the correct .log filename for each input file. The program needed to replace the original file extension with .log while still keeping the correct directory path. 
This was handled using string functions such as strrchr() and dynamic memory allocation.
Another challenge was making sure the parent process waited for every child process to finish. If the parent exited too early, some child processes might not complete their work.
This was fixed by storing each child process ID and using waitpid() to wait for them.
A final challenge was reading files safely. Instead of using a fixed size buffer, 
the program uses getline(), which can handle long lines without cutting them off. This helps make sure corrupted lines are counted correctly.

6. AI was used in getting a better understanding of the project requrements and different syntax and tools that would achive what we wanted
to acomplish. And example promote we used was " how can you prevent memory leaks when fork is call prior and a child calls execl".

