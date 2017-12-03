; Spinlock implementation

global _spinlock_lock
global _spinlock_unlock

; Lock the spinlock
_spinlock_lock:
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
_spinlock_unlock:
    push eax
    push ebx

    mov eax, 0
    mov ebx, [ebp + 8]

    xchg eax, [ebx]

    pop ebx
    pop eax
    ret
