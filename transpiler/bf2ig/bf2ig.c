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
 * This is Brianfuck to Ichiglyph transpiler. It decodes the Brainfuck
 * instructions according the common specification available at
 * https://en.wikipedia.org/wiki/Brainfuck and outputs the corresponding
 * Ichiglyph instructions.
 *
 * Note that any characters not representing a Brainfuck instruction
 * are silently ignored and dropped.
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

/** Brainfuck/Ichiglyph instructions
 *
 * These are the eight Brainfuck and Ichilglyph instructions.
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

/** Decode Brainfuck instruction opcode
 *
 * Decode the Brainfuck instruction opcode. The eight valid
 * instruction characters are decoded to the respective
 * instructions, any unrecognized characters are interpretted
 * as a NOP.
 *
 * @param opcode Instruction opcode.
 *
 * @return Decoded instruction.
 *
 */
static instruction_t brainfuck_opcode_decode(uint8_t opcode)
{
	switch (opcode) {
	case '>':
		return INST_DP_INC;
	case '<':
		return INST_DP_DEC;
	case '+':
		return INST_VAL_INC;
	case '-':
		return INST_VAL_DEC;
	case '.':
		return INST_VAL_OUTPUT;
	case ',':
		return INST_VAL_ACCEPT;
	case '[':
		return INST_JMP_FORWARD;
	case ']':
		return INST_JMP_BACK;
	default:
		return INST_NOP;
	}
}

int main(int argc, char *argv[])
{
	/*
	 * The first command-line argument is the Brainfuck
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
	
	size_t program_size = stat.st_size;
	
	/*
	 * We mmap the entire source file.
	 */
	uint8_t *program = (uint8_t *) mmap(NULL, program_size, PROT_READ,
	    MAP_PRIVATE, source, 0);
	if (program == MAP_FAILED) {
		fprintf(stderr, "%s: Unable to mmap\n", source_name);
		close(source);
		return 4;
	}
	
	size_t ip = 0;
	
	while (ip < program_size) {
		/*
		 * Brainfuck instruction fetch and decode.
		 */
		uint8_t brainfuck_opcode = program[ip];
		instruction_t instruction =
		    brainfuck_opcode_decode(brainfuck_opcode);
		
		/*
		 * Ichiglyph instruction encode.
		 */
		switch (instruction) {
		case INST_DP_INC:
			fputs("ll", stdout);
			break;
		case INST_DP_DEC:
			fputs("lI", stdout);
			break;
		case INST_VAL_INC:
			fputs("Il", stdout);
			break;
		case INST_VAL_DEC:
			fputs("II", stdout);
			break;
		case INST_VAL_OUTPUT:
			fputs("1l", stdout);
			break;
		case INST_VAL_ACCEPT:
			fputs("1I", stdout);
			break;
		case INST_JMP_FORWARD:
			fputs("l1", stdout);
			break;
		case INST_JMP_BACK:
			fputs("I1", stdout);
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
