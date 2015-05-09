#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include "job.h"

 #define DEBUG
void jobstate(struct jobinfo *job,char *str);


int jobid=0;
int siginfo=1;
int fifo;
int globalfd;

struct waitqueue *head1=NULL, *head2=NULL, *head3=NULL; // 修改点@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
struct waitqueue *next=NULL, *current =NULL;
void queueEnd(struct waitqueue *newnode); // 修改点@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void queueHead(struct waitqueue *newnode);
int checkRunTime(struct waitqueue* p);
void jobstate(struct jobinfo *job,char *str);

/* 调度程序 */
void scheduler()
{
	struct jobinfo *newjob=NULL;
	struct jobcmd cmd;
	int  count = 0;
	bzero(&cmd,DATALEN);//初始化操作
	if((count=read(fifo,&cmd,DATALEN))<0)
		error_sys("read fifo failed");
#ifdef DEBUG

	if(count){
		printf("cmd cmdtype\t%d\ncmd defpri\t%d\ncmd data\t%s\n",cmd.type,cmd.defpri,cmd.data);
	}
	else
		printf("no data read\n");
#endif

	/* 更新等待队列中的作业 */
/* 调试任务3@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
printf("update jobs in wait queue!\n");
#endif

/* 调试任务6@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
printf("before updateAll:\n");
do_stat(cmd);
#endif
	updateall();
/* 调试任务6@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
printf("after updateAll:\n");
do_stat(cmd);
#endif
	/* 对从FIFO中读取的cmd进行处理@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
	switch(cmd.type){
	case ENQ:
/* 调试任务7@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
printf("before do_enq:\n");
do_stat(cmd);
#endif
#ifdef DEBUG
printf("Execute enq!\n");
#endif
		do_enq(newjob,cmd);
#ifdef DEBUG
printf("after do_enq:\n");
do_stat(cmd);
#endif
		break;
	case DEQ:
#ifdef DEBUG
printf("before do_deq:\n");
do_stat(cmd);
#endif
#ifdef DEBUG
printf("Execute deq!\n");
#endif
		do_deq(cmd);
#ifdef DEBUG
printf("after do_deq:\n");
do_stat(cmd);
#endif
		break;
	case STAT:
#ifdef DEBUG
printf("before do_stat:\n");
do_stat(cmd);
#endif
#ifdef DEBUG
printf("Execute stat!\n");
#endif
		do_stat(cmd);
#ifdef DEBUG
printf("after do_stat:\n");
do_stat(cmd);
#endif
		break;
	default:
		break;
	}

	/* 选择高优先级作业 */
/* 调试任务3@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
printf("select which job to run next!\n");
#endif
	next=jobselect();

	/* 作业切换 */
/*调试任务3@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
printf("switch to the next jobs\n");
#endif

/* 调试任务9@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
printf("before jobswitch:\n");
do_stat(cmd);
#endif
	jobswitch();
#ifdef DEBUG
printf("after jobswitch:\n");
do_stat(cmd);
#endif
}

int allocjid()
{
	return ++jobid;
}


// 修改点 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void updateall()
{
	struct waitqueue *p, *tmp;
// printf("111111111111111111111111\n");////@##################################@@@@#####
	/* 更新作业运行时间 */
	if (current){
		current->job->run_time += 1; /* 加1代表1000ms */
		current->job->constant_time += 1;
	}
// printf("2222222222222222222222222\n");////@##################################@@@@#####
	/* 更新作业等待时间及优先级 */
	for (p = head1; p != NULL; p = p->next)
		p->job->wait_time += 1000;
	for (p = head2; p != NULL; p = p->next)
		p->job->wait_time += 1000;
	for (p = head3; p != NULL; p = p->next)
		p->job->wait_time += 1000;
// printf("33333333333333333333333333\n");////@##################################@@@@#####
	while (head1!=NULL && head1->job->wait_time >= 10000 && head1->job->curpri < 3){
		tmp = head1;
		head1 = head1->next;
		tmp->next = NULL;
		tmp->job->curpri++;
		tmp->job->wait_time = 0;
		queueEnd(tmp);
	}
// printf("4444444444444444444444444444\n");////@##################################@@@@#####
	if (head1 != NULL){
		tmp = head1;
		p = head1->next;
		while (p != NULL){
			if (p->job->wait_time >= 10000 && p->job->curpri < 3){
				p->job->curpri++;
				p->job->wait_time = 0;
				tmp->next = p->next;
				p->next = NULL;
				queueEnd(p);
				p = tmp;
			}
			tmp = p;
			p = p->next;
		}
	}
// printf("5555555555555555555555555555\n");////@##################################@@@@#####
	while (head2!=NULL && head2->job->wait_time >= 10000 && head2->job->curpri < 3){
		tmp = head2;
		head2 = head2->next;
		tmp->next = NULL;
		tmp->job->curpri++;
		tmp->job->wait_time = 0;
		queueEnd(tmp);
	}
// printf("66666666666666666666666666666\n");////@##################################@@@@#####
	if (head2 != NULL){
		tmp = head2;
		p = head2->next;
		while (p != NULL){
			if (p->job->wait_time >= 10000 && p->job->curpri < 3){
				p->job->curpri++;
				p->job->wait_time = 0;
				tmp->next = p->next;
				p->next = NULL;
				queueEnd(p);
				p = tmp;
			}
			tmp = p;
			p = p->next;
		}
	}
// printf("99999999999999999999999999999999\n");////@##################################@@@@#####
}

