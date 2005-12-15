/*
 * Copyright (C) 2005 Jakub Jermar
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

#include <arch/trap/trap.h>
#include <arch/trap/trap_table.h>
#include <arch/trap/regwin.h>
#include <arch/asm.h>
#include <memstr.h>
#include <debug.h>
#include <arch/types.h>
#include <typedefs.h>

/** Initialize trap table. */
void trap_init(void)
{
	/*
	 * Copy OFW's trap table into kernel.
	 */
	memcpy((void *) trap_table, (void *) tba_read(), TRAP_TABLE_SIZE);
	
	/*
	 * Install kernel-provided handlers.
	 */
	trap_install_handler(TT_CLEAN_WINDOW, CLEAN_WINDOW_HANDLER_SIZE, false);
	trap_install_handler(TT_SPILL_0_NORMAL, SPILL_HANDLER_SIZE, false);
	trap_install_handler(TT_FILL_0_NORMAL, FILL_HANDLER_SIZE, false);
}

/** Copy trap handler to active trap table.
 *
 * The handler is copied from trap_table_kernel to trap_handler.
 *
 * @param tt Trap type. An index that uniquelly identifies handler code.
 * @param len Length of the handler in bytes. Must be multiple of TRAP_TABLE_ENTRY_SIZE (i.e. 32).
 * @param tlnonz If false, tt is considered from the lower half of trap table. If true, the upper half is chosen.
 */
void trap_install_handler(index_t tt, size_t len, bool tlnonz)
{
	count_t cnt;
	int i;

	ASSERT(tt < TRAP_TABLE_ENTRY_COUNT/2);
	ASSERT(len % TRAP_TABLE_ENTRY_SIZE == 0);

	if (tlnonz)
		tt += TRAP_TABLE_ENTRY_COUNT/2;

	cnt = len/TRAP_TABLE_ENTRY_SIZE;
	
	for (i = tt; i < tt + cnt; i++) {
		trap_table[i] = trap_table_kernel[i];
	}	
}
