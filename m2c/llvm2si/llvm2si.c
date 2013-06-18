/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>
#include <stdio.h>

#include <m2c/si2bin/arg.h>
#include <m2c/si2bin/inst.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/Core.h>

#include "basic-block.h"
#include "function.h"
#include "llvm2si.h"


/*
 * Private Functions
 */

void llvm2si_compile_file(char *source_file, char *output_file)
{
	LLVMMemoryBufferRef llmem_buf;
	LLVMModuleRef llmodule;
	LLVMValueRef llfunction;
	LLVMAttribute llattr;
	LLVMBool err;

	char *message;
	FILE *f;

	struct llvm2si_function_t *function;
	struct llvm2si_basic_block_t *basic_block;

	/* Create memory buffer with source file */
	err = LLVMCreateMemoryBufferWithContentsOfFile(source_file,
			&llmem_buf, &message);
	if (err)
		fatal("%s: %s", source_file, message);

	/* Read module */
	err = LLVMParseBitcode(llmem_buf, &llmodule, &message);
	if (err)
		fatal("%s: %s", source_file, message);

	/* Open output file */
	f = fopen(output_file, "w");
	if (!f)
		fatal("%s: cannot open output file", output_file);

	/* Translate all functions */
	for (llfunction = LLVMGetFirstFunction(llmodule); llfunction;
			llfunction = LLVMGetNextFunction(llfunction))
	{
		/* Ignore built-in functions. Built-in function declarations are
		 * generated by the CL-to-LLVM front-end with the 'nounwind'
		 * attribute. */
		llattr = LLVMGetFunctionAttr(llfunction);
		if (llattr & LLVMNoUnwindAttribute)
			continue;

		/* Create function */
		function = llvm2si_function_create(llfunction);

		/* Emit code for function */
		llvm2si_function_emit_header(function);
		llvm2si_function_emit_args(function);

		/* Create a basic block and generate body in it */
		basic_block = llvm2si_basic_block_create(NULL);
		llvm2si_function_add_basic_block(function, basic_block);

		llvm2si_function_emit_body(function, basic_block);

		/* Dump and free function */
		llvm2si_function_dump(function, f);
		llvm2si_function_free(function);
	}

	/* Close output file */
	fclose(f);

	/* Free module and memory buffer */
	LLVMDisposeModule(llmodule);
	LLVMDisposeMemoryBuffer(llmem_buf);
}




/*
 * Public Functions
 */

void llvm2si_init(void)
{
}


void llvm2si_done(void)
{
}


void llvm2si_compile(struct list_t *source_file_list,
		struct list_t *output_file_list)
{
	char *source_file;
	char *output_file;

	int index;

	LIST_FOR_EACH(source_file_list, index)
	{
		/* Get file names */
		source_file = list_get(source_file_list, index);
		output_file = list_get(output_file_list, index);
		assert(source_file);
		assert(output_file);

		/* Compile one file */
		llvm2si_compile_file(source_file, output_file);
	}
}
