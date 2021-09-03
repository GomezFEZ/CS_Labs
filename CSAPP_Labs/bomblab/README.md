# BombLab

**简介**:通过disassemble可执行文件bomb并参考bomb.c推测出拆解炸弹所需要的密码,需要拆除六个阶段.

**工具**:为gdb,官网下载Lab的界面附带了一个<u>gdbnotes-x86-64</u>作为说明书.此外,bomblab.pdf中还提到了三个指令:

- objdump -t
- objdump -d
- strings

这些指令在终端而不是gdb中运行.

**重要提示:**

一些gdb指令有简写形式:

- stepi的简写:si
- 类似地,step的简写是s
- break的简写:b
- continue的简写:c
- quit的简写:q
- ...

**额外的说明**:

- bomb.c是可执行文件bomb的源码,但是它包含了两个没有给出的自定义头文件:

```C
#include "support.h"
#include "phases.h"
```

并调用了其中的函数,没有办法直接看源码来作弊. 所以bomb.c只具有参考作用.

- 此文件夹下没有其它文件,因为完成该Lab最后只会得到六行密码,并不产生其它内容

- 下面的内容全部独立完成,指过程中没有查看别人的答案



#### 1.phase_1

载入bomb之后给main打个断点再运行,不然会一直运行到输入密码的地方,没法查看之前的状态:

```
(gdb) break main
Breakpoint 1 at 0x400da0: file bomb.c, line 37.
(gdb) run
Starting program: /home/gaomez/桌面/BombLab/bomb/bomb 

Breakpoint 1, main (argc=1, argv=0x7fffffffe038) at bomb.c:37
37	{
```

可以看见停在了main的入口处,并且显示对应bomb.c的第37行(是一个左花括号).

这时候可以看一下main的汇编码:

```
(gdb) disas main
Dump of assembler code for function main:
=> 0x0000000000400da0 <+0>:	push   %rbx
   ...(无关紧要的东西)...
   0x0000000000400e19 <+121>:	callq  0x4013a2 <initialize_bomb>
   ......
   0x0000000000400e32 <+146>:	callq  0x40149e <read_line>
   0x0000000000400e37 <+151>:	mov    %rax,%rdi
   ......
   0x0000000000400e3a <+154>:	callq  0x400ee0 <phase_1>
   0x0000000000400e3f <+159>:	callq  0x4015c4 <phase_defused>
   0x0000000000400e44 <+164>:	mov    $0x4023a8,%edi
   0x0000000000400e49 <+169>:	callq  0x400b10 <puts@plt>
   0x0000000000400e4e <+174>:	callq  0x40149e <read_line>
   0x0000000000400e53 <+179>:	mov    %rax,%rdi
   0x0000000000400e56 <+182>:	callq  0x400efc <phase_2>
   0x0000000000400e5b <+187>:	callq  0x4015c4 <phase_defused>
   0x0000000000400e60 <+192>:	mov    $0x4022ed,%edi
   0x0000000000400e65 <+197>:	callq  0x400b10 <puts@plt>
   0x0000000000400e6a <+202>:	callq  0x40149e <read_line>
   0x0000000000400e6f <+207>:	mov    %rax,%rdi
   0x0000000000400e72 <+210>:	callq  0x400f43 <phase_3>
   0x0000000000400e77 <+215>:	callq  0x4015c4 <phase_defused>
   0x0000000000400e7c <+220>:	mov    $0x40230b,%edi
--Type <RET> for more, q to quit, c to continue without paging--

```

可以看到函数名与bomb.c是非常对应的.不过printf显示为"<puts@plt>".

下一个将要执行的函数是initialize_bomb,看一下汇编码:

```
(gdb) disas initialize_bomb
Dump of assembler code for function initialize_bomb:
   0x00000000004013a2 <+0>:	sub    $0x8,%rsp
   0x00000000004013a6 <+4>:	mov    $0x4012a0,%esi
   0x00000000004013ab <+9>:	mov    $0x2,%edi
   0x00000000004013b0 <+14>:	callq  0x400b90 <signal@plt>
   0x00000000004013b5 <+19>:	add    $0x8,%rsp
   0x00000000004013b9 <+23>:	retq   
End of assembler dump.
```

没有看出来是在做什么.

之后的read_line看名字就知道是用来读取输入的字符串了,并且将结果放到了寄存器%rdi.

尝试验证,在0x0000000000400e37设置断点,继续运行,随便输入个字符串("Hello"):

```
(gdb) break * 0x0000000000400e37
Breakpoint 3 at 0x400e37: file bomb.c, line 74.
(gdb) continue 
Continuing.
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Hello

Breakpoint 3, main (argc=<optimized out>, argv=<optimized out>) at bomb.c:74
74	    phase_1(input);                  /* Run the phase               */

```

然后查看%rdi,发现是个空串,而%rax正是"Hello",这意味着断点处的指令是尚未运行的.

```
(gdb) print (char *) $rdi
$1 = 0x603787 <input_strings+7> ""
(gdb) print (char *) $rax
$2 = 0x603780 <input_strings> "Hello"
```

意外地发现了函数<input_strings>,不过看起来不重要.

现在进phase_1看看:

```
(gdb) break phase_1
Breakpoint 4 at 0x400ee0
(gdb) until 4

Breakpoint 4, 0x0000000000400ee0 in phase_1 ()
(gdb) disas
Dump of assembler code for function phase_1:
=> 0x0000000000400ee0 <+0>:	sub    $0x8,%rsp
   0x0000000000400ee4 <+4>:	mov    $0x402400,%esi
   0x0000000000400ee9 <+9>:	callq  0x401338 <strings_not_equal>
   0x0000000000400eee <+14>:	test   %eax,%eax
   0x0000000000400ef0 <+16>:	je     0x400ef7 <phase_1+23>
   0x0000000000400ef2 <+18>:	callq  0x40143a <explode_bomb>
   0x0000000000400ef7 <+23>:	add    $0x8,%rsp
   0x0000000000400efb <+27>:	retq   
End of assembler dump.
(gdb) print (char *) $rdi
```

