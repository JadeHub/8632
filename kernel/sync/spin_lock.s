[global spin_lock:function]
spin_lock:
    mov eax, [esp+4]
    test dword [eax], 1      ;Is the lock free?
    jnz spin_lock           ;no, wait
 acquireLock:
    lock bts dword [eax], 0        ;Attempt to acquire the lock (in case lock is uncontended)
    jc spin_lock            ;Spin if locked ( organize code such that conditional jumps are typically not taken ) 
    ret                      ;Lock obtained

[global spin_unlock:function]
spin_unlock:
    mov eax, [esp+4]
    mov dword [eax],0
    ret