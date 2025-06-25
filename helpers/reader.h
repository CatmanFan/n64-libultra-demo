extern void init_reader();
extern void read_segment(char* seg, char address, char start, char end);
extern void read_binary(u32 src, void *dst, u32 size);
extern void read_all_segments();