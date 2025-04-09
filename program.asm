section .data
    newline db 0xA, 0

section .bss
    t1 resd 1
    t2 resd 1
    t3 resd 1

section .text
    global _start

_start:
    ; t1 = 5
    mov dword [t1], 5

    ; t2 = 10
    mov dword [t2], 10

    ; t3 = t1 + t2
    mov eax, [t1]
    add eax, [t2]
    mov [t3], eax

    ; print t3
    ; convert t3 to string
    mov eax, [t3]
    call print_int

    ; newline
    mov eax, 4
    mov ebx, 1
    mov ecx, newline
    mov edx, 1
    int 0x80

    ; exit
    mov eax, 1
    xor ebx, ebx
    int 0x80

;--------------------------------------
; print_int: prints EAX as decimal
;--------------------------------------
print_int:
    ; buffer to store digits
    push eax
    mov ecx, 10
    mov edi, esp         ; stack as buffer
.print_loop:
    xor edx, edx
    div ecx              ; EAX /= 10, EDX = remainder
    add dl, '0'
    push dx
    test eax, eax
    jnz .print_loop

.print_digits:
    pop ax
    mov [esp - 1], al
    mov eax, 4
    mov ebx, 1
    lea ecx, [esp - 1]
    mov edx, 1
    int 0x80
    cmp esp, edi
    jne .print_digits

    add esp, 4           ; restore stack
    ret
