#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>


#ifndef likely
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#endif


/* 
 * Replace a pattern in src with custom text. May either leave or remove the original pattern
 * Works inplace
 */
int replace_buf1(char* dst, const char* with, const char* pattern, const char* src, int leave_pattern)
{
	const char* begin = strstr(src, pattern);
	if (!begin) return 0;

	if (begin > src)
	{
		// Then we have to copy the beginning
		memcpy(dst, src, (size_t)(begin - src));
	}

	strcpy(dst, with); // copy input
	if (leave_pattern)
		strcpy(dst, begin); // add pattern with the rest of the file
	else
	{
		size_t pattern_len = strlen(pattern);
		if (strlen(begin) > pattern_len) // check if there is data after the pattern
			strcpy(dst, begin + pattern_len);
	}

	return 1;
}


/*
 * Note on buf_size_half:
 *   buf_size_half is capped by the actual buffer size, since we need to account for EOF
 */


/* 
 * Find if the pattern is present in the double buffer. The buffer and pattern should be null-terminated
 */
int find_buf2(const char* pattern, const char* src)
{
	const char* begin = strstr(src, pattern);
	if (!begin) return -1;
	return begin - src; // Index
}


/*
 * Same as find_buf2, but supports multiple inclusions of the pattern. old_pattern_end should be 0 if there is no previous pattern
 */
int find_buf2_multi(const char* pattern, const char* src, int old_pattern_end, const int buf_size)
{
	if (old_pattern_end >= buf_size)
		return -1; // nothing to do
	const char* begin = strstr(src + old_pattern_end, pattern);
	if (!begin) return -1;
	return begin - src;
}


/*
 * Write the double buffer to output during the pattern search stage
 */
int write_buf2(int fd, const char* src, int pattern_pos, const int buf_size_half, const int buf_total, int eof)
{
	int total = 0;
	if (pattern_pos == -1)
	{
		// the start of the pattern may be in the second buffer, so we only write the first half
		if (!eof)
			total = buf_size_half;
		else
			total = buf_total;
	} else {
		// write everything up to the pattern starting position
		total = pattern_pos;
	}

	int written = 0;
	do {
		ssize_t bytes = write(fd, src, total - written);
		if (bytes == -1)
		{
			// handle err
		}
		written += bytes;
	} while (unlikely(written < total));
	return total; // do we need to return this?
}


/*
 * Same as write_buf2, but supports multiple inclusions of the pattern. old_pattern_end should always be >= than pattern_pos
 */
int write_buf2_multi(int fd, const char* src, int pattern_pos, int old_pattern_end, const int buf_size_half, const int buf_total, int eof)
{
	int total = 0;
	if (pattern_pos == -1)
	{
		if (!eof)
		{
			if (old_pattern_end >= buf_size_half)
				return 0; // nothing to do
			// the start of the pattern may be in the second buffer, so we only write the first half
			total = buf_size_half - old_pattern_end;
		} else {
			total = buf_total;
		}
	} else {
		// old_pattern_end >= pattern_pos
		// write everything up to the pattern starting position
		total = pattern_pos - old_pattern_end;
	}

	int written = 0;
	do {
		ssize_t bytes = write(fd, src + old_pattern_end, total - written);
		if (bytes == -1)
		{
			// handle err
		}
		written += bytes;
	} while (unlikely(written < total));
	return total;
}


/*
 * Replace pattern with the buffer. buf contains bytes that should substitute the pattern
 * This function may be called repeatedly
 */
void replace_buf2(int fd, const char* buf, const size_t buf_size)
{
	int written = 0;
	do {
		ssize_t bytes = write(fd, buf, buf_size - written);
		if (bytes == -1)
		{
			// handle err
		}
		written += bytes;
	} while (unlikely(written < buf_size));
}


/*
 * Write the double buffer to the output at the end of the pattern search stage
 * pattern_pos should indicate the position from where to write
 * This function is only used in single-pattern logic
 */
void write_buf2_end(int fd, const char* src, int pattern_pos, const int pattern_size, const int buf_size)
{
	int offset, total;
	if (pattern_pos == -1)
	{
		// We don't need to check for the pattern, so we write the whole buffer
		offset = 0;
		total = buf_size;
	} else {
		// We need to move the rest of the pattern to the end
		offset = pattern_pos + pattern_size;
		if (offset >= buf_size)
			return; // nothing to do
		total = buf_size - offset;
	}
	
	int written = 0;
	do {
		ssize_t bytes = write(fd, src + offset, total - written);
		if (bytes == -1)
		{
			// handle err
		}
		written += bytes;
	} while (unlikely(written < total));
}


/*
 * Move the second half of the buffer to the beginning. We assume that there is data in the second half of the buffer
 */
void move_buf2(char* src, const int buf_size_half, const int total)
{
	memmove(src, src + buf_size_half, total - buf_size_half); // Move buffer to the beginning
}



#endif
