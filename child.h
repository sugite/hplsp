typedef struct 
{
	pid_t	child_pid;
	int		child_pipefd;
	int		child_status;
	long 	child_count;
}Child;

Child *cptr;