可以看出来,如果strings_not_equal的返回值%eax是0就可以拆解第一阶段的炸弹.否则就会调用explode_bomb,显然在这个函数上打个断点就可以防止引爆炸弹的时候扣分了,虽然我并不在CMU上学.

注意到调用strings_not_equal之前把地址0x402400放到了%esi中.显然是作为函数参数传递的,这个函数应该还有另一个参数,就是之前提到的%rdi,它的值现在正是"Hello".

```
(gdb) print (char *) $rdi
$3 = 0x603780 <input_strings> "Hello"
```

那%esi里的很明显就是第一阶段的密码了,直接看看这个地址:

```
(gdb) print (char *) 0x402400
$4 = 0x402400 "Border relations with Canada have never been better."
```

密码即

>  Border relations with Canada have never been better.



#### 2.phase_2

phase_2(input)的汇编码:

```
Dump of assembler code for function phase_2:
   0x0000000000400efc <+0>:	push   %rbp	# 被调用者保存寄存器
   0x0000000000400efd <+1>:	push   %rbx
   0x0000000000400efe <+2>:	sub    $0x28,%rsp
   0x0000000000400f02 <+6>:	mov    %rsp,%rsi	# %rsi是下面的函数的第二个参数
   0x0000000000400f05 <+9>:	callq  0x40145c <read_six_numbers>
...
End of assembler dump.
```

%rsi作为read_six_numbers第二个参数,被赋予了一个地址,指向phase_2的栈帧.

根据寄存器的使用顺序,显然read_six_numbers是有两个参数的,第一个参数%rdi在main中可以看出来:

```
Dump of assembler code for function main:
...
   0x0000000000400e4e <+174>:	callq  0x40149e <read_line>
   0x0000000000400e53 <+179>:	mov    %rax,%rdi
   0x0000000000400e56 <+182>:	callq  0x400efc <phase_2>
   0x0000000000400e5b <+187>:	callq  0x4015c4 <phase_defused>
...

```

%rdi首先是作为phase_2(input)的参数,在进入phase_2后直接又成为了read_six_numbers的第一个参数,它的值即为指向input的指针.(也就是输入的字符串)

现在查看read_six_numbers:

```
Dump of assembler code for function read_six_numbers:
   0x000000000040145c <+0>:	sub    $0x18,%rsp
   0x0000000000401460 <+4>:	mov    %rsi,%rdx
   0x0000000000401463 <+7>:	lea    0x4(%rsi),%rcx
   0x0000000000401467 <+11>:	lea    0x14(%rsi),%rax
   0x000000000040146b <+15>:	mov    %rax,0x8(%rsp)
   0x0000000000401470 <+20>:	lea    0x10(%rsi),%rax
   0x0000000000401474 <+24>:	mov    %rax,(%rsp)
   0x0000000000401478 <+28>:	lea    0xc(%rsi),%r9
   0x000000000040147c <+32>:	lea    0x8(%rsi),%r8
   0x0000000000401480 <+36>:	mov    $0x4025c3,%esi
   0x0000000000401485 <+41>:	mov    $0x0,%eax
   0x000000000040148a <+46>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x000000000040148f <+51>:	cmp    $0x5,%eax	# sscanf会返回读取成功的数量
   0x0000000000401492 <+54>:	jg     0x401499 <read_six_numbers+61>	# 如果发现成功的数量没有达到6就会爆炸
   0x0000000000401494 <+56>:	callq  0x40143a <explode_bomb>
   0x0000000000401499 <+61>:	add    $0x18,%rsp
   0x000000000040149d <+65>:	retq   
End of assembler dump.
```

只能说是非常复杂,但是注意到<+46>位置的sscanf函数,它的作用类似于scanf,不过多了一个 const char*参数,函数从这个字符串读取格式化输入.详见此[链接](https://www.runoob.com/cprogramming/c-function-sscanf.html).

sscanf的第一个参数是将要读取的字符串,是仍然没有变化的%rdi.(input)

第二个参数是读取的格式,在<+36>位置看见这个参数为$0x4025c3,打印出来:

```
(gdb) print (char *) 0x4025c3
$17 = 0x4025c3 "%d %d %d %d %d %d"
```

也就是说,这个函数就是单纯地从input里读取6个数字,意味着密码就是6个(用空格分开的)数字.

读取之后保存到6个地方,可以看出来从<+0>到<+41>的一系列复杂操作就是在把这些地方准备好,暂时不要仔细去分析,回到phase_2看看.

```
Dump of assembler code for function phase_2:
...
   0x0000000000400f05 <+9>:	callq  0x40145c <read_six_numbers>
   0x0000000000400f0a <+14>:	cmpl   $0x1,(%rsp)
   0x0000000000400f0e <+18>:	je     0x400f30 <phase_2+52>
   0x0000000000400f10 <+20>:	callq  0x40143a <explode_bomb>
   0x0000000000400f15 <+25>:	jmp    0x400f30 <phase_2+52>
   0x0000000000400f17 <+27>:	mov    -0x4(%rbx),%eax
   0x0000000000400f1a <+30>:	add    %eax,%eax
   0x0000000000400f1c <+32>:	cmp    %eax,(%rbx)
   0x0000000000400f1e <+34>:	je     0x400f25 <phase_2+41>
   0x0000000000400f20 <+36>:	callq  0x40143a <explode_bomb>
   0x0000000000400f25 <+41>:	add    $0x4,%rbx
   0x0000000000400f29 <+45>:	cmp    %rbp,%rbx
   0x0000000000400f2c <+48>:	jne    0x400f17 <phase_2+27>
   0x0000000000400f2e <+50>:	jmp    0x400f3c <phase_2+64>
   0x0000000000400f30 <+52>:	lea    0x4(%rsp),%rbx
   0x0000000000400f35 <+57>:	lea    0x18(%rsp),%rbp
   0x0000000000400f3a <+62>:	jmp    0x400f17 <phase_2+27>
...
End of assembler dump.

```

如果始终沿着避免引爆炸弹的分支进行跳转,不久就可以发现这段代码大部分相当于下面这段循环:

```
for(%rbx = %rsp + 4, %rbp = %rsp + 24; %rbp != %rbx; %rbx += 4){	// 已转为10进制
	%eax = *(%rbx - 4);
	%eax += %eax;
	if(%eax != *(%rbx)) explode_bomb();
}
```

据此可以分析出来,读取的6个数字存储到了%rsp到%rsp+20这段位置上,验证如下(输入了"1 2 3 4 5 6"):

```
(gdb) print *(int*) ($rsp)
$10 = 1
(gdb) print *(int*) ($rsp+4)
$11 = 2
(gdb) print *(int*) ($rsp+8)
$12 = 3
(gdb) print *(int*) ($rsp+12)
$13 = 4
(gdb) print *(int*) ($rsp+16)
$14 = 5
(gdb) print *(int*) ($rsp+20)
$15 = 6
```

只要顺利地通过这个循环就可以拆除炸弹,这要求每一个数字都是上一个数字的两倍,而根据<+14>可以看出第一个数字为1,于是有密码:

> 1 2 4 8 16 32



> 发现了一个彩蛋:
>
> Welcome to my fiendish little bomb. You have 6 phases with
> which to blow yourself up. Have a nice day!
> Phase 1 defused. How about the next one?
> That's number 2.  Keep going!
> ^CSo you think you can stop the bomb with ctrl-c, do you?
> ^CWell...OK. :-)



