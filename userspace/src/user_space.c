#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <sys/signals.h>
#include <sys/io_defs.h>

void test_io()
{
	uint32_t fd = sys_open("/initrd/temp/f1.txt", 0);
	if (fd == 0xffffffff)
	{
		printf("Failed to open file");
	}
	else
	{
		printf("Opened file ");

		size_t l = 80;
		char buff[80];
		memset(buff, 0, l);
		sys_read(fd, buff, l-1);
		printf(buff);
		sys_close(fd);
	}
}

void test_sleep()
{
	printf("Sleeping\n");

	sys_sleep_ms(1000);

	printf("Sleep finished\n");
}


void test_con()
{
	uint32_t fd = sys_open("/dev/console", 0);
	if (fd == 0xffffffff)
	{
		printf("Failed to open consol");
	}
	else
	{
		char buff[128];

		do
		{
			printf("Prompt> ");
			sys_read(fd, buff, 127);
			printf("Read %s len %d\n", buff, strlen(buff));
		} while (buff[0] != 'q');
	}
}

#include <sys/keyboard.h>

static const char* _key_state(bool b)
{
	static const char* p = "pressed";
	static const char* r = "released";
	return b ? p : r;
}

static const char _bool_state(bool s)
{
	return s ? 'Y' : 'N';
}

void read_kbd()
{
	uint32_t fd = sys_open("/dev/keyboard", 0);
	if (fd == 0xffffffff)
	{
		printf("Failed to open keyboard\n");
		return;
	}
	printf("Reading... Ctrl+c to stop\n");
	kb_event_t kbe;
	for (;;)
	{
		sys_read(fd, (uint8_t*)&kbe, sizeof(kb_event_t));
		printf("0x%02x %-10s %-8s C[%c%c] A[%c%c] S[%c%c]\n", kbe.code, kb_key_name(kbe.code),
			_key_state(kbe.pressed),
			_bool_state(kbe.state.lctrl), _bool_state(kbe.state.rctrl),
			_bool_state(kbe.state.lalt), _bool_state(kbe.state.ralt),
			_bool_state(kbe.state.lshift), _bool_state(kbe.state.rshift)			
			);
		if (kbe.code == 'c' && kb_is_ctrl(&kbe.state))
			break;
	}
}

#include <dirent.h>

void test_dir()
{
	struct DIR* d = opendir("/");

	if (!d)
	{
		printf("opendir failed\n");
		return;
	}

	printf("dir opened\n");

	struct dirent de;
	while (readdir(d, &de))
	{
		
		printf("got %s\n", de.name);
	}

	closedir(d);
	//printf("dir closed\n");

}

extern uint32_t regs_cs();

void cb()
{
	printf("CB! \n");
	sys_sig_handler_return();
}

static uint64_t _coutn = 0;

void calc()
{
	_coutn++;
	sys_sleep_ms(10);
}

int main(int argc, char* argv[])
{
	char* msg = "Hello from user land %d";

	printf("Hello\n");
	//printf("Hello\n");

	//sys_reg_sig_handler(1, cb);

	uint32_t fd = sys_open("fatfs/initrd/bin/test.txt", OPEN_READ);

	if (fd == 0xffffffff)
	{
		printf("Failed to open file\n");
	}
	else
	{
		printf("Opened file\n");

		size_t l = 80;
		char buff[80];
		memset(buff, 0, l);
		size_t s = sys_read(fd, buff, l-1);
		//size_t s = 12;
		buff[l - 1] = '\0';
		printf("Read %d bytes %s\n", s, buff);
		
		//printf("%d 0x%x\n", buff[2000], buff[2000]);
		sys_close(fd);
	}

	//sys_send_signal()

	//for (int i = 0; i < 100000; i++)
	//	for (int j = 0; j < 10000; j++)
		//	calc();
	//test_io();

	//for(int j=0;j<5;j++)
		//test_sleep();
	
	//test_con();
	//read_kbd();
	//ftest_dir();
	return 0;
}