struct waitqueue* jobselect()
{
	struct waitqueue *select;
/* 调试任务8@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
char timebuf[BUFLEN];
#endif

	select = NULL;
	
	// 先从最高优先级开始找，依次查询
	// 每一次都是取出头节点，放回时放回尾部。
	if(head3){
	select=head3;
	head3=head3->next;
	select->next=NULL;
	}
	else if(head2){
	select=head2;
	head2=head2->next;
	select->next=NULL;
	}
	else if(head1){
	select=head1;
	head1=head1->next;
	select->next=NULL;
	}
	else{
	select=NULL;
	}

/* 调试任务8@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
if(select==NULL)
{
	printf("the select job is null!\n");
}
else
{
	printf("the select job:\n");
	strcpy(timebuf,ctime(&(select->job->create_time)));
	timebuf[strlen(timebuf)-1]='\0';
	printf("jobid:%d\npid:%d\nowner:%d\nruntime:%d\nwaittime:%d\nconstanttime:%d\ndefpri:%d\ncurpri:%d\ncreattime:%s\nstate:%s\n",
		select->job->jid,
		select->job->pid,
		select->job->ownerid,
		select->job->run_time,
		select->job->wait_time,
		select->job->constant_time,
		select->job->defpri,
		select->job->curpri,
		timebuf,"Ready");	
}
#endif
	return select;
}


void jobswitch()
{
	struct waitqueue *p;
	int i;
// 修改点 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@2
if (next != NULL && current == NULL)
{
printf("begin start new job\n");
}
	if(current && current->job->state == DONE){ /* 当前作业完成 */
		/* 作业完成，删除它 */
		for(i = 0;(current->job->cmdarg)[i] != NULL; i++){
			free((current->job->cmdarg)[i]);
			(current->job->cmdarg)[i] = NULL;
		}
		/* 释放空间 */
		free(current->job->cmdarg);
		free(current->job);
		free(current);

		current = NULL;
	}

	if(next == NULL && current == NULL) /* 没有作业要运行 */

		return;
	else if (next != NULL && current == NULL){ /* 开始新的作业 */
		current = next;
		next = NULL;
		current->job->state = RUNNING;
		kill(current->job->pid,SIGCONT);
#ifdef DEBUG
printf("send SIGCONT to current\n"); // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#endif
		return;
	}
	else if (next != NULL && current != NULL){ /* 切换作业 */
// 修改点 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@2
// 切换的条件：next的优先级高于current，同优先级且current的时间片到
if(next->job->curpri > current->job->curpri || (next->job->curpri == current->job->curpri && checkRunTime(current) ))
{

		printf("switch to Pid: %d\n",next->job->pid);
		kill(current->job->pid,SIGSTOP);
		current->job->curpri = current->job->defpri ; /// 
		current->job->wait_time = 0;
		current->job->constant_time = 0; // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		current->job->state = READY;

		/* current需要放回等待队列尾部 */
		queueEnd(current); // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
		current = next;
		next = NULL;
		current->job->state = RUNNING;
		current->job->wait_time = 0;
		current->job->constant_time = 0;
		kill(current->job->pid,SIGCONT);
		return;
}
else // 不切换，继续运行current    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
{
// 需要把next重新放回相应队列的头部！！！
queueHead(next);
next=NULL;
// kill(current->job->pid,SIGCONT);  
return ;
}

	}else{ /* next == NULL且current != NULL，不切换 */
//		kill(current->job->pid,SIGCONT);   // 修改点 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		return;
	}
}

void sig_handler(int sig,siginfo_t *info,void *notused)
{
	int status;
	int ret;
/* 调试任务10 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
struct jobcmd cmd;
#endif

	switch (sig) {
case SIGVTALRM: /* 到达计时器所设置的计时间隔 */
	scheduler();

/* 调试任务2 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
printf("SIGVTSLRM receives!\n");
#endif
	return;

case SIGCHLD: /* 子进程结束时传送给父进程的信号 */
	ret = waitpid(-1,&status,WNOHANG);
	if (ret == 0)
		return;
	if(WIFEXITED(status)){
		current->job->state = DONE;
		printf("normal termation, exit status = %d\n",WEXITSTATUS(status));
/* 调试任务10 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
do_stat(cmd);
#endif
	}else if (WIFSIGNALED(status)){
		printf("abnormal termation, signal number = %d\n",WTERMSIG(status));
	}else if (WIFSTOPPED(status)){
		printf("child stopped, signal number = %d\n",WSTOPSIG(status));
	}
	return;
	default:
		return;
	}
}

void do_enq(struct jobinfo *newjob,struct jobcmd enqcmd)
{
	struct waitqueue *newnode,*p;
	int i=0,pid;
	char *offset,*argvec,*q;
	char **arglist;
	sigset_t zeromask;

	sigemptyset(&zeromask);

	/* 封装jobinfo数据结构 */
	newjob = (struct jobinfo *)malloc(sizeof(struct jobinfo));
	newjob->jid = allocjid();
	newjob->defpri = enqcmd.defpri;
	newjob->curpri = enqcmd.defpri; // ///////
	newjob->ownerid = enqcmd.owner;
	newjob->state = READY;
	newjob->create_time = time(NULL);
	newjob->wait_time = 0;
	newjob->run_time = 0;
	newjob->constant_time = 0; // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	arglist = (char**)malloc(sizeof(char*)*(enqcmd.argnum+1));
	newjob->cmdarg = arglist;
	offset = enqcmd.data;
	argvec = enqcmd.data;
	while (i < enqcmd.argnum){
		if(*offset == ':'){
			*offset++ = '\0';
			q = (char*)malloc(offset - argvec);
			strcpy(q,argvec);
			arglist[i++] = q;
			argvec = offset;
		}else
			offset++;
	}

	arglist[i] = NULL;

#ifdef DEBUG

	printf("enqcmd argnum %d\n",enqcmd.argnum);
	for(i = 0;i < enqcmd.argnum; i++)
		printf("parse enqcmd:%s\n",arglist[i]);

#endif

	/*向等待队列中增加新的作业*/
	newnode = (struct waitqueue*)malloc(sizeof(struct waitqueue));
	newnode->next =NULL;
	newnode->job=newjob;

// 修改点@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
queueEnd(newnode);
/*	if(head)
	{
		for(p=head;p->next != NULL; p=p->next);
		p->next =newnode;
	}else
		head=newnode;
*/
	/*为作业创建进程*/
	if((pid=fork())<0)
		error_sys("enq fork failed");

	if(pid==0){
		newjob->pid =getpid();
		/*阻塞子进程,等等执行*/
		raise(SIGSTOP);
#ifdef DEBUG

		printf("begin running\n");
		for(i=0;arglist[i]!=NULL;i++)
			printf("arglist %s\n",arglist[i]);
#endif

		/*复制文件描述符到标准输出*/
		dup2(globalfd,1);
		/* 执行命令 */
		if(execv(arglist[0],arglist)<0)
			printf("exec failed\n");
		exit(1);
	}else{
		newjob->pid=pid;
		wait(NULL); // 让父进程等待子进程结束@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	}
}


void do_deq(struct jobcmd deqcmd)
{
	int deqid,i;
	struct waitqueue *p,*prev,*select,*selectprev,*rhead;
	deqid=atoi(deqcmd.data);

#ifdef DEBUG
	printf("deq jid %d\n",deqid);
#endif

	/* current jodid==deqid,终止当前作业 */
	if (current && current->job->jid ==deqid){
		printf("teminate current job\n");
		kill(current->job->pid,SIGKILL);
		for(i=0;(current->job->cmdarg)[i]!=NULL;i++){
			free((current->job->cmdarg)[i]);
			(current->job->cmdarg)[i]=NULL;
		}
		free(current->job->cmdarg);
		free(current->job);
		free(current);
		current=NULL;
printf("1111111111111111111111\n");  // @@@@@@@@@@@@@@@@@@@@@@########@@@@@@@@@@@@@@@@##########
	}

	else{ /* 或者在等待队列中查找deqid */
		select=NULL;
		selectprev=NULL;
printf("22222222222222222222\n");  // @@@@@@@@@@@@@@@@@@@@@@########@@@@@@@@@@@@@@@@##########
for(i=0;i<3;i++){
if(i==0)rhead=head3;
else if(i==1)rhead=head2;
else rhead=head1;
printf("333333333333333333333\n");  // @@@@@@@@@@@@@@@@@@@@@@########@@@@@@@@@@@@@@@@##########
		if(rhead){
			for(prev=rhead,p=rhead;p!=NULL;prev=p,p=p->next)
				if(p->job->jid==deqid){
					select=p;
					selectprev=prev;
					break;
				}
printf("4444444444444444444444444\n");  // @@@@@@@@@@@@@@@@@@@@@@########@@@@@@@@@@@@@@@@##########		
				selectprev->next=select->next;
				if(select==selectprev)
					rhead=rhead->next; // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
printf("5555555555555555555555555\n");  // @@@@@@@@@@@@@@@@@@@@@@########@@@@@@@@@@@@@@@@##########
if(i==0)head3=rhead;
else if(i==1)head2=rhead;
else head1=rhead;
		}
printf("666666666666666666666666\n");  // @@@@@@@@@@@@@@@@@@@@@@########@@@@@@@@@@@@@@@@##########
		if(select){
			for(i=0;(select->job->cmdarg)[i]!=NULL;i++){
				free((select->job->cmdarg)[i]);
				(select->job->cmdarg)[i]=NULL;
			}
printf("77777777777777777777777777\n");  // @@@@@@@@@@@@@@@@@@@@@@########@@@@@@@@@@@@@@@@##########
			free(select->job->cmdarg);
			free(select->job);
			free(select);
			select=NULL;
			}
	if(p!=NULL && p->job->jid==deqid) break;
printf("88888888888888888888888888\n");  // @@@@@@@@@@@@@@@@@@@@@@########@@@@@@@@@@@@@@@@##########
		}
	}
printf("9999999999999999999999999\n");  // @@@@@@@@@@@@@@@@@@@@@@########@@@@@@@@@@@@@@@@##########
}


void do_stat(struct jobcmd statcmd)
{
	struct waitqueue *p;
	char timebuf[BUFLEN];
/* 修改do_stat函数@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
char str[10];


	/*
	*打印所有作业的统计信息:
	*1.作业ID
	*2.进程ID
	*3.作业所有者
	*4.作业运行时间
	*5.作业等待时间
	*6.作业创建时间
	*7.作业状态
	*/
printf("current:\n"); // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	/* 打印信息头部 */
	printf("JOBID\tPID\tOWNER\tRUNTIME\tWAITIME\tCONTIME\tDEFPRI\tCURPRI\tCRETIME\t\tSTATE\n");
	if(current){
jobstate(current->job,str); // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		strcpy(timebuf,ctime(&(current->job->create_time)));
		timebuf[strlen(timebuf)-1]='\0';
		printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\n",
			current->job->jid,
			current->job->pid,
			current->job->ownerid,
			current->job->run_time,
			current->job->wait_time,
			current->job->constant_time,
			current->job->defpri,
			current->job->curpri,
			timebuf,str);
	}

/*
	for(p=head;p!=NULL;p=p->next){
jobstate(p->job,str);
		strcpy(timebuf,ctime(&(p->job->create_time)));
		timebuf[strlen(timebuf)-1]='\0';
		printf("%d\t%d\t%d\t%d\t%d\t%s\t%s\n",
			p->job->jid,
			p->job->pid,
			p->job->ownerid,
			p->job->run_time,
			p->job->wait_time,
			timebuf,
			str);
	}
*/

// 修改点@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
printf("waitting queue:\n");
printf("JOBID\tPID\tOWNER\tRUNTIME\tWAITIME\tCONTIME\tDEFPRI\tCURPRI\tCRETIME\t\tSTATE\n");
/*优先级为三的队列的ready状态的进程输出*/
	for(p=head3;p!=NULL;p=p->next){
		jobstate(p->job,str); // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		strcpy(timebuf,ctime(&(p->job->create_time)));
		timebuf[strlen(timebuf)-1]='\0';
		printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\n",
			p->job->jid,
			p->job->pid,
			p->job->ownerid,
			p->job->run_time,
			p->job->wait_time,
			p->job->constant_time,
			p->job->defpri,
			p->job->curpri,
			timebuf,
			str);
	}
/*优先级为二的队列的ready状态的进程输出*/
	for(p=head2;p!=NULL;p=p->next){
		jobstate(p->job,str); // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		strcpy(timebuf,ctime(&(p->job->create_time)));
		timebuf[strlen(timebuf)-1]='\0';
		printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\n",
			p->job->jid,
			p->job->pid,
			p->job->ownerid,
			p->job->run_time,
			p->job->wait_time,
			p->job->constant_time,
			p->job->defpri,
			p->job->curpri,
			timebuf,
			str);
	}