#### 3.phase_3

phase_3的汇编码:

```
(gdb) disas phase_3
Dump of assembler code for function phase_3:
   0x0000000000400f43 <+0>:	sub    $0x18,%rsp
   0x0000000000400f47 <+4>:	lea    0xc(%rsp),%rcx	# sscanf的第四个参数
   0x0000000000400f4c <+9>:	lea    0x8(%rsp),%rdx	# 第三个参数
   0x0000000000400f51 <+14>:	mov    $0x4025cf,%esi	# 第二个参数
   0x0000000000400f56 <+19>:	mov    $0x0,%eax
   0x0000000000400f5b <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000400f60 <+29>:	cmp    $0x1,%eax
   0x0000000000400f63 <+32>:	jg     0x400f6a <phase_3+39>
   0x0000000000400f65 <+34>:	callq  0x40143a <explode_bomb>
   0x0000000000400f6a <+39>:	cmpl   $0x7,0x8(%rsp)
   0x0000000000400f6f <+44>:	ja     0x400fad <phase_3+106>
   0x0000000000400f71 <+46>:	mov    0x8(%rsp),%eax
   0x0000000000400f75 <+50>:	jmpq   *0x402470(,%rax,8)
   0x0000000000400f7c <+57>:	mov    $0xcf,%eax
   0x0000000000400f81 <+62>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f83 <+64>:	mov    $0x2c3,%eax
   0x0000000000400f88 <+69>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f8a <+71>:	mov    $0x100,%eax
   0x0000000000400f8f <+76>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f91 <+78>:	mov    $0x185,%eax
   0x0000000000400f96 <+83>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f98 <+85>:	mov    $0xce,%eax
   0x0000000000400f9d <+90>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400f9f <+92>:	mov    $0x2aa,%eax
   0x0000000000400fa4 <+97>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fa6 <+99>:	mov    $0x147,%eax
   0x0000000000400fab <+104>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fad <+106>:	callq  0x40143a <explode_bomb>
   0x0000000000400fb2 <+111>:	mov    $0x0,%eax
   0x0000000000400fb7 <+116>:	jmp    0x400fbe <phase_3+123>
   0x0000000000400fb9 <+118>:	mov    $0x137,%eax
   0x0000000000400fbe <+123>:	cmp    0xc(%rsp),%eax
   0x0000000000400fc2 <+127>:	je     0x400fc9 <phase_3+134>
   0x0000000000400fc4 <+129>:	callq  0x40143a <explode_bomb>
   0x0000000000400fc9 <+134>:	add    $0x18,%rsp
   0x0000000000400fcd <+138>:	retq   
End of assembler dump.
```

可以看到有大量的重复片段出现,看起来像是一种switch的实现.

注意到<+24>处又是一个sscanf,看一下它的格式字符串:

```
(gdb) print (char *) 0x4025cf
$1 = 0x4025cf "%d %d"
```

这次的密码是两个数字,他们依次存储到了栈上的0x8(%rsp)和0xc(%rsp)位置.

读取完两个数字之后可以看到如果第一个数字超过了7就会直接爆炸,否则执行:

```
jmpq   *0x402470(,%rax,8)
```

这个0x402470是一个内存位置,但我不是很清楚这里面的含义,只知道尝试读取汇编码中这个地址时得到了没有什么意义的结果,我在这里卡了很久.

最后想起来加上个\*试试,发现指向了一个位置:(注意这时假设%rax,即输入的第一个数是0)

```
(gdb) x/w *0x402470
0x400f7c <phase_3+57>:	53176
```

之后跟着走下去,很容易得出第二个数应该是0xcf(输入207),所以密码之一就是:

> 0 207

这是在假设输入的第一个数是0的情况下得到的解,之后我验证了这一阶段是有多个解的,假设输入的第一个数是1,2,分别又可推出第二个数为311,707,均可拆除该阶段炸弹.

其它解:(部分)

> 1 311
>
> 2 707



#### 4.phase_4

phase_4()的反汇编码:

