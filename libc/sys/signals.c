#include <sys/signals.h>
#include <sys/syscall.h>

void set_sig_handler(uint32_t sig, sig_handler_t handler)
{
	sys_reg_sig_handler(sig, handler);
}

bool signal(uint32_t pid, uint32_t sig)
{
	return sys_send_signal(pid, sig);
}