/*优先级为一的队列的ready状态的进程输出*/
	for(p=head1;p!=NULL;p=p->next){
		jobstate(p->job,str); // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		strcpy(timebuf,ctime(&(p->job->create_time)));
		timebuf[strlen(timebuf)-1]='\0';
		printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\n",
			p->job->jid,
			p->job->pid,
			p->job->ownerid,
			p->job->run_time,
			p->job->wait_time,
			p->job->constant_time,
			p->job->defpri,
			p->job->curpri,
			timebuf,
			str);
	}
}

int main()
{
	struct timeval interval;
	struct itimerval new,old;
	struct stat statbuf;
	struct sigaction newact,oldact1,oldact2;

/* 调试任务一 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
printf("debug is open!\n");
#endif

	if(stat("/tmp/server",&statbuf)==0){
		/* 如果FIFO文件存在,删掉 */
		if(remove("/tmp/server")<0)
			error_sys("remove failed");
	}

	if(mkfifo("/tmp/server",0666)<0)
		error_sys("mkfifo failed");
	/* 在非阻塞模式下打开FIFO,返回文件描述符 */
	if((fifo=open("/tmp/server",O_RDONLY|O_NONBLOCK))<0)
		error_sys("open fifo failed");

	/* 建立信号处理函数 */
	newact.sa_sigaction=sig_handler;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags=SA_SIGINFO;
	sigaction(SIGCHLD,&newact,&oldact1);
	sigaction(SIGVTALRM,&newact,&oldact2);

	/* 设置时间间隔为1000毫秒 */
	interval.tv_sec=1;
	interval.tv_usec=0;

	new.it_interval=interval;
	new.it_value=interval;
	setitimer(ITIMER_VIRTUAL,&new,&old);

	while(siginfo==1);

	close(fifo);
	close(globalfd);
	return 0;
}