```
Dump of assembler code for function phase_4:
   0x000000000040100c <+0>:	sub    $0x18,%rsp
   0x0000000000401010 <+4>:	lea    0xc(%rsp),%rcx	# 密码第二个数
   0x0000000000401015 <+9>:	lea    0x8(%rsp),%rdx	# 密码第一个数
   0x000000000040101a <+14>:	mov    $0x4025cf,%esi
   0x000000000040101f <+19>:	mov    $0x0,%eax
   0x0000000000401024 <+24>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000401029 <+29>:	cmp    $0x2,%eax
   0x000000000040102c <+32>:	jne    0x401035 <phase_4+41>
   0x000000000040102e <+34>:	cmpl   $0xe,0x8(%rsp)
   0x0000000000401033 <+39>:	jbe    0x40103a <phase_4+46>
   0x0000000000401035 <+41>:	callq  0x40143a <explode_bomb>
   0x000000000040103a <+46>:	mov    $0xe,%edx
   0x000000000040103f <+51>:	mov    $0x0,%esi
   0x0000000000401044 <+56>:	mov    0x8(%rsp),%edi
   0x0000000000401048 <+60>:	callq  0x400fce <func4>
   0x000000000040104d <+65>:	test   %eax,%eax
   0x000000000040104f <+67>:	jne    0x401058 <phase_4+76>
   0x0000000000401051 <+69>:	cmpl   $0x0,0xc(%rsp)
   0x0000000000401056 <+74>:	je     0x40105d <phase_4+81>
   0x0000000000401058 <+76>:	callq  0x40143a <explode_bomb>
   0x000000000040105d <+81>:	add    $0x18,%rsp
   0x0000000000401061 <+85>:	retq   
End of assembler dump.
```

密码同样是两个数字,不再解释.

遵循避免引爆的原则,得出第一个数小于等于14,随后调用了func4(第一个数,0,14),从后续可以看出这个函数必须返回0.此外,在<+69>直接得出第二个数是0.

现在研究一下func4,发现里面有递归调用:

```
Dump of assembler code for function func4:
   0x0000000000400fce <+0>:	sub    $0x8,%rsp
   0x0000000000400fd2 <+4>:	mov    %edx,%eax
   0x0000000000400fd4 <+6>:	sub    %esi,%eax
   0x0000000000400fd6 <+8>:	mov    %eax,%ecx
   0x0000000000400fd8 <+10>:	shr    $0x1f,%ecx
   0x0000000000400fdb <+13>:	add    %ecx,%eax
   0x0000000000400fdd <+15>:	sar    %eax
   0x0000000000400fdf <+17>:	lea    (%rax,%rsi,1),%ecx
   0x0000000000400fe2 <+20>:	cmp    %edi,%ecx
   0x0000000000400fe4 <+22>:	jle    0x400ff2 <func4+36>
   0x0000000000400fe6 <+24>:	lea    -0x1(%rcx),%edx
   0x0000000000400fe9 <+27>:	callq  0x400fce <func4>
   0x0000000000400fee <+32>:	add    %eax,%eax
   0x0000000000400ff0 <+34>:	jmp    0x401007 <func4+57>
   0x0000000000400ff2 <+36>:	mov    $0x0,%eax
   0x0000000000400ff7 <+41>:	cmp    %edi,%ecx
   0x0000000000400ff9 <+43>:	jge    0x401007 <func4+57>
   0x0000000000400ffb <+45>:	lea    0x1(%rcx),%esi
   0x0000000000400ffe <+48>:	callq  0x400fce <func4>
   0x0000000000401003 <+53>:	lea    0x1(%rax,%rax,1),%eax
   0x0000000000401007 <+57>:	add    $0x8,%rsp
   0x000000000040100b <+61>:	retq   
End of assembler dump.
```

