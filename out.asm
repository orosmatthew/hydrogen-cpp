global _start
_start:
    ;; exit
    mov rax, 1
    push rax
    mov rax, 60
    pop rdi
    syscall
    ;; /exit
    mov rax, 60
    mov rdi, 0
    syscall
