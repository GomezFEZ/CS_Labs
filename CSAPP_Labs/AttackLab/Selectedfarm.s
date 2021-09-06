000000000000001f <addval_219>:
  1f:	f3 0f 1e fa          	
  23:	8d 87 51 73 
  	58 			popq %rax
  	90    			
  29:	c3                   	

000000000000004b <setval_426>:
  4b:	f3 0f 1e fa          	
  4f:	c7 07 
  	48 89 c7		mov %rax,%rdi 	
  	90    	
  55:	c3                   	                 	

00000000000000b3 <addval_190>:
  b3:	f3 0f 1e fa         
  b7:	8d 87 41 
  	48 89 e0    		movq %rsp,%rax
  bd:	c3                                   	

00000000000000c9 <addval_436>:
  c9:	f3 0f 1e fa          	
  cd:	8d 87 
  	89 ce 			movl %ecx,%esi
  	90 90    	
  d3:	c3                     

00000000000000ff <getval_159>:
  ff:	f3 0f 1e fa          	
 103:	b8 
 	89 d1			mov %edx,%ecx 
 	38 c9       	
 108:	c3                     

0000000000000109 <addval_110>:
 109:	f3 0f 1e fa          	
 10d:	8d 87 c8 
 	89 e0 			mov %esp,%eax
 	c3    	
 113:	c3                   	 

0000000000000114 <addval_487>:
 114:	f3 0f 1e fa          	
 118:	8d 87 
 	89 c2 			movl %eax,%edx
 	84 c0    	
 11e:	c3                   	

0000000000000154 <getval_311>:
 154:	f3 0f 1e fa          	
 158:	b8 
 	89 d1 			movl %edx,%ecx
 	08 db       	
 15d:	c3                   	
                 	
000000000000006a <add_xy>:
  6a:	f3 0f 1e fa          	
  6e:	48 8d 04 37          	/* add two arguements */
  72:	c3    
