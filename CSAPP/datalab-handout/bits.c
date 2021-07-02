/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * Gaomez
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
    /* Just stare at the case : 1100 ^ 1010, and I happen to find that the following 
     * steps satisfy the Xor operation. */
    int v1 = ~(x & y);
    int v2 = ~((~x) & (~y));
    return v1 & v2;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
    /* In the lab the integer is 32-bit, so Tmin is 1000 0000 0000 0000 0000
     * 0000 0000 0000, ouviously 1 << 31 */
    return 1 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 2
 */
int isTmax(int x) {
    /* For two special values x = 0 and x = Tmin, we have x = ~x + 1. */
    int nx = x + 1;
    int chk1 = !!nx;	// nx is not 0?
    return (!(nx ^ (~nx + 1))) & chk1;
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
    /* Construct the 1010 1010...1010 as "odd", use it to hide even bits in x,
     * and then only allOddBits numbers satisfy x  | (x >> 1) == 1111..11 */
    int odd = 0xAA + (0xAA << 8) + (0xAA << 16) + (0xAA << 24);
    int nx = x & odd;
    int ret = (nx | (nx >> 1));
    return !(~ret);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
    /* -x equals to ~x + 1, for all integers, inclding Tmin. */
    return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
    /* Check if the front 28 bits are 000...0011, and then check the last 4 bits using the following rules:
     * 1.The 4 bits represents 8 or 9.
     * 2.The 4 bits is 0xxx, which is smaller than 8.
     * If 1 or 2 is satisfies, then the last 4 bits are OK. */
    int chkf = !((0x3) ^ (x >> 4)); // check front 0000... 0011
    int back = x & 0xF;		    // the rest 4 bits, should be within 0000~1001
    int is89 = (!(back ^ 8)) | (!(back ^ 9));
    int b4 = (x & 8);
    return chkf & (is89 | (!b4));
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
/* Use !x and !!x to get 0/1, creat two masks:00...0 and 11...0,
 * apply the two masks to y and z and return them.
 * */
    int v1 = !!x;
    int v2 = !x;
    int msk1 = (v1 << 31) >> 31;
    int msk2 = (v2 << 31) >> 31;
    return ((y & msk1) | (z & msk2)); 
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
/* Check signs of x and y, if:
 * 1.sx-sign of x is 1, sy is 0, then x < y.
 * 2.sx == sy == 1, check the sign of x - y, which equals to x + ~y + 1.
 * 3.sx == sy == 0, do the same things as 2.
 * 4.Otherwise sx == 0, sy == 1, x > y.
 * Take signs into consideration to deal with the overflow trouble.
 * */
    int sx = !!(x >> 31);
    int sy = !!(y >> 31);
    int sum = x + ~y + 1;
    int ssu = !!(sum >> 31);
    return ((sx & !sy) | (!(sx ^ sy) & ssu) | !(x ^ y));
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) { 
/* Use binary search to apply bitOr to all bits of x with 5 steps.
 * Use left shift but not right to avoid sign bit trouble.
 * */
    int val = x | (x << 16);
    val = val | (val << 8);
    val = val | (val << 4);
    val = val | (val << 2);
    val = val | (val << 1);
    return (val >> 31) + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
/*A number can be represented in two's complement in only one way.
 * Set theses variables so that dlc won't complain.
 * */
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
//float
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
/*Divide and conquer.*/
    int exp = (uf & 0x7F800000) >> 23;
    int sign = uf & (1 << 31);
    if(exp == 0) return sign | (uf << 1);
    if(exp == 255) return uf;
    exp++;
    if(exp == 255) return sign | 0x7F800000;
    return (exp << 23) | (0x807FFFFF & uf);
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
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
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
/* If exp is 0, which is denormal, E = 1 - Bias = -126, return 0.
 * If exp if 255, then it's inf or NaN, return 0x80000000u.
 * Then uf is normal: 
 *   E = exp - Bias, M = 1 + frac.
 *   In binary representation, M=1.f1 f2 f3 f4 ... f23.
 *   Then M * 2^E is to move the point E to the right.
 *   Float to Int, throw away the part in the right of the point.
 *   So save E+1 bits of M's binary representation, counting from the left.
 * */
    int sign = uf & (1 << 31);
    int exp = (uf & 0x7F800000) >> 23;
    int frac = uf & 0x7FFFFF;
    int E = exp - 127;
    int M = frac + (1 << 23);
    int tmp = M << 7;
    int ret;

    if(exp == 255 || E >= 31) return 0x80000000u;
    if(E < 0) return 0;
    ret = tmp >> (30 - E);
    if(sign) ret = ~ret + 1;
    return ret;
}
