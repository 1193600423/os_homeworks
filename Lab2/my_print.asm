section .text
global my_print

my_print:
    push edx
    push ecx
    push ebx
    push eax
    push ebp
    mov ebp, esp
    mov edx, [ebp+28]
    mov ecx, [ebp+24]
    mov ebx, 1
    mov eax, 4
    int 80h
    pop ebp
    pop eax
    pop ebx
    pop ecx
    pop edx
    ret