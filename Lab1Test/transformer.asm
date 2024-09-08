SECTION .data
msg0        db      'Error', 0h
msgb        db      '0b', 0h
msgo        db      '0o', 0h
msgh        db      '0x', 0h
 
SECTION .bss
strinput:     resb    255
ansout:       resb    255

SECTION .text
global  _start
 
_start:
    ; 读入
    mov     edx, 40        
    mov     ecx, strinput   ; 输入地址  
    mov     ebx, 0          
    mov     eax, 3           
    int     80h

    mov     ecx, 0      ; ecx 十进制长度

    mov     esi, strinput   ; esi，当前字符的位置
    ; 判断退出
    xor     ebx, ebx        ; ebx 置 0，方便下面存字符
    mov     bl, [esi]   ; 取第一个字节

    cmp     bl, 113         ; 和 q 比较
    je      exit    

; 获取十进制长度，同时判断输入的合法性    
stringToInt:
    xor     ebx, ebx        ; ebx 置 0，方便下面存字符
    mov     bl, [esi]   ; 取一字节
    
    ; 检查空格
    cmp     bl, 32 
    je      typeCheck       ; 判断类型

    ; 检查整数
    cmp     bl, 48          ; 和 0 比较
    jl      error          ; 小于 0 ，发生 error
    cmp     bl, 57          ; 和 9 比较
    jg      error       ; 大于 9 ，发生 error

    inc     ecx            ; 位数 ++
    inc     esi             ; 后移
    jmp     stringToInt

; 检查转换进制类型
typeCheck:
    
    xor     ebx, ebx        ; ebx 置 0，方便下面存字符

    inc     esi
    mov     bl, [esi]         ; 取一字节

    cmp     bl, 98          ; b, 二进制
    je      binary
    cmp     bl, 111         ; o, 八进制
    je      octal
    cmp     bl, 104         ; h, 十六进制
    je      hexadecimal
    
    jmp     error
    
; 二进制处理
binary:
    mov     ebx, 2

    push    ecx
    push    edx
    mov     ecx, msgb
    mov     edx, 2
    call    sprint
    pop     edx
    pop     ecx

    jmp     transfer

; 八进制处理
octal:
    mov     ebx, 8
    
    push    ecx
    push    edx
    mov     ecx, msgo
    mov     edx, 2
    call    sprint
    pop     edx
    pop     ecx

    jmp     transfer

; 十六进制处理
hexadecimal:
    mov     ebx, 16
    
    push    ecx
    push    edx
    mov     ecx, msgh
    mov     edx, 2
    call    sprint
    pop     edx
    pop     ecx

    jmp     transfer

; 进制转化过程
transfer:

    ; ebx 存需要转化的进制所对应除 2 的次数
    ; ecx 十进制位数
    mov     esi, ansout

; 模拟除法，每次获得 ansout 的一位 
divLoop:
    xor     edx, edx
    mov     edx, 0

; 除以 2 / 8 / 16，每次获得除一次的余数
.loop:
    call    divTry              ; edx 获得新的余数

    ; 十六进制字母转化
    cmp     dl, 9
    jg      bigtochar

    add     dl, 48     ; 转为 字符

ret:  ; 十六进制字母转化结束返回的断点

    mov     [esi], dl           ; div 的余数放入 ansout
    inc     esi

    ; 判断是否结束/strinput 为 0
    push    ebx
    push    eax
    mov     eax, 0
    call    checkZero
    pop     eax
    pop     ebx 
   
    jmp     divLoop

;  自顶向下输出结果
printout:
    mov esi, ansout

; 找到 ansout 中数字字符串的结束位置
.find_end:
    cmp byte [esi], 0
    je .output
    inc esi
    jmp .find_end

.output:
    ; 回退到字符字符串的末尾位置
    dec esi
.print:
    ; 依次输出每个字符
    cmp esi, ansout
    jl .exit  ; 如果已经到达字符串开头，则退出程序
    
    mov eax, 4  
    mov ebx, 1  
    mov ecx, esi  ; 要输出的字符地址
    mov edx, 1  ; 要输出的字符数量
    int 0x80    
    dec esi
    jmp .print    

.exit:
    call    printLF
    pop     eax

; 刷新 SECTION .bss 区域
clear:
    ; 初始化 strinput 变量为全零
    mov edi, strinput   
    mov ecx, 255        
    xor eax, eax        
    rep stosb           ; 将 eax 寄存器的值（0）重复存储到 edi 指向的内存地址，直到 ecx 计数器减为零

    ; 初始化 ansout 变量为全零
    mov edi, ansout     
    mov ecx, 255        
    xor eax, eax        
    rep stosb           

    jmp     _start

; 退出
exit:
    mov     ebx, 0
    mov     eax, 1
    int     80h

; 十六进制字母转化
bigtochar:
    add     edx, 87
    jmp     ret

; 一位 除以 进制
divTry:
    push    ecx
    push    eax   
    push    esi 
    mov     edx, 0   ; edx 上一位的 十进制 int

    mov     esi, strinput

.divLoop:
    ;上一位 * 10 放入 eax
    xor     eax, eax
    mov     eax, 10             ; eax 置 10
    mul     edx 

    ; 当前位 加入 eax
    push    ebx
    xor     ebx, ebx
    mov     bl, [esi]           ; edx 当前位的 十进制 char
    sub     bl, 48              ; char to int
    add     eax, ebx            ; eax 当前的被除数
    pop     ebx 

    ; eax 除以进制
    div     ebx

    ; eax   商存回 strinput
    push    ebx
    mov     ebx, eax
    add     bl, 48
    mov     [esi], bl
    pop     ebx

    ; 判断 str 是否除到头
    dec     ecx
    inc     esi
    cmp     ecx, 0
    jne     .divLoop

.store:  
    pop     esi  
    pop     eax
    pop     ecx
    ret


; 检查 strinput 是否为 0
checkZero:
    mov     bl, [strinput+eax]
    cmp     bl, 48
    jne     .ret
    inc     eax
    
    cmp     eax, ecx
    je      printout
    jmp     checkZero
.ret:
    ret


; 错误输入
error:
    ; 打印 error
    push    ecx
    push    edx
    mov     ecx, msg0
    mov     edx, 5
    call    sprint
    pop     edx
    pop     ecx

    call    printLF

    jmp     _start       ; 下一次输入


; 打印， ecx 地址，edx 长度
sprint:
    push    eax
    push    ebx

    mov     ebx, 1
    mov     eax, 4
    int     80h
    
    pop     ebx
    pop     eax
    ret

; 打印换行符
printLF:
    push    eax
    push    ebx
    push    ecx
    push    edx

    mov     eax, 0AH
    push    eax
    mov     ecx, esp
    mov     eax, 4
    mov     ebx, 1
    mov     edx, 1      ; 输出字节数
    int     80h
    pop     eax

    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret