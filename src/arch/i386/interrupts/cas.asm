global cas

cas:
	push ebp
	mov ebp, esp

	mov eax, [ebp + 12] ; Load old value
	mov ebx, [ebp + 16] ; Load new vale

	lock cmpxchg [ebp + 8], ebx ; do locked exchnage

	mov eax, 0
	setz al

	pop ebp
	ret