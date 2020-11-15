###### System Programming - Shell Implementation

------------------------------------------------------------------

> ​	*File components: cpp files, make file (with cmake stuffs), executable file minish*



* (a) Compiling method: 

  I compiled the files on the WSL environment. After finding HW3_SP file and moving to that directory, you can compile .cpp files by using command __make__. You should be sure to download make and cmake by using __sudo apt install make & sudo apt install cmake__ (If you are using ubuntu) etc. 

  

* (b) & (c) Functionality implemented and the method to implement it:

  1. Basic execution of commands are implemented.

     <img src = "C:\Users\USER\AppData\Roaming\Typora\typora-user-images\image-20201116005903824.png" width = "60%"> 

     After getting an input from an user, I parsed the commands using strtok function and while loop. (To watch details, please watch parsecmd function on the main.cpp source code.) Then, I used fork-exec method to execute the commands. A child process runs the command and a parent process get wait until the child process gives an exit status. (For details, please watch execute part on the main.cpp source code.)

     

  2. Own signal handler and the built-in command "quit" is implemented.