.pos 0x0
                 ld   $0x1028, r5         # initialize sp
                 ld   $0xfffffff4, r0     # r0 = -12
                 add  r0, r5              # allocate stack space
                 ld   $0x200, r0          # r0 = &a
                 ld   0x0(r0), r0         # r0 = a[0]
                 st   r0, 0x0(r5)         # store a[0] on stack
                 ld   $0x204, r0          # r0 = &a[1]
                 ld   0x0(r0), r0         # r0 = a[1]
                 st   r0, 0x4(r5)         # stpre a[1] on stack
                 ld   $0x208, r0          # r0 = &a[2]
                 ld   0x0(r0), r0         # r0 = a[2]
                 st   r0, 0x8(r5)         # store a[2] on stack
                 gpc  $6, r6              # set pc
                 j    0x300               # call procedure
                 ld   $0x20c, r1          # r1 = &z
                 st   r0, 0x0(r1)         # z = proc() == y
                 halt                     
.pos 0x200
                 .long 0x00000000         # a[0] = i
                 .long 0x00000000         # a[1] = x
                 .long 0x00000000         # a[2] = y
                 .long 0x00000000         # a[3] = z
.pos 0x300
                 ld   0x0(r5), r0         # r0 = a[0] = i
                 ld   0x4(r5), r1         # r1 = a[1] = x
                 ld   0x8(r5), r2         # r2 = a[2] = y
                 ld   $0xfffffff6, r3     # r3 = -10
                 add  r3, r0              # i = i-10
                 mov  r0, r3              # r3 = i-10
                 not  r3                  
                 inc  r3                  # r3 = -(i-10) = 10-i
                 bgt  r3, L6              # if (i < 10) goto L6 DEFAULT
                 mov  r0, r3              # else j = r3 = (i-10)
                 ld   $0xfffffff8, r4     # r4 = -8
                 add  r4, r3              # j = (i-10) - 8 == i-18
                 bgt  r3, L6              # if i > 18 goto L6 (i-18=0) DEFAULT
                 ld   $0x400, r3          # r3 = &JT
                 j    *(r3, r0, 4)        # goto jumptable[i-10]
.pos 0x330
                 add  r1, r2              # if (i==10) y += x
                 br   L7                  # goto CONT
                 not  r2                  # if (i==12) ~y
                 inc  r2                  # y = -y
                 add  r1, r2              # y = (x-y)
                 br   L7                  # goto CONT
                 not  r2                  # if (i==14) ~y
                 inc  r2                  # y = -y
                 add  r1, r2              # y = (x-y)
                 bgt  r2, L0              # if x>y goto L0 == (x-y>0)
                 ld   $0x0, r2            # else y = 0
                 br   L1                  # goto L1
L0:              ld   $0x1, r2            # if x>y then y=1
L1:              br   L7                  # else return 0 (x >= y -> goto CONT)
                 not  r1                  # if (i==16) ~x
                 inc  r1                  # if (i==16) x = -x
                 add  r2, r1              # x = y-x
                 bgt  r1, L2              # if y>x goto L2
                 ld   $0x0, r2            # else y = 0
                 br   L3                  # goto L3
L2:              ld   $0x1, r2            # if (x < y) return 1 
L3:              br   L7                  # else goto CONT
                 not  r2                  # if (i==18) ~y
                 inc  r2                  # y = -y
                 add  r1, r2              # y = x-y
                 beq  r2, L4              # if x==y goto L4 (x-y=0)
                 ld   $0x0, r2            # else y=0
                 br   L5                  # goto L5
L4:              ld   $0x1, r2            # if x==y then y=1
L5:              br   L7                  # else goto CONT
L6:              ld   $0x0, r2            # if (i<10||i>18) then y=0 (DEFAULT)
                 br   L7                  # goto CONT
L7:              mov  r2, r0              # ret = y
                 j    0x0(r6)             # return y
.pos 0x400
                 .long 0x00000330     # JT[0]    
                 .long 0x00000384     # JT[1]    
                 .long 0x00000334     # JT[2]    
                 .long 0x00000384     # JT[3]    
                 .long 0x0000033c     # JT[4]    
                 .long 0x00000384     # JT[5]    
                 .long 0x00000354     # JT[6]    
                 .long 0x00000384     # JT[7]    
                 .long 0x0000036c     # JT[8]    
.pos 0x1000
                 .long 0x00000000         
                 .long 0x00000000         
                 .long 0x00000000         
                 .long 0x00000000         
                 .long 0x00000000         
                 .long 0x00000000         
                 .long 0x00000000         
                 .long 0x00000000         
                 .long 0x00000000         
                 .long 0x00000000         
