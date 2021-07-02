# DataLab

**简介**:运用位操作,二进制补码算数和浮点数操作完成13个空的函数体.

除howManyBits,float_twice,float_i2f外均独立完成.



注:以下顺序根据datalab.pdf,与bits.c中的有所区别,并且删除了我在函数体中的注释.

## 1.Bit Manipulations

**1.bitXor**

x ˆ y using only & and ~

```c
int bitXor(int x, int y) {
    int v1 = ~(x & y);
    int v2 = ~((~x) & (~y));
    return v1 & v2;
}
```

运用德摩根律,但实际上观察一下1100^1010就可以凑出来这种操作.



**2.allOddBits**

Are all odd-numbered bits set to 1?

```C
int allOddBits(int x) {
    int odd = 0xAA + (0xAA << 8) + (0xAA << 16) + (0xAA << 24);
    int nx = x & odd;
    int ret = (nx | (nx >> 1));
    return !(~ret);
}
```

使用掩码1010 1010 ... 1010将x的偶数位清零.此时,只有当x的奇数位全为1时,才会有x | (x >> 1) == 1111 1111 ... 1111.



**3.isAsciiDigit**

Is x an ASCII digit?(0x30~0x39)

```C
int isAsciiDigit(int x) {
    int chkf = !((0x3) ^ (x >> 4)); // check front 0000... 0011
    int back = x & 0xF;		    // the rest 4 bits, should be within 0000~1001
    int is89 = (!(back ^ 8)) | (!(back ^ 9));
    int b4 = (x & 8);
    return chkf & (is89 | (!b4));
}
```

0x30即0011 0000,0x39即0011 1001.

首先检查x的高28位是否为0x0000003,然后检查低4位.

对于0x30~0x37,低4位覆盖了如下模式:0x**0**???.也就是说只要加粗位是0,就是在范围内的数.

此外,对0x38和0x39进行特殊判断即可(is89).

*上述操作可以满足在规定操作数内完成任务,但实际上这是一种暴力求解,网上有更好的思路.*



**4.conditional**

Same as C’s “x ? y : z”

```C
int conditional(int x, int y, int z) {
    int v1 = !!x;
    int v2 = !x;
    int msk1 = (v1 << 31) >> 31;
    int msk2 = (v2 << 31) >> 31;
    return ((y & msk1) | (z & msk2)); 
}
```

使用!!x获取布尔值0/1,对0/1左移31位再右移31位,由于算数位移的原因得到了0x0000 0000和0xFFFF FFFF.分别用他们对y和z进行掩码即可.



**5.logicalNeg**

Compute !x without using ! operator

