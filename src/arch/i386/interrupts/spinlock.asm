; Spinlock implementation

global _spinlock_lock
global _spinlock_unlock

; Lock the spinlock
_spinlock_lock:
    push ebp
    mov ebp, esp

    push eax
    push ebx

    mov ebx, [ebp + 8]

_spinlock_spin:
    mov eax, 1

    lock xchg eax, [ebx]
    test eax, eax
    jnz _spinlock_spin

    pop ebx
    pop eax

    pop ebp
    ret

; Unlock the spinlock
_spinlock_unlock:
    push ebp
    mov ebp, esp

    push eax
    push ebx

    mov ebx, [ebp + 8]
    mov eax, 0

    lock xchg eax, [ebx]

    pop ebx
    pop eax

    pop ebp
    ret
