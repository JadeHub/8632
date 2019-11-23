void main() {
	char* video_memory = (char *) 0xb8000;
	char* c  = (char*) 0x8500;
	*video_memory = *c;
}
