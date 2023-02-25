# ShellLab

**简介**:基于已有的框架编写一个tiny shell程序,需要用到很多进程,信号相关的知识和技巧.

**代码见shlab-handout/tsh.c**

### 1.前台与后台作业

**区别**:

前台作业启动后需要shell调用waitfg等待它完成,后台作业不需要,只是给出一条提示消息然后就可以输出prompt等待下一条指令了.

```
        if (!bg) {
            // foreground
            waitfg(pid);
        } else {
            // background
            printf("[%d] (%d) %s", getjobpid(jobs, pid)->jid, pid, cmdline);
        }
```

**前后台切换**:

给出的资料没有提示fg,bg这两个指令具体的操作,用神器tldr查了一下:

`tldr fg`,`tldr bg`.得知fg就是把一个**后台运行的或挂起**的作业切换到前台运行,bg会将**挂起的**(比如前台作业运行的时候按下Ctrl+Z)作业作为后台作业运行(重新启动了).

所以do_bgfg中需要实现:

- fg-重新启动指定作业,并且调用waitfg(于是就成了前台作业)
- bg-重新启动指定作业,但是只给出一条提示消息,shell继续等待指令(于是就成了后台作业)

**waitfg()**

shell需要在前台进程结束前保持阻塞,暴力的实现方法是不断的检查当前前台进程是否为waitfg的参数,如果不是就一直循环.

### 2.修改全局数据结构

tsh.c用`job_t jobs[MAXJOBS]`全局变量存储作业,我们时常会去修改它,例如bg,fg指令改变作业的状态.

每当修改一个全局数据结构(这里是jobs)时,必须阻塞所有信号,之后再恢复.这是为了防止产生竞争.例如很明显不希望当eval中的addjob()还没有返回就出现了一个SIGCHLD信号,后者的信号处理函数将调用delelejob(),这种情况下可能会产生破坏性的后果.

```C
        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        addjob(jobs, pid, (bg ? BG : FG), cmdline);
        sigprocmask(SIG_SETMASK, &prev, NULL);  // parent unblock SIGCHILD
```

```C
    sigset_t mask, prev;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, &prev);
    // 2.fg or bg?
    if (!strcmp(argv[0], "fg"))
        j->state = FG;
    else
        j->state = BG;
    sigprocmask(SIG_SETMASK, &prev, NULL);
```

### 3.execve覆盖信号处理程序

- 一个进程调用fork产生子进程,这个子进程具有和父进程同样的信号处理程序
- 一个进程调用execve运行新程序,这个**新程序覆盖了该进程的所有空间,包括信号处理程序,因此安装的信号处理程序失效,信号行为变为默认**

不过在这个lab中并不会造成什么影响,因为我们的shell程序只为自己安装了SIGINT(ctrl-c),SIGSTP(ctrl-z)和SIGCHLD的处理器,由shell创建的新进程在经过上面两步后失去了这些信号处理器而变为默认行为,这正是我们需要的.

### 4.信号发送给谁?

shell进程调用外部程序时(例如myspin)产生一个子进程来执行它,这时候按Ctrl+C(或Z)会把信号发送给谁呢?

答案是父进程和它的所有子进程(同一进程组)都会收到信号,见[这个提问](https://stackoverflow.com/questions/31907212/will-ctrlc-send-sigint-signals-to-both-parent-and-child-processes-in-linux).(这显然是一个因为shlab出现的提问)

但这并不是我们希望的效果,根据shlab.pdf提示(The tsh Specification),应该把SIGINT(或SIGTSTP)仅仅被发送到**前台程序和它的子进程**.如何实现呢?见下面的一系列步骤:

1. 每当**shell**(而不会是其它进程,请仔细观察eval函数中的流程控制)创建子进程(fork)时,子进程调用函数`setpgid(0, 0);`来把自己**放到新的进程组**.(这个方法也是上面的提问链接中出现的)
2. 信号处理程序如下:

```C
// kill(pid_t pid, int sig)-如果pid<0,那么发送信号sig给进程组|pid|(绝对值)中的每个进程
void sigint_handler(int sig) {
    int old_errno = errno;
    pid_t pid = fgpid(jobs);	// fgpid-返回前台进程的pid
    if (pid) kill(-pid, sig);
    errno = old_errno;
}

void sigtstp_handler(int sig) {
    int old_errno = errno;
    pid_t pid = fgpid(jobs);
    if (pid) kill(-pid, sig);
    errno = old_errno;
}
```

"前台程序和它的子进程",也就是前台程序所在的进程组,用kill()可以方便地实现这一点.

再次强调,eval中的setpgid函数仅仅是把shell进程和它的所有子进程放到不同的进程组,但是那些子进程自己创建(这可能会在execve后发生)的进程和他们是在同一个进程组中的.



现在就可以总结出实现这个效果的流程了:

- 键入Ctrl+C或Ctrl+Z,SIGINT或SIGTSTP仅被发送到shell所在的进程组,这个进程组只有shell进程
- shell进程跳转到上面所编写的信号处理程序
- 信号处理程序把信号发送给前台进程所在进程组中的每个进程,也就是前台进程及其所有子进程

书上似乎没有提到信号与进程组的行为,搜索了一下:

- 信号会被发送到父进程所在的进程组中的所有进程
- 父进程和子进程默认处于同一个进程组,允许更改子进程的进程组,这并不会改变父子进程关系

### 5.回收子进程

每当一个子进程**终止或停止**时,会将SIGCHLD信号发送给其父进程.父进程跳转到信号处理程序sigchld_handler.

利用这个信号处理程序来更新全局变量jobs.注意对于终止和停止两种情况要分别对待:终止需要删除这个job,停止则只是改变job的状态.

```C
void sigchld_handler(int sig) {
    int old_errno = errno;
    sigset_t mask_all, prev_all;
    sigfillset(&mask_all);
    pid_t pid;
    int status;

    // WNOHANG | WUNTRACED意味着立即返回,如果等待集合中无子进程终止返回0,否则返回终止的子进程的PID
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);	// 修改全局变量前需要阻塞所有信号
        // 下面根据status状态分别处理
        if (WIFSTOPPED(status)) {	// 子进程停止
            struct job_t *j = getjobpid(jobs, pid);
            j->state = ST;
            printf("Job [%d] (%d) stopped by signal %d\n", j->jid, pid,
                   WSTOPSIG(status));
        } else {
            deletejob(jobs, pid);		// 子进程正常终止
            if (WIFSIGNALED(status))	// 子进程因为一个未被信号处理器捕获(也就是执行默认行为)的信号终止,按Ctrl+C就会触发这种情况
                printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(pid),
                       pid, WTERMSIG(status));
        }
        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }
    errno = old_errno;
}
```
