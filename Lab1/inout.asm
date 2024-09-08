section .data
    size    dd  34           ; 字符串长度为 5
    in_buf  times 32 db 0   ; 输入缓冲区，大小为 32 字节，足够存储 "Hello" 和换行符
    error  db "Error", 10, 0Ah  ; ⽆效输⼊

section .text
global _start

_start:
	
    ; 设置系统调用号为 0x03，即 read
    mov eax, 3
    ; 文件描述符为标准输入
    mov ebx, 0
    ; 数据缓冲区地址
    mov ecx, in_buf
    ; 要读取的字节数
    mov edx, [size]
    ; 进行系统调用
    int 0x80

    ; 打印输入的字符串
    mov eax, 4             ; 设置系统调用号为 4，即 write
    mov ebx, 1             ; 文件描述符为标准输出
    mov ecx, in_buf        ; 数据缓冲区地址
    mov edx, [size]        ; 要打印的字节数
    int 0x80

    ; 退出程序
    mov eax, 1             ; 设置系统调用号为 1，即 exit
    xor ebx, ebx           ; 返回值为 0
    int 0x80

errotExit:
    mov eax, 4             
    mov ebx, 1             
    mov ecx, error        ; 地址
    mov edx, 5       ; 要打印的字节数
    int 0x80
