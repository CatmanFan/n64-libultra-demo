#ifndef __READER_H__
#define __READER_H__

extern void init_reader();
extern void load_segment(char* seg, char address, char start, char end);
extern void load_binary(void *rom_address, void *ram_buffer, int size);
extern void load_all_segments();

#endif