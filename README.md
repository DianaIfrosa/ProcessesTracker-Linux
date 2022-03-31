# ProcessesTracker-Linux

### Team member: [Tudor Cosmin Oanea](https://github.com/TudorCosmin)  
This Linux project is written in C and keeps track of active processes of each user that is logged in.  
Whenever a user authenticates (with username and password verified from a file named 'users.txt'), there will be a folder created for it (with user's name) where active processes will be listed in a file named 'procs.txt'. 
Then the user can do commands such as 'ls', 'cat' that displays content of procs.txt file from user folder, a custom command named 'nru' which displays the number of users connected in that moment or 'exit'. 
