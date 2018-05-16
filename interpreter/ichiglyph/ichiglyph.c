/*
 * Copyright (c) 2017 Martin Decky
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @file
 *
 * This is simple Ichiglyph interpreter. Ichiglyph is a simple
 * Turing-complete language derived from Brainfuck (as defined by
 * https://en.wikipedia.org/wiki/Brainfuck).
 *
 * Ichiglyph and Brainfuck implement the same set of instructions,
 * the only major difference is their encoding. The following table
 * presents the encoding of the instructions of both languages.
 *
 * Brainfuck | Ichiglyph
 * ----------+----------
 * >         | ll
 * <         | lI
 * +         | Il
 * -         | II
 * .         | 1l
 * ,         | 1I
 * [         | l1
 * ]         | I1
 *
 * The Ichiglyph language was inspired by a remark by Josefina Madrova,
 * who cleverly noted that using the characters l, I and 1 in identifiers
 * is a bad practice.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>

/** Memory allocation granularity */
#define DATA_GRANULARITY  32768

/** Ichiglyph instruction opcode */
typedef uint8_t ichiglyph_opcode_t[2];

/** Ichiglyph instructions
 *
 * These are the eight Ichiglyph instructions. The INST_NOP
 * instruction represets any other program input that should
 * be simply ignored.
 *
 */
typedef enum {
	INST_DP_INC,
	INST_DP_DEC,
	INST_VAL_INC,
	INST_VAL_DEC,
	INST_VAL_OUTPUT,
	INST_VAL_ACCEPT,
	INST_JMP_FORWARD,
	INST_JMP_BACK,
	INST_NOP
} instruction_t;

/** Data memory
 *
 * Ichiglyph data memory is unbounded by definition. To
 * accomodate such abstraction we resize the actual data
 * memory on demand.
 *
 */
typedef struct {
	uint8_t *data;  /**< Actual data */
	size_t size;    /**< Size of the currently allocated data */
} data_t;

/** Decode instruction opcode
 *
 * Decode the Ichiglyph instruction opcode. The eight valid
 * instruction characters are decoded to the respective
 * instructions, any unrecognized characters are interpretted
 * as a NOP.
 *
 * @param opcode Instruction opcode.
 *
 * @return Decoded instruction.
 *
 */
static instruction_t opcode_decode(ichiglyph_opcode_t opcode)
{
	switch (opcode[0]) {
	case 'l':
		switch (opcode[1]) {
		case 'l':
			return INST_DP_INC;
		case 'I':
			return INST_DP_DEC;
		case '1':
			return INST_JMP_FORWARD;
		default:
			return INST_NOP;
		}
	case 'I':
		switch (opcode[1]) {
		case 'l':
			return INST_VAL_INC;
		case 'I':
			return INST_VAL_DEC;
		case '1':
			return INST_JMP_BACK;
		default:
			return INST_NOP;
		}
	case '1':
		switch (opcode[1]) {
		case 'l':
			return INST_VAL_OUTPUT;
		case 'I':
			return INST_VAL_ACCEPT;
		default:
			return INST_NOP;
		}
	default:
		return INST_NOP;
	}
}

/** Initialize data memory
 *
 * Initialize data memory. Actually no memory is allocated.
 *
 * @param data Data memory to initialize.
 *
 */
static void data_init(data_t *data)
{
	data->data = NULL;
	data->size = 0;
}

/** Cleanup data memory
 *
 * Free the data memory.
 *
 * @param data Data memory to be freed.
 *
 */
static void data_done(data_t *data)
{
	free(data->data);
	data->data = NULL;
	data->size = 0;
}

/** Check data memory access bound
 *
 * Make sure the access to the data memory is safe
 * by reallocating the data memory to the required size.
 * Data cells are initialized to 0.
 *
 * @param data Data memory.
 * @param dp   Data memory pointer.
 *
 * @return 0 if the data cell can be safely accessed.
 * @return Non-zero value if the data cell cannot be accessed
 *         (out-of-memory condition).
 *
 */
static int data_bound(data_t *data, size_t dp)
{
	if (dp >= data->size) {
		size_t size = dp + 1 + DATA_GRANULARITY;
		data->data = (uint8_t *) realloc(data->data, size);
		if (data->data == NULL)
			return -1;
		
		memset(data->data + data->size, 0, size - data->size);
		data->size = size;
	}
	
	return 0;
}

/** Increment the value of a data cell
 *
 * Increment the value of a data cell at the data pointer.
 *
 * @param data Data memory.
 * @param dp   Data memory pointer.
 *
 * @return 0 if the data cell was increased.
 * @return Non-zero value if the data cell cannot be increased
 *         (out-of-memory condition).
 *
 */
static int data_inc(data_t *data, size_t dp)
{
	int ret = data_bound(data, dp);
	if (ret != 0)
		return ret;
	
	data->data[dp]++;
	return 0;
}

/** Decrement the value of a data cell
 *
 * Decrement the value of a data cell at the data pointer.
 *
 * @param data Data memory.
 * @param dp   Data memory pointer.
 *
 * @return 0 if the data cell was decreased.
 * @return Non-zero value if the data cell cannot be decreased
 *         (out-of-memory condition).
 *
 */
static int data_dec(data_t *data, size_t dp)
{
	int ret = data_bound(data, dp);
	if (ret != 0)
		return ret;
	
	data->data[dp]--;
	return 0;
}

