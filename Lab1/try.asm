section .data
    input_buffer db 1  ; 输入地址
    type_buffer db 1   ; 类型地址
    newline db '\n' ; 换行符
    blank db ' '    ; 空格
    len db 0        ; 输入字符串长度 
    

section .text
global _start

_start:
    mov rcx, input_buffer        ; 将 输入地址加载到 rxc 寄存器中
    
    read: 
        
	; 读取单个字符
	mov rax, 0
        mov rdi, 0
        mov rsi, rcx ; 输入缓冲区地址
        mov rdx, 1
        syscall

	; 检查输入的字符是否为换行符
	cmp byte [rcx], '\n'  
	je exit          ; 如果是换行符，报错

	; 检查输入的字符是否为空格
	cmp byte [rcx], ' '  
	je readType
	
	; 实现len 加一
        mov al, [len]  ; 将len的值加载到al中
        inc al              ; 将al中的值加一
        mov [len], al  ; 将al中的值存回len中
    	
        add rcx, rax        ;update 地址

	; 在这里可以使用 input_buffer 中的字符进行操作

	; 继续读取下一个字符，可以使用循环来重复读取字符
	jmp read

    readType:
        ; 读取类型
        mov rax, 0
        mov rdi, 0
        mov rsi, type_buffer ; 类型缓冲区地址
        mov rdx, 1
        syscall

        ; 退出
        jmp exit

exit:
    mov rax, 1             ; 设置系统调用号为 1，即 write
    mov rdi, 1             ; 文件描述符为标准输出
    mov rsi, input_buffer  ; 数据缓冲区地址
    mov rdx, [len]         ; 要打印的字节数
    syscall                ; 进行系统调用	
    
    ; 退出程序
    mov rax, 60
    mov rdi, 0
    syscall

