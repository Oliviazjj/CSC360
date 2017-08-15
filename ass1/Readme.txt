
-------------------------explanation of files ----------------------
PMan.c:combination of methods to run a program in the background and continus accepting input from the user at the same time. PMan supports the following commands: bg, bglist, bgkill, bgstop, bgstart, and pstat.
Makefile: compile PMan.c
Readme.txt: instructions of how to use files in this assignment
inf.c: a sample program which allows the user to act a trival background process and performs re-starting, stopping, abd kiling commands.

------------------------------run code------------------------------

1. run make (compile PMan.c)

2. run ./PMan

3. It prompts
	PMan: >
4. available commands:
	a. bgkill <pid>	 
		this command is to terminate the job of pid
	b. bgstop <pid>
		this command is to stop the job temporarily
	c. bgstart <pid>
		this command is to re-start the stopped job
	d. bg [command]
		this command is to execute the command in the background
		for example: bg ./inf
	e. bglist <pid>
		this command is to list all of the precesses currently running in the background
	f. pstat <pid>
		this command is to list the comm(filename of executable), state, utime, stime, rss(resident set size), voluntary_ctxt_swithes and nonvoluntary_ctxt_swithes of a process with given pid
	
5. To exit the program, Ctrl+Shift+Z(depends on the setting of computer)

