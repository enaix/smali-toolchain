#ifndef REPLACE_H
#define REPLACE_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"



#define BUF_ACTUAL_SIZE 256
#define BUF_SIZE_SINGLE BUF_ACTUAL_SIZE - 1 // Reserve 1 for '\0', but we cannot reserve only one char for the double buffer

#define SUBST_BUF_SIZE 256



int do_replace(int out_fd, int subst_fd, int leave_pattern, const char* pattern, const size_t pattern_size)
{
	char subst[SUBST_BUF_SIZE];
	int eof = 0;
	do {
		int read_size = 0;
		do {
			ssize_t bytes = read(subst_fd, subst, SUBST_BUF_SIZE - read_size); // read remaining bytes
			if (bytes == -1)
			{
				// handle err
				return 0;
			}
			read_size += bytes;
			if (bytes == 0)
			{
				eof = 1;
				break; // done reading
			}
		} while (unlikely(read_size < SUBST_BUF_SIZE));
		
		// Since we're not using str* functions, we shouldn't null-terminate the string
		// use read_size as actual buf_size
		replace_buf2(out_fd, subst, read_size);	
	} while (!eof);
	
	if (leave_pattern)
		replace_buf2(out_fd, pattern, pattern_size);
	
	return 1;
}



int replace_single(int fd, int out_fd, int subst_fd, const char* pattern, const size_t pattern_size, int leave_pattern)
{
	assert(pattern_size < BUF_SIZE_SINGLE && "replace_single() : pattern does not fit into the double buffer");
	// double buffer init
	char buf2[BUF_ACTUAL_SIZE * 2];
	
	// populate the double buffer
	int read_size = 0, eof = 0;
	do {
		ssize_t bytes = read(fd, buf2, BUF_SIZE_SINGLE * 2 - read_size); // read remaining bytes
		if (bytes == -1)
		{
			// handle err
		}
		read_size += bytes;
		if (bytes == 0)
		{
			eof = 1;
			break; // done reading, buf size is smaller!
		}
	} while (unlikely(read_size < BUF_SIZE_SINGLE * 2)); // Fill the rest of the buffer

	buf2[read_size] = '\0'; // actual size is larger by 2 bytes
	
	// search for the pattern
	while(1)
	{
		// If we encountered EOF:
		int buf_single_size = (read_size < BUF_SIZE_SINGLE ? read_size : BUF_SIZE_SINGLE);
	
		int pattern_pos = find_buf2(pattern, buf2);

		// write_buf2 may not write the second half of the buffer, so we would need to write remaining characters later
		write_buf2(out_fd, buf2, pattern_pos, buf_single_size, read_size, eof);

		if (pattern_pos != -1)
		{
			// handle the found pattern
			if (!do_replace(out_fd, subst_fd, leave_pattern, pattern, pattern_size))
				return 0;
			// dump the rest of the buffer
			write_buf2_end(out_fd, buf2, pattern_pos, pattern_size, read_size);
			break; // exit the loop - write the remaining file
		} else {
			// move the double buffer and read the rest
			if (!eof)
			{
				move_buf2(buf2, BUF_SIZE_SINGLE, read_size);
				read_size = 0;

				// Populate the second half of the buffer
				do {
					ssize_t bytes = read(fd, buf2 + BUF_SIZE_SINGLE, BUF_SIZE_SINGLE - read_size);
					if (bytes == -1)
					{
						// handle err
					}
					read_size += bytes;
					if (bytes == 0)
					{
						eof = 1;
						break; // done reading, buf size is smaller!
					}
				} while (unlikely(read_size < BUF_SIZE_SINGLE));
				read_size += BUF_SIZE_SINGLE; // since the first half of the buffer is still present
			} else return 1; // reached EOF, nothing to do now - 
		}
	}

	if (eof) return 1; // Nothing to write

	// Write the rest of the file
	do
	{
		read_size = 0;
		do {
			ssize_t bytes = read(fd, buf2, BUF_SIZE_SINGLE * 2 - read_size);
			if (bytes == -1)
			{
				// handle err
			}
			read_size += bytes;
			if (bytes == 0)
			{
				eof = 1;
				break; // done reading, buf size is smaller!
			}
		} while (unlikely(read_size < BUF_SIZE_SINGLE * 2));
		write_buf2_end(out_fd, buf2, -1, pattern_size, read_size);
	} while (!eof);
	return 1;
}

#endif
