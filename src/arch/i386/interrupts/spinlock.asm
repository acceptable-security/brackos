; Spinlock implementation

global spinlock_lock
global spinlock_unlock

; Lock the spinlock
spinlock_lock:
    push eax
    push ebp

_spinlock_spin:
    mov eax, 1
    mov ebx, [ebp + 8]

    xchg eax, [ebx]
    jnz _spinlock_spin

    pop ebp
    pop eax
    ret

; Unlock the spinlock
spinlock_unlock:
    push eax
    push ebx

    mov eax, 0
    mov ebx, [ebp + 8]

    xchg eax, [ebx]

    pop ebx
    pop eax
    ret
