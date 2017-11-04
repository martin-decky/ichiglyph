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
 * This is Ichiglyph to Brainfuck transpiler. It decodes the Ichiglyph
 * instructions and outputs the corresponding Brainfuck instructions.
 *
 * Note that any characters not representing a Ichiglyph instruction
 * are silently ignored and dropped.
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

/** Ichiglyph instruction opcode */
typedef uint8_t ichiglyph_opcode_t[2];

/** Ichiglyph/Brainfuck instructions
 *
 * These are the eight Ichilglyph and Brainfuck instructions.
 * The INST_NOP instruction represets any other program inpu
 * that should be simply ignored.
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

/** Decode Ichiglyph instruction opcode
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
static instruction_t ichiglyph_opcode_decode(ichiglyph_opcode_t opcode)
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
	
	while (ip < program_size) {
		/*
		 * Ichiglyph instruction fetch and decode.
		 */
		ichiglyph_opcode_t ichiglyph_opcode;
		memcpy(&ichiglyph_opcode, program + ip,
		    sizeof(ichiglyph_opcode));
		
		instruction_t instruction =
		    ichiglyph_opcode_decode(ichiglyph_opcode);
		
		/*
		 * Brainfuck instruction encode.
		 */
		switch (instruction) {
		case INST_DP_INC:
			fputs(">", stdout);
			break;
		case INST_DP_DEC:
			fputs("<", stdout);
			break;
		case INST_VAL_INC:
			fputs("+", stdout);
			break;
		case INST_VAL_DEC:
			fputs("-", stdout);
			break;
		case INST_VAL_OUTPUT:
			fputs(".", stdout);
			break;
		case INST_VAL_ACCEPT:
			fputs(",", stdout);
			break;
		case INST_JMP_FORWARD:
			fputs("[", stdout);
			break;
		case INST_JMP_BACK:
			fputs("]", stdout);
			break;
		case INST_NOP:
			break;
		}
		
		ip++;
	}
	
	munmap(program, program_size);
	close(source);
	
	return 0;
}