> 插曲:在<+15>发现sar反常地只有一个操作数,百度之后发现了一个从Stackoverflow爬取内容的网站,最后去SO看了[原问题](https://stackoverflow.com/questions/12813962/sar-command-in-x86-assembly-with-one-parameter?newreg=f85163b7f03445b5be565d24ed205d27).下面是解答:
>
> Looks like the dissembler used short-hand for SAR EAX,1 which has an opcode of 0xD1F8. when the immediate is not 1, aka SAR EAX,xx, the opcode is 0xC1F8 xx, see the Intel Instruction reference, Vol. 2B, 4-353.

为了避免把问题复杂化,我忽略递归调用的分支翻译下去:

```
func4(A, B, C){
	rax = C - B;
	rcx = (C - B) >> 31;	// 逻辑右移
	rax += rcx;
	rax >>= 1;	// 算术右移
	rcx = rax + B;
	if(rcx - A <= 0) goto <+36>
	else ...(递归)
	...
	<+36>:
	rax = 0;
	if(rcx - A >= 0) return rax;
	else ...(递归)
}
```

这样看不出什么内容,但是把phase_4里对func4的第一次调用的值放进去会发现:

```
// 用X表示密码第一个数
func4(X, 0, 14){
	rax = 14 - 0;
	rcx = (14 - 0) >> 31;
	rax += rax;
	rax >>= 1;
	rcx = rax + 0;
	// rcx == 7
	if(7 - X <= 0) goto <+36>
	else ...(递归)
	...
	<+36>:
	rax = 0;
	if(7 - X >= 0) return rax;
	else ...(递归)
}
```

可以得出,X = 7即可让func4返回0.

密码是

> 7 0



#### 5.phase_5

phase_5的汇编代码:

```
Dump of assembler code for function phase_5:
   0x0000000000401062 <+0>:	push   %rbx
   0x0000000000401063 <+1>:	sub    $0x20,%rsp
   0x0000000000401067 <+5>:	mov    %rdi,%rbx
   0x000000000040106a <+8>:	mov    %fs:0x28,%rax
   0x0000000000401073 <+17>:	mov    %rax,0x18(%rsp)
   0x0000000000401078 <+22>:	xor    %eax,%eax	# 异或自己,清零
   0x000000000040107a <+24>:	callq  0x40131b <string_length>
   0x000000000040107f <+29>:	cmp    $0x6,%eax	# 密码长6位
   0x0000000000401082 <+32>:	je     0x4010d2 <phase_5+112>
   0x0000000000401084 <+34>:	callq  0x40143a <explode_bomb>
   0x0000000000401089 <+39>:	jmp    0x4010d2 <phase_5+112>
 *********************************************************************
 核心部分:
 // %rbx的值是指向所输入字符串开头的地址,%rax已清零
   0x000000000040108b <+41>:	movzbl (%rbx,%rax,1),%ecx
   0x000000000040108f <+45>:	mov    %cl,(%rsp)
   0x0000000000401092 <+48>:	mov    (%rsp),%rdx
   0x0000000000401096 <+52>:	and    $0xf,%edx
   0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx
   0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1)
   0x00000000004010a4 <+66>:	add    $0x1,%rax
   0x00000000004010a8 <+70>:	cmp    $0x6,%rax
   0x00000000004010ac <+74>:	jne    0x40108b <phase_5+41>
 *********************************************************************
   0x00000000004010ae <+76>:	movb   $0x0,0x16(%rsp)
   0x00000000004010b3 <+81>:	mov    $0x40245e,%esi
   0x00000000004010b8 <+86>:	lea    0x10(%rsp),%rdi
   0x00000000004010bd <+91>:	callq  0x401338 <strings_not_equal>
   0x00000000004010c2 <+96>:	test   %eax,%eax
   0x00000000004010c4 <+98>:	je     0x4010d9 <phase_5+119>
   0x00000000004010c6 <+100>:	callq  0x40143a <explode_bomb>
   0x00000000004010cb <+105>:	nopl   0x0(%rax,%rax,1)
   0x00000000004010d0 <+110>:	jmp    0x4010d9 <phase_5+119>
   0x00000000004010d2 <+112>:	mov    $0x0,%eax
   0x00000000004010d7 <+117>:	jmp    0x40108b <phase_5+41>
   0x00000000004010d9 <+119>:	mov    0x18(%rsp),%rax
   0x00000000004010de <+124>:	xor    %fs:0x28,%rax
   0x00000000004010e7 <+133>:	je     0x4010ee <phase_5+140>
   0x00000000004010e9 <+135>:	callq  0x400b30 <__stack_chk_fail@plt>
   0x00000000004010ee <+140>:	add    $0x20,%rsp
   0x00000000004010f2 <+144>:	pop    %rbx
   0x00000000004010f3 <+145>:	retq   
End of assembler dump.
```

发现这里出现了栈保护机制的代码,不过没有影响,忽略.

先看核心部分,是一个循环,逐行解读:

```
	0x000000000040108b <+41>:	movzbl (%rbx,%rax,1),%ecx
```

这是数组的访问方式,假设输入字符串S,则将S[%rax]的值赋给%ecx.

```
   0x000000000040108f <+45>:	mov    %cl,(%rsp)	# %cl是%rcx的低位部分
   0x0000000000401092 <+48>:	mov    (%rsp),%rdx
   0x0000000000401096 <+52>:	and    $0xf,%edx	# 砍掉一半
```

把%rcx寄存器的最低的一个字节的值附给%rdx寄存器,之后砍掉一半,只留下最低的4位.

```
	0x0000000000401099 <+55>:	movzbl 0x4024b0(%rdx),%edx	# 零拓展拓回一字节
```

修改%edx的值为0x4024b0+%rdx位置的值.注意这里%rdx(修改前)的值的范围是0~15(10进制).

```
   0x00000000004010a0 <+62>:	mov    %dl,0x10(%rsp,%rax,1)
```

这一段单纯地就是把结果保存一下.

现在看一下0x4024b0处有什么:

```
(gdb) print (char *) 0x4024b0
$3 = 0x4024b0 <array> "maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"		# 后半段这句话在彩蛋里看见过
```

现在观察前面很乱的字符串,发现刚好是16个.

现在就可以理解了,输入的字符串会被逐字符分析,方法是保留字符ASCII码的二进制下低4位,得到一个在0~15之间的值,据此转化为"maduiersnfotvbyl"中的一个字符.

完成这些操作后,输入的字符串被转化为一个新的字符串,根据后面的语句:

```
0x00000000004010b3 <+81>:	mov    $0x40245e,%esi
```

以及

```
(gdb) x/s 0x40245e
0x40245e:	"flyers"
```

可知,需要使得转化后的字符串为"flyers",字符对应的位置是(转换为16进制)0x9,0xf,0xe,0x5,0x6,0x7.

这就是所输入字符串的6个字符的二进制低4位,但是还没有高4位.

只需要想个办法凑出来就行了,比如设置一个不影响低4位的偏移量:

```
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

int main() {
    int bias = 0x20;
    printf("%c%c%c%c%c%c", 0x9 + bias, 0xf + bias, 0xe + bias, 0x5 + bias,
           0x6 + bias, 0x7 + bias);

    return 0;
}
```

输出:

> )/.%&'

这就是一个答案了,如果觉得太奇怪可以把偏置量改成0x40:

> IONEFG

同样能过.



#### 6.phase_6

难度骤升,这次汇编码很长,我拆分为多个部分讲解:

