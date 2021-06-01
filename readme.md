# Shell Implementation

> ​	*File components: cpp files, make file (with cmake stuffs), executable file minish*
>
> ​	*Developer: Geonwoo Cho* 
>
> ​	This document is written as a markdown file. (https://github.com/Cho-Geonwoo/shell_implementation)



## (a) Compiling method: 

I compiled the files on the WSL environment. After finding HW3_SP file and moving to that directory, you can compile .cpp files by using command __make__. You should be sure to download make and cmake by using __sudo apt install make & sudo apt install cmake__ (If you are using ubuntu) etc. 



## (b) & (c) Functionality implemented and the method to implement it:

1. __Basic execution of commands__ are implemented.

   <img src = "https://raw.githubusercontent.com/Cho-Geonwoo/img/master/shell%201.png" width = "60%"> 

   After getting an input from an user, I parsed the commands using strtok function and while loop. (To watch details, please watch parsecmd function on the main.cpp source code.) Then, I used fork-exec method to execute the commands. A child process runs the command and a parent process get wait until the child process gives an exit status. (For details, please watch execute part on the main.cpp source code.)

   

2. __Ctrl-C signal handler__ and the __built-in command "quit"__ is implemented.<img src = "https://raw.githubusercontent.com/Cho-Geonwoo/img/master/shell%202.png" width = "60%">

   First, I blocked all signal except SIGINT and SIGCHLD using sigprocmask function. Then, I add a signal handler for SIGINT signal to clear current running process. If the command is quit, "minish" clears all running processes and quit "minish" by calling exit function.

   

3. __Current directory path is printed__ in a prompt.<img src = "https://raw.githubusercontent.com/Cho-Geonwoo/img/master/shell%203.png" width = "60%">

   If "minish" is created, the variable pwd which stores current directory information is initialized by getcwd function. If a current directory path is changed by the command cd, the variable pwd will be updated. Every time you use "minish", current directory path will be printed in the prompt

   

4. __Redirection__ is implemented.<img src = "https://raw.githubusercontent.com/Cho-Geonwoo/img/master/shell%204.png" width = "80%">

   If the redirection command is detected while parsing the commands, make a temporary file descriptor which directs ordinary stdout file descriptor. Then, change a stdout file descriptor using a dup2 function to point out the file descriptor we want to redirect for. Subsequently, we'll execute the commands which the results will be stored on the redirected file (On the picture above, the result of ls -al is stored at a.txt file). Finally, we'll bring back the stdout file descriptor into the ordinary one by using the temporary file descriptor.



5. __Pipe__ is implemented.

   <img src = "https://raw.githubusercontent.com/Cho-Geonwoo/img/master/shell%205.png" width = "80%">
   
   By using fork, pipe, and dup2 function, it was able to implement pipe. (For details, watch fork_pipes part in the main.cpp source code.)
   
   
   
6. __Background process__ and the built in command __jobs__ and __kjob__ for Background processes are implemented.<img src = "https://raw.githubusercontent.com/Cho-Geonwoo/img/master/shell%206.png" width = "90%">

   After the background process is being executed, my own sigchild handler catches a SIGCHLD signal and prints the done message. By storing all the informations of background processes, it was able to implement the built-in command jobs and kjobs. Especially, for kjobs, not only deleting the information about the background process, but also killing the background process by using kill function is implemented.

   

7. Built-in commands __cd, path,__ and __status__ are implemented.

   <img src = "https://raw.githubusercontent.com/Cho-Geonwoo/img/master/shell%207.png" width = "100%">

   The "cd" command is implemented by using the chdir function. The "path" command is implemented by using setenv and getenv function. The status command is implemented by storing every exit information in a variable prev_exit_status.

   

8. __Wildcard processing__ is implemented.

   <img src = "https://raw.githubusercontent.com/Cho-Geonwoo/img/master/shell%208.png" width = "100%">

   Wildcard processing is implemented by using readdir and fnmatch command at the command parsing level.

   

## (d) Conclusion

It was able to implement all major functions in the shell. Maybe there can be some bugs, but for simple command groups, it worked very well. If you have any questions, please contact to the email below. Thank you.



----

> ​	*Email: joungju257@gist.ac.kr*
>
