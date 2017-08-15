
Jingjing Zhu
CSC360 Assignment #2
-------------------------explanation of file ----------------------
This project is used to make a task scheduler
MFS.c:
	1.main(int argc, char* argv): open the file provides in argv[1] and exectues the others methods in order to transmit flows ehihc are read from input file.
	2.thrFunction(void *flowItem): create a thread corresponding to the flowItem, and execute this function and its tasks for each thread.
	3.requestPipe(flow *item): send a request to pipe. If  the pipe is available and queue is empty, directly start transmission. If the pipe is not available, put this thread into queue and sort the queue. Then wait until the pipe is available again and find the flow with highest priority, then transmit that flow.
	4. releasepipe(): release the pipe and tell all the threads that the pipe is available again.
	5. get_time(): get the relative time to the starting time.
flow.txt: the number and the details of the flow, whihc can be used as input file.
Makefile: compile MFS.c
Readme.txt: instructions of how to use files in this assignment

------------------------------run code------------------------------

1. run make (compile MFS.c)

2. run ./MFS [file_name]
	[file_name]: the name of the input file which includes flows to be transmitted
	For example: ./MFS flow.txt 

Caution: the max number of flow here is set into 100; If the number of flows is higher than 100, please change the value of MAXFLOW in MFS.c.