先放一个[知乎上的答案:](https://zhuanlan.zhihu.com/p/59534845)

```C
return ((x|(~x+1))>>31)+1;
```

x和-x的符号位的位或,除非x是0,一定是1.

这是我的繁琐方法:

```C
int logicalNeg(int x) { 
    int val = x | (x << 16);
    val = val | (val << 8);
    val = val | (val << 4);
    val = val | (val << 2);
    val = val | (val << 1);
    return (val >> 31) + 1;
}
```

!x为真当且仅当x位级表示为0000 0000 .... 0000.

把x的低16位与高16位进行位或,结果存为16位的v1,

把v1的低8位与高8位进行位或,结果存为8位的v2,

...

把v4的低1位与高1位进行位或,结果存为1位的v5.

return v5.

为了避免算数位移,描述的细节与代码实现稍有区别.



## 2.Two's Complement Arithmetic

**1.tmin**

Return smallest two’s complement integer

```C
int tmin(void) {
    return 1 << 31;
}
```

环境中的int是32位的.



**2.isTmax**

Is x the largest 32-bit two’s complement intger?

```C
int isTmax(int x) {
    int nx = x + 1;
    int chk1 = !!nx;	// nx is not 0?
    return (!(nx ^ (~nx + 1))) & chk1;
}
```

有且仅有两个数具有这样的特殊性质:x=-x,即x=~x+1.他们是TMin和0.

TMax+1 -> Tmin,所以问题变成判断TMax+1的值是不是TMin.

利用上述性质判断是否为TMin,且排除掉为0的情况即可.

用到了异或的性质,x ^ y == 0当且仅当x == y.



**3.negate**

-x without negation(这里negation指负号)

```C
int negate(int x) {
    return ~x + 1;
}
```

结论,-x = ~x + 1对所有32位补码数成立.



**4.isLessOrEqual**

x <= y?

```C
int isLessOrEqual(int x, int y) {
    int sx = !!(x >> 31);
    int sy = !!(y >> 31);
    int sum = x + ~y + 1;
    int ssu = !!(sum >> 31);
    return ((sx & !sy) | (!(sx ^ sy) & ssu) | !(x ^ y));
}
```

注意到,允许使用加法运算.

x <= y ≡ x + (-y) <= 0.其中-y由~y + 1得到.

所以,检查值sum = x + ~ y + 1的符号位就可以判断x <= y?(需要特判等号情况)

但是,直接这样做会发生越界导致错误结果.所以要根据x和y的符号预先处理一下:

sx --- x的符号位,sy --- y的符号位

1. sx = 1, sy = 0, x < y.
2. sx = 0, sy = 1, x > y.
3. sx == sy, 此时x + (-y)一定不会越界.采用上述方法进行判断即可.



**5.howManyBits**

Compute minimum number of bits required to represent x

```C
int howManyBits(int x) {
    int sign = (x >> 31);
    int v = (x & ~sign) | ((~x) & sign);
    int b16 = !!(v >> 16) << 4;
    int v1 = v >> b16;
    int b8 = !!(v1 >> 8) << 3;
    int v2 = v1 >> b8;
    int b4 = !!(v2 >> 4) << 2;
    int v3 = v2 >> b4;
    int b2 = !!(v3 >> 2) << 1;
    int v4 = v3 >> b2;
    int b1 = !!(v4 >> 1);
    int v5 = v4 >> b1;
    int b0 = !!v5;
    return b16 + b8 + b4 + b2 + b1 + b0 + 1;
}
```

没想出来,看了网上的答案.

原理是,一个数字在二进制下具有唯一的表示.

操作数限制是90,实际上远远没有达到.



## 3.  Floating-Point Operations

**1.float_twice**

Compute 2.0*f

```C
unsigned float_twice(unsigned uf) {
    int exp = (uf & 0x7F800000) >> 23;
    int sign = uf & (1 << 31);
    if(exp == 0) return sign | (uf << 1);
    if(exp == 255) return uf;
    exp++;
    if(exp == 255) return sign | 0x7F800000;
    return (exp << 23) | (0x807FFFFF & uf);
}
```

浮点数的存储分为多种情况,需要分类讨论.参考了他人答案.

- exp=255 说明是NaN或者inf,return参数即可

- exp=0 非规格化值

  ​	这时的尾数M=frac,需要将尾数乘以2,即左移一位.

  ​	<u>左移后,可能会有frac越过了23位,让exp变成0x01,变成一个规格化值,发现这个规格化值刚好是所需要的正确的值.</u>故有return sign | (uf << 1).

- exp∈(0,255) 规格化值

  ​	只需要使exp加1即可,注意若exp加1后达到了255,变为inf,根据sign返回±inf.



**2.float_f2i(uf)**

Compute(int) f (float转int)

```C
unsigned float_i2f(int x) {
    int sign = x & (1 << 31);
    int exp = 0;
    int frac = 0;
    int E;
    int round;

    if(x == 0) return x;
    if(x == 0x80000000) exp = 158;
    else{
		if(sign) x = -x;
        E = 0;
            while(x >> E) E++;
            E--;

        exp = E + 127;
        x = x << (31 - E);	// Not 32, to avoid arithmetic right shift
        frac = 0x7FFFFF & (x >> 8);	// 5 'F's
            // Now check round up?
        round = ((x & 0xFF) > 128) || (((x & 0xFF) == 128) && (frac & 1));
        frac += round;
            if(frac >> 23){
                exp++;
            frac = frac & 0x7FFFFF;
        }
    }
    return sign | (exp << 23) | frac;
}
```

本Lab最难题,参考了[他人答案](https://zhuanlan.zhihu.com/p/38753345).

- 理一理思路:

  - int转float,向偶数舍入.

  - 浮点数的计算式:V = (-1)^s x M x 2 ^E

  - 先判断边界情况后把符号s拿掉简化处理

  - 拿掉符号后有x≥0,只有0是非规格化值,其余均为规格化值
  - int转化为规格化的float,只需要计算出exp和frac,再加上符号位就可以了

- 实现:

  - 特判没法拿掉符号的值TMin,作为一个规格化值,它的exp=E+Bias=31+127=158

  - 特判拿掉符号后唯一的非规格化值0,return 0即可

  - 拿掉符号:if(sign) x = -x; 经过预处理后不会发生溢出

  - 计算阶码E,即最高位的位置减去1,因为M=1.f1 f2 f3 ...

  - 计算frac,即从x的最高位开始往后的23位,不足补0(注意这里对x的操作)

  - 判断舍入,若x的低8位大于1000 0000(二进制中的"一半",十进制的128),进1,或者,等于1000 0000且当前的frac是奇数,进1

  - 舍入后,若frac越过了23位,exp加一,frac直接截断至23位即可

    由于int本身的范围,不可能变为无穷大(E<31)

    直接截断frac看似很奇怪,结合规格化值的M=1+frac就能理解其正确性了.



**3.float_i2f**

Compute (float)x (float转int)

```C
int float_f2i(unsigned uf) {
    int sign = uf & (1 << 31);
    int exp = (uf & 0x7F800000) >> 23;
    int frac = uf & 0x7FFFFF;
    int E = exp - 127;
    int M = frac + (1 << 23);		// M=1+f
    int tmp = M << 7;
    int ret;

    if(exp == 255 || E >= 31) return 0x80000000u;
    if(E < 0) return 0;
    ret = tmp >> (30 - E);
    if(sign) ret = ~ret + 1;
    return ret;
}
```

比上一题简单得多,因为int的范围相比于float小得多,只需要应对一小部分情况就可以了.

首先,浮点数转化为int是直接舍弃小数部分,只保留整数部分的.

- exp=0 非规格化值,有E=1-Bias<0,意味着整数部分是0,返回0
- exp=255 NaN和inf,据题意返回0x80000000u
- 规格化值,E=exp-Bias
  - E>=31 溢出了int的范围,返回0x80000000u
  - E<0 整数部分又是0,返回0
  - 0≤E＜31,只需要把frac拿出来,根据E把小数点移动到正确的位置上,截取小数点前面的部分返回即可