```
Dump of assembler code for function phase_6:
第一部分:
   0x00000000004010f4 <+0>:	push   %r14
   0x00000000004010f6 <+2>:	push   %r13
   0x00000000004010f8 <+4>:	push   %r12
   0x00000000004010fa <+6>:	push   %rbp
   0x00000000004010fb <+7>:	push   %rbx
   0x00000000004010fc <+8>:	sub    $0x50,%rsp
   0x0000000000401100 <+12>:	mov    %rsp,%r13
   0x0000000000401103 <+15>:	mov    %rsp,%rsi
   0x0000000000401106 <+18>:	callq  0x40145c <read_six_numbers>
   0x000000000040110b <+23>:	mov    %rsp,%r14
   0x000000000040110e <+26>:	mov    $0x0,%r12d
   0x0000000000401114 <+32>:	mov    %r13,%rbp
   0x0000000000401117 <+35>:	mov    0x0(%r13),%eax
   0x000000000040111b <+39>:	sub    $0x1,%eax
   0x000000000040111e <+42>:	cmp    $0x5,%eax
   0x0000000000401121 <+45>:	jbe    0x401128 <phase_6+52>
   0x0000000000401123 <+47>:	callq  0x40143a <explode_bomb>
   0x0000000000401128 <+52>:	add    $0x1,%r12d
   0x000000000040112c <+56>:	cmp    $0x6,%r12d
   0x0000000000401130 <+60>:	je     0x401153 <phase_6+95>
   0x0000000000401132 <+62>:	mov    %r12d,%ebx
   0x0000000000401135 <+65>:	movslq %ebx,%rax
   0x0000000000401138 <+68>:	mov    (%rsp,%rax,4),%eax
   0x000000000040113b <+71>:	cmp    %eax,0x0(%rbp)
   0x000000000040113e <+74>:	jne    0x401145 <phase_6+81>
   0x0000000000401140 <+76>:	callq  0x40143a <explode_bomb>
   0x0000000000401145 <+81>:	add    $0x1,%ebx
   0x0000000000401148 <+84>:	cmp    $0x5,%ebx
   0x000000000040114b <+87>:	jle    0x401135 <phase_6+65>
   0x000000000040114d <+89>:	add    $0x4,%r13
   0x0000000000401151 <+93>:	jmp    0x401114 <phase_6+32>
   ---------------------------------------------------------
第二部分:
   0x0000000000401153 <+95>:	lea    0x18(%rsp),%rsi
   0x0000000000401158 <+100>:	mov    %r14,%rax
   0x000000000040115b <+103>:	mov    $0x7,%ecx
   0x0000000000401160 <+108>:	mov    %ecx,%edx
   0x0000000000401162 <+110>:	sub    (%rax),%edx
   0x0000000000401164 <+112>:	mov    %edx,(%rax)
   0x0000000000401166 <+114>:	add    $0x4,%rax
   0x000000000040116a <+118>:	cmp    %rsi,%rax
   0x000000000040116d <+121>:	jne    0x401160 <phase_6+108>
   ---------------------------------------------------------
第三部分:
   0x000000000040116f <+123>:	mov    $0x0,%esi
   0x0000000000401174 <+128>:	jmp    0x401197 <phase_6+163>
   0x0000000000401176 <+130>:	mov    0x8(%rdx),%rdx
   0x000000000040117a <+134>:	add    $0x1,%eax
   0x000000000040117d <+137>:	cmp    %ecx,%eax
   0x000000000040117f <+139>:	jne    0x401176 <phase_6+130>
   0x0000000000401181 <+141>:	jmp    0x401188 <phase_6+148>
   0x0000000000401183 <+143>:	mov    $0x6032d0,%edx
   0x0000000000401188 <+148>:	mov    %rdx,0x20(%rsp,%rsi,2)
   0x000000000040118d <+153>:	add    $0x4,%rsi
   0x0000000000401191 <+157>:	cmp    $0x18,%rsi
   0x0000000000401195 <+161>:	je     0x4011ab <phase_6+183>
   0x0000000000401197 <+163>:	mov    (%rsp,%rsi,1),%ecx
   0x000000000040119a <+166>:	cmp    $0x1,%ecx
   0x000000000040119d <+169>:	jle    0x401183 <phase_6+143>
   0x000000000040119f <+171>:	mov    $0x1,%eax
   0x00000000004011a4 <+176>:	mov    $0x6032d0,%edx
   0x00000000004011a9 <+181>:	jmp    0x401176 <phase_6+130>
   ---------------------------------------------------------
第四部分:
   0x00000000004011ab <+183>:	mov    0x20(%rsp),%rbx
   0x00000000004011b0 <+188>:	lea    0x28(%rsp),%rax
   0x00000000004011b5 <+193>:	lea    0x50(%rsp),%rsi
   0x00000000004011ba <+198>:	mov    %rbx,%rcx
   0x00000000004011bd <+201>:	mov    (%rax),%rdx
   0x00000000004011c0 <+204>:	mov    %rdx,0x8(%rcx)
   0x00000000004011c4 <+208>:	add    $0x8,%rax
   0x00000000004011c8 <+212>:	cmp    %rsi,%rax
   0x00000000004011cb <+215>:	je     0x4011d2 <phase_6+222>
   0x00000000004011cd <+217>:	mov    %rdx,%rcx
   0x00000000004011d0 <+220>:	jmp    0x4011bd <phase_6+201>
   0x00000000004011d2 <+222>:	movq   $0x0,0x8(%rdx)
   ----------------------------------------------------------
第五部分:
   0x00000000004011da <+230>:	mov    $0x5,%ebp
   0x00000000004011df <+235>:	mov    0x8(%rbx),%rax
   0x00000000004011e3 <+239>:	mov    (%rax),%eax
   0x00000000004011e5 <+241>:	cmp    %eax,(%rbx)
   0x00000000004011e7 <+243>:	jge    0x4011ee <phase_6+250>
   0x00000000004011e9 <+245>:	callq  0x40143a <explode_bomb>
   0x00000000004011ee <+250>:	mov    0x8(%rbx),%rbx
   0x00000000004011f2 <+254>:	sub    $0x1,%ebp
   0x00000000004011f5 <+257>:	jne    0x4011df <phase_6+235>
   0x00000000004011f7 <+259>:	add    $0x50,%rsp
   0x00000000004011fb <+263>:	pop    %rbx
   0x00000000004011fc <+264>:	pop    %rbp
   0x00000000004011fd <+265>:	pop    %r12
   0x00000000004011ff <+267>:	pop    %r13
   0x0000000000401201 <+269>:	pop    %r14
   0x0000000000401203 <+271>:	retq   
End of assembler dump.
```

这五个部分分别执行特定的功能,每个部分相对而言是独立的,没有从某部分jmp到另一个部分的情况.

**第一部分**

<+18>再次出现read_six_numbers,说明这次的密码又是6个数字.

之后的内容大约等同于下面的双重循环:

``` 
// 假设六个数字存储在int A[6]中
// A的地址为$rsp
for(int i = 0; i != 6; i++){
    unsigned x = A[i] - 1;	// 由<+45>使用jbe而不是jle推出是unsigned
    if(x <= 5)
    	for(int j = 0; j <= 5; j++){
    		int y = A[j];
    		if(x == y) explode_bomb();
    	}
}
```

从里面得出如下信息:

- 六个数字的取值范围是1~6(注意0u - 1是一个巨大的数).

- 六个数字必须两两不同.

这意味着密码是1~6的一个排列.

**第二部分**

这一部分做了一件很简单的事情,相当于:

```
for(int i = 0; i < 6; i++) A[i] = 7 - A[i];
```

只是起到了微小的干扰作用.