/** Get the value of a data cell
 *
 * Get the value of a data cell at the data pointer. Data
 * cells are initialized to 0.
 *
 * @param data Data memory.
 * @param dp   Data memory pointer.
 *
 * @return Value of the data cell.
 *
 */
static uint8_t data_get(data_t *data, size_t dp)
{
	if (dp >= data->size)
		return 0;
	
	return data->data[dp];
}

/** Set the value of a data cell
 *
 * Set the value of a data cell at the data pointer.
 *
 * @param data Data memory.
 * @param dp   Data memory pointer.
 * @param val  New data cell value.
 *
 * @return 0 if the data cell was set.
 * @return Non-zero value if the data cell cannot be set
 *         (out-of-memory condition).
 *
 */
static int data_set(data_t *data, size_t dp, uint8_t val)
{
	int ret = data_bound(data, dp);
	if (ret != 0)
		return ret;
	
	data->data[dp] = val;
	return 0;
}

int main(int argc, char *argv[])
{
	/*
	 * The first command-line argument is the Ichiglyph
	 * source file.
	 */
	if (argc < 2) {
		fprintf(stderr, "Syntax: %s <source>\n", argv[0]);
		return 1;
	}
	
	char *source_name = argv[1];
	int source = open(source_name, O_RDONLY);
	if (source < 0) {
		fprintf(stderr, "%s: Unable to open\n", source_name);
		return 2;
	}
	
	struct stat stat;
	int ret = fstat(source, &stat);
	if (ret != 0) {
		fprintf(stderr, "%s: Unable to stat\n", source_name);
		close(source);
		return 3;
	}
	
	size_t program_size = stat.st_size / sizeof(ichiglyph_opcode_t);
	
	/*
	 * We mmap the entire source file.
	 */
	ichiglyph_opcode_t *program = (ichiglyph_opcode_t *) mmap(NULL,
	    program_size * sizeof(ichiglyph_opcode_t), PROT_READ,
	    MAP_PRIVATE, source, 0);
	if (program == MAP_FAILED) {
		fprintf(stderr, "%s: Unable to mmap\n", source_name);
		close(source);
		return 4;
	}
	
	size_t ip = 0;
	size_t dp = 0;
	
	data_t data;
	data_init(&data);
	
	while (ip < program_size) {
		/*
		 * Instruction fetch and decode.
		 */
		ichiglyph_opcode_t opcode;
		memcpy(&opcode, program + ip, sizeof(opcode));
		
		instruction_t instruction = opcode_decode(opcode);
		uint8_t val;
		int input_val;
		
		/*
		 * Instruction execute.
		 */
		switch (instruction) {
		case INST_DP_INC:
			dp++;
			break;
		case INST_DP_DEC:
			dp--;
			break;
		case INST_VAL_INC:
			ret = data_inc(&data, dp);
			if (ret != 0) {
				fprintf(stderr, "%s: Out of memory\n", source_name);
				ip = program_size;
			}
			
			break;
		case INST_VAL_DEC:
			ret = data_dec(&data, dp);
			if (ret != 0) {
				fprintf(stderr, "%s: Out of memory\n", source_name);
				ip = program_size;
			}
			
			break;
		case INST_VAL_OUTPUT:
			val = data_get(&data, dp);
			fputc(val, stdout);
			fflush(stdout);
			break;
		case INST_VAL_ACCEPT:
			input_val = fgetc(stdin);
			if (input_val == EOF) {
				ip = program_size;
				break;
			}
			
			ret = data_set(&data, dp, input_val);
			if (ret != 0) {
				fprintf(stderr, "%s: Out of memory\n", source_name);
				ip = program_size;
			}
			
			break;
		case INST_JMP_FORWARD:
			val = data_get(&data, dp);
			
			if (val == 0) {
				size_t balance = 1;
				
				while (balance != 0) {
					if (ip == program_size) {
						/*
						 * No matching instruction can be found.
						 * We simply terminate the execution.
						 */
						break;
					}
					
					ip++;
					
					ichiglyph_opcode_t opcode2;
					memcpy(&opcode2, program + ip, sizeof(opcode2));
					
					instruction_t instruction2 = opcode_decode(opcode2);
					
					switch (instruction2) {
					case INST_JMP_FORWARD:
						balance++;
						break;
					case INST_JMP_BACK:
						balance--;
						break;
					default:
						break;
					}
				}
			}
			
			break;
		case INST_JMP_BACK:
			val = data_get(&data, dp);
			
			if (val != 0) {
				size_t balance = 1;
				
				while (balance != 0) {
					if (ip == 0) {
						/*
						 * No matching instruction can be found.
						 * We simply terminate the execution.
						 */
						ip = program_size;
						break;
					}
					
					ip--;
					
					ichiglyph_opcode_t opcode2;
					memcpy(&opcode2, program + ip, sizeof(opcode2));
					
					instruction_t instruction2 = opcode_decode(opcode2);
					
					switch (instruction2) {
					case INST_JMP_FORWARD:
						balance--;
						break;
					case INST_JMP_BACK:
						balance++;
						break;
					default:
						break;
					}
				}
			}
			
			break;
		case INST_NOP:
			break;
		}
		
		ip++;
	}
	
	data_done(&data);
	munmap(program, program_size);
	close(source);
	
	return 0;
}
