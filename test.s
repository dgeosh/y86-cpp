.pos 0
init: irmovl Stack, %esp
      irmovl Stack, %ebp
      call Main
      halt
      .align 4
array: .long 0xd
       .long 0xc0
       .long 0xb00
       .long 0xa000
Main:  pushl %ebp
       rrmovl %esp, %ebp
       irmovl $4, %eax
       pushl %eax
       irmovl array, %edx
       pushl %edx
       call Sum
       rrmovl %ebp, %esp
       popl %ebp
       ret
Sum:   pushl %ebp
       rrmovl %esp, %ebp
       mrmovl 8(%ebp), %ecx
       mrmovl 12(%ebp), %edx
       xorl %eax, %eax
       andl %edx, %edx
       je End
Loop:  mrmovl (%ecx), %esi
       addl %esi, %eax
       irmovl $4, %ebx
       addl %ebx, %ecx
       irmovl $-1, %ebx
       addl %ebx, %edx
       jne Loop
End:   rrmovl %ebp, %esp
       popl %ebp
       ret