/* 添加jobstate函数@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
void jobstate(struct jobinfo *job,char *str)
{
	if(job->state==RUNNING)
	{
		strcpy(str,"RUNNING");
		str[7]='\0';
	}	
	else if(job->state==READY)
	{
		strcpy(str,"READY");
		str[5]='\0';
	}
	else if(job->state==DONE)
	{
		strcpy(str,"DONE");
		str[4]='\0';
	}
	else 
	{
		str[0]='\0';
	}
}

// // 修改点@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// 把一个节点添加到相应的队列尾部
void queueEnd(struct waitqueue *newnode)
{
	struct waitqueue *p;
#ifdef DEBUG
if(newnode->next!=NULL)
printf("queueEnd error!!!!!!!!!!!!!!!\n");
#endif

	switch (newnode->job->curpri){
	case 1:
		if (head1){
		for (p = head1; p->next != NULL; p = p->next);
		p->next = newnode;
		}
		else
			head1 = newnode;
		return;
	case 2:
		if (head2){
		for (p = head2; p->next != NULL; p = p->next);
		p->next = newnode;
		}
		else
			head2 = newnode;
		return;
	case 3:
		if (head3){
		for (p = head3; p->next != NULL; p = p->next);
		p->next = newnode;
		}
		else
			head3 = newnode;
		return;
	}
}

// 把一个节点添加到相应的队列头部
void queueHead(struct waitqueue *newnode)
{
	struct waitqueue *p;

	switch (newnode->job->curpri){
	case 1:
		newnode->next=head1;
		head1=newnode;
		return;
	case 2:
		newnode->next=head2;
		head2=newnode;
		return;
	case 3:
		newnode->next=head3;
		head3=newnode;
		return;
	}
}

//  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int checkRunTime(struct waitqueue* p)
{
if(p->job->curpri == 1 && p->job->constant_time >= 5)
	return 1;
if(p->job->curpri == 2 && p->job->constant_time >= 2)
	return 1;
if(p->job->curpri == 3 && p->job->constant_time >= 1)
	return 1;
return 0;
}