**第三部分**

这一段是在处理一个类似于链表的结构,但我并不知道到底是不是链表,由于他在汇编码中(以及栈帧中)是连续存储的,所以不如看作:

```
struct node{
	int val;
	int number;
	struct node* next;	// data[i].next = &data[i + 1];
	void* dummy;		// NULL
}data[6];				// 每个元素的大小都是16字节
```

这个结构体由下面的事实推出:

```
(gdb) x/50x 0x6032d0
# 这一行是注释	   	val			number		next		dummy
0x6032d0 <node1>:	0x0000014c	0x00000001	0x006032e0	0x00000000
0x6032e0 <node2>:	0x000000a8	0x00000002	0x006032f0	0x00000000
0x6032f0 <node3>:	0x0000039c	0x00000003	0x00603300	0x00000000
0x603300 <node4>:	0x000002b3	0x00000004	0x00603310	0x00000000
0x603310 <node5>:	0x000001dd	0x00000005	0x00603320	0x00000000
0x603320 <node6>:	0x000001bb	0x00000006	0x00000000	0x00000000
(其他的东西)...
```

这部分代码涉及到指针,必须搞清楚结构体的寻址方式才能看明白.

首先,下面是我没有经过验证的观点:

- data[6]的数值存储在0x6032d0~0x603320这段汇编代码中,每16字节存储了一个元素.

- 栈帧中%rsp+0x20~%rsp+0x48**将会**存储指向这六个元素的指针,每8字节存储一个.

现在就可以读懂这行代码的含义:

```
# 假设%rdx = 0x6032d0
	0x0000000000401176 <+130>:	mov    0x8(%rdx),%rdx
```

地址%rdx+0x8即0x6032d0,而这是data[0]的next元素的位置,因此引用0x8(%rdx)会得到一个新的地址,这个地址就是next的值,将它赋回%rdx就相当于链表检索到下一个节点.

现在直接给出这段代码的最终含义:

- 检查由第一,二部分得到的1~6序列(设为A[i]),**在%rsp+0x20+8i的位置存储指向data[A[i] - 1]的指针**

**第四部分**

这一部分出现了指向指针的指针,花了很久才看懂.

翻译过来(略有出入但效果相同):

```
rbx = *(rsp + 0x20);		# rsp + 0x20处的值是指针,指向了另一个指针0x6032d0,即rbx = 0x6032d0
rcx = rbx;
for(rax = rsp + 0x28, rsi = rsp + 0x50; rax != rsi; rax += 0x8){
	rdx = *rax;			# rdx = data[x]
	*(rcx + 8) = rdx;	# data[y].next = rdx;
	rcx = rdx;			# rcx = data[x];
}
*(rdx + 8) = 0;			# 将尾节点的next设为NULL
```

效果是,根据在栈帧中的存储顺序将这个链表连接起来.

**第五部分**

这一部分涉及到了前面没有提到的链表节点上的val值.

例如,对于rbx = 0x6932d0,*rbx就是data[0].val.

这个val是可以用指令(gdb) x/50x 0x6032d0直接查看的,见第三部分.

现在可以解读出这部分汇编代码的含义:

从头到尾检索这个在上一部分连接好的链表,如果发现有某个节点的值小于其之后的一个节点,那么引爆炸弹.

所以,根据val的值可以得出应该输入下面的序列:

```
3 4 5 6 1 2
```

再将第二部分造成的干扰抵消掉,得出真正的密码:

> 4 3 2 1 6 5



---

看了别人的攻略后发现自己漏掉了一个隐藏phase,惊了.

#### secret_phase

这一阶段的入口藏在phase_defused()函数里,正常解决前面6个阶段大概是不会去考虑这里面有东西的.

phase_defused()汇编码:

```
   0x00000000004015c4 <+0>:	sub    $0x78,%rsp
   0x00000000004015c8 <+4>:	mov    %fs:0x28,%rax
   0x00000000004015d1 <+13>:	mov    %rax,0x68(%rsp)
   0x00000000004015d6 <+18>:	xor    %eax,%eax
   0x00000000004015d8 <+20>:	cmpl   $0x6,0x202181(%rip)        # 	0x603760 <num_input_strings>
   0x00000000004015df <+27>:	jne    0x40163f <phase_defused+123>
   0x00000000004015e1 <+29>:	lea    0x10(%rsp),%r8
   0x00000000004015e6 <+34>:	lea    0xc(%rsp),%rcx
   0x00000000004015eb <+39>:	lea    0x8(%rsp),%rdx
   0x00000000004015f0 <+44>:	mov    $0x402619,%esi
   0x00000000004015f5 <+49>:	mov    $0x603870,%edi
   0x00000000004015fa <+54>:	callq  0x400bf0 <__isoc99_sscanf@plt>
   0x00000000004015ff <+59>:	cmp    $0x3,%eax
   0x0000000000401602 <+62>:	jne    0x401635 <phase_defused+113>
   0x0000000000401604 <+64>:	mov    $0x402622,%esi
   0x0000000000401609 <+69>:	lea    0x10(%rsp),%rdi
   0x000000000040160e <+74>:	callq  0x401338 <strings_not_equal>
   0x0000000000401613 <+79>:	test   %eax,%eax
   0x0000000000401615 <+81>:	jne    0x401635 <phase_defused+113>
   0x0000000000401617 <+83>:	mov    $0x4024f8,%edi
   0x000000000040161c <+88>:	callq  0x400b10 <puts@plt>
   0x0000000000401621 <+93>:	mov    $0x402520,%edi
   0x0000000000401626 <+98>:	callq  0x400b10 <puts@plt>
   0x000000000040162b <+103>:	mov    $0x0,%eax
   0x0000000000401630 <+108>:	callq  0x401242 <secret_phase>	# 藏在这里
   0x0000000000401635 <+113>:	mov    $0x402558,%edi
   0x000000000040163a <+118>:	callq  0x400b10 <puts@plt>
   0x000000000040163f <+123>:	mov    0x68(%rsp),%rax
   0x0000000000401644 <+128>:	xor    %fs:0x28,%rax
   0x000000000040164d <+137>:	je     0x401654 <phase_defused+144>
   0x000000000040164f <+139>:	callq  0x400b30 <__stack_chk_fail@plt>
   0x0000000000401654 <+144>:	add    $0x78,%rsp
   0x0000000000401658 <+148>:	retq   
End of assembler dump.

```

在<+20>行看到一个奇怪的东西,注释标识为num_input_strings,看起来是个函数,但是disas 0x603760会得出几行奇怪的东西,而且运行过程中会不断变化,比如:

```
Dump of assembler code for function num_input_strings:
   0x0000000000603760 <+0>:	(bad)  
   0x0000000000603761 <+1>:	add    %al,(%rax)
   0x0000000000603763 <+3>:	add    %al,(%rax)
End of assembler dump.
```

实际上num_input_strings更像是一个变量:

```
# 拆除六个阶段后
(gdb) x/w 0x603760
0x603760 <num_input_strings>:	6
```

这个数值"6"表示已经拆除的阶段数,推测这是一个全局变量.

最终得出进入secret_phase条件是:

- 拆除前六个阶段

- phase_4额外输入一个字符串"DrEvil"

现在看看secret_phase:

```
Dump of assembler code for function secret_phase:
   0x0000000000401242 <+0>:	push   %rbx
   0x0000000000401243 <+1>:	callq  0x40149e <read_line>
   0x0000000000401248 <+6>:	mov    $0xa,%edx
   0x000000000040124d <+11>:	mov    $0x0,%esi
   0x0000000000401252 <+16>:	mov    %rax,%rdi
   0x0000000000401255 <+19>:	callq  0x400bd0 <strtol@plt>	# string to long,把输入的字符串里的数字拿出来而已
   0x000000000040125a <+24>:	mov    %rax,%rbx
   0x000000000040125d <+27>:	lea    -0x1(%rax),%eax
   0x0000000000401260 <+30>:	cmp    $0x3e8,%eax
   0x0000000000401265 <+35>:	jbe    0x40126c <secret_phase+42>
   0x0000000000401267 <+37>:	callq  0x40143a <explode_bomb>
   0x000000000040126c <+42>:	mov    %ebx,%esi
   0x000000000040126e <+44>:	mov    $0x6030f0,%edi		# 一个地址
   0x0000000000401273 <+49>:	callq  0x401204 <fun7>
   0x0000000000401278 <+54>:	cmp    $0x2,%eax
   0x000000000040127b <+57>:	je     0x401282 <secret_phase+64>
   0x000000000040127d <+59>:	callq  0x40143a <explode_bomb>
   0x0000000000401282 <+64>:	mov    $0x402438,%edi
   0x0000000000401287 <+69>:	callq  0x400b10 <puts@plt>
   0x000000000040128c <+74>:	callq  0x4015c4 <phase_defused>
   0x0000000000401291 <+79>:	pop    %rbx
   0x0000000000401292 <+80>:	retq   
End of assembler dump.
```

会发现一个地址0x6030f0作为第一个参数,输入的一个数字作为第二个参数,传给了fun7.

研究一下这个地址:

```
(gdb) x/80g 0x6030f0
0x6030f0 <n1>:	0x0000000000000024	0x0000000000603110
0x603100 <n1+16>:	0x0000000000603130	0x0000000000000000
0x603110 <n21>:	0x0000000000000008	0x0000000000603190
0x603120 <n21+16>:	0x0000000000603150	0x0000000000000000
0x603130 <n22>:	0x0000000000000032	0x0000000000603170
0x603140 <n22+16>:	0x00000000006031b0	0x0000000000000000
0x603150 <n32>:	0x0000000000000016	0x0000000000603270
0x603160 <n32+16>:	0x0000000000603230	0x0000000000000000
0x603170 <n33>:	0x000000000000002d	0x00000000006031d0
0x603180 <n33+16>:	0x0000000000603290	0x0000000000000000
0x603190 <n31>:	0x0000000000000006	0x00000000006031f0
0x6031a0 <n31+16>:	0x0000000000603250	0x0000000000000000
0x6031b0 <n34>:	0x000000000000006b	0x0000000000603210
0x6031c0 <n34+16>:	0x00000000006032b0	0x0000000000000000
0x6031d0 <n45>:	0x0000000000000028	0x0000000000000000
0x6031e0 <n45+16>:	0x0000000000000000	0x0000000000000000
0x6031f0 <n41>:	0x0000000000000001	0x0000000000000000
0x603200 <n41+16>:	0x0000000000000000	0x0000000000000000
0x603210 <n47>:	0x0000000000000063	0x0000000000000000
0x603220 <n47+16>:	0x0000000000000000	0x0000000000000000
0x603230 <n44>:	0x0000000000000023	0x0000000000000000
0x603240 <n44+16>:	0x0000000000000000	0x0000000000000000
0x603250 <n42>:	0x0000000000000007	0x0000000000000000
0x603260 <n42+16>:	0x0000000000000000	0x0000000000000000
0x603270 <n43>:	0x0000000000000014	0x0000000000000000
0x603280 <n43+16>:	0x0000000000000000	0x0000000000000000
0x603290 <n46>:	0x000000000000002f	0x0000000000000000
0x6032a0 <n46+16>:	0x0000000000000000	0x0000000000000000
0x6032b0 <n48>:	0x00000000000003e9	0x0000000000000000
0x6032c0 <n48+16>:	0x0000000000000000	0x0000000000000000
(其它东西)...
```

发现这是个结构体:

```
struct Tree{
	int val;
	struct Tree* leftChild;
	struct Tree* rightChild;
	// 一段空白,为了凑齐16字节
}
```

之后推出这是一个Binary Search Tree,而fun7所做的事情是在这棵树中查找输入的数字,并在此过程值对一个累积值进行一系列操作.具体见[此回答](Introduction to CSAPP（十九）：这可能是你能找到的分析最全的Bomblab了 - Yannick的文章 - 知乎 https://zhuanlan.zhihu.com/p/104130161).

推断可得secret_phase的密码:

> 20

或者

> 22

