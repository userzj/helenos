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

/**
 * This file is meant to contain all wrapper functions for
 * all kconsole commands. The point is in separating
 * kconsole specific wrappers from kconsole-unaware functions
 * from other subsystems.
 */

#include <console/cmd.h>
#include <console/kconsole.h>
#include <print.h>
#include <panic.h>
#include <typedefs.h>
#include <arch/types.h>
#include <list.h>
#include <arch.h>
#include <func.h>
#include <macros.h>
#include <debug.h>
#include <symtab.h>

#include <mm/tlb.h>
#include <arch/mm/tlb.h>
#include <mm/frame.h>

/** Data and methods for 'help' command. */
static int cmd_help(cmd_arg_t *argv);
static cmd_info_t help_info = {
	.name = "help",
	.description = "List of supported commands.",
	.func = cmd_help,
	.argc = 0
};

static cmd_info_t exit_info = {
	.name = "exit",
	.description ="Exit kconsole",
	.argc = 0
};

/** Data and methods for 'description' command. */
static int cmd_desc(cmd_arg_t *argv);
static void desc_help(void);
static char desc_buf[MAX_CMDLINE+1];
static cmd_arg_t desc_argv = {
	.type = ARG_TYPE_STRING,
	.buffer = desc_buf,
	.len = sizeof(desc_buf)
};
static cmd_info_t desc_info = {
	.name = "describe",
	.description = "Describe specified command.",
	.help = desc_help,
	.func = cmd_desc,
	.argc = 1,
	.argv = &desc_argv
};

/** Data and methods for 'symaddr' command. */
static int cmd_symaddr(cmd_arg_t *argv);
static char symaddr_buf[MAX_CMDLINE+1];
static cmd_arg_t symaddr_argv = {
	.type = ARG_TYPE_STRING,
	.buffer = symaddr_buf,
	.len = sizeof(symaddr_buf)
};
static cmd_info_t symaddr_info = {
	.name = "symaddr",
	.description = "Return symbol address.",
	.func = cmd_symaddr,
	.argc = 1,
	.argv = &symaddr_argv
};

static char set_buf[MAX_CMDLINE+1];
static int cmd_set4(cmd_arg_t *argv);
static cmd_arg_t set4_argv[] = {
	{
		.type = ARG_TYPE_STRING,
		.buffer = set_buf,
		.len = sizeof(set_buf)
	},
	{ 
		.type = ARG_TYPE_INT
	}
};
static cmd_info_t set4_info = {
	.name = "set4",
	.description = "set <dest_addr> <value> - 4byte version",
	.func = cmd_set4,
	.argc = 2,
	.argv = set4_argv
};



/** Data and methods for 'call0' command. */
static char call0_buf[MAX_CMDLINE+1];
static char carg1_buf[MAX_CMDLINE+1];
static char carg2_buf[MAX_CMDLINE+1];
static char carg3_buf[MAX_CMDLINE+1];

static int cmd_call0(cmd_arg_t *argv);
static cmd_arg_t call0_argv = {
	.type = ARG_TYPE_STRING,
	.buffer = call0_buf,
	.len = sizeof(call0_buf)
};
static cmd_info_t call0_info = {
	.name = "call0",
	.description = "call0 <function> -> call function().",
	.func = cmd_call0,
	.argc = 1,
	.argv = &call0_argv
};

/** Data and methods for 'call1' command. */
static int cmd_call1(cmd_arg_t *argv);
static cmd_arg_t call1_argv[] = {
	{
		.type = ARG_TYPE_STRING,
		.buffer = call0_buf,
		.len = sizeof(call0_buf)
	},
	{ 
		.type = ARG_TYPE_VAR,
		.buffer = carg1_buf,
		.len = sizeof(carg1_buf)
	}
};
static cmd_info_t call1_info = {
	.name = "call1",
	.description = "call1 <function> <arg1> -> call function(arg1).",
	.func = cmd_call1,
	.argc = 2,
	.argv = call1_argv
};

/** Data and methods for 'call2' command. */
static int cmd_call2(cmd_arg_t *argv);
static cmd_arg_t call2_argv[] = {
	{
		.type = ARG_TYPE_STRING,
		.buffer = call0_buf,
		.len = sizeof(call0_buf)
	},
	{ 
		.type = ARG_TYPE_VAR,
		.buffer = carg1_buf,
		.len = sizeof(carg1_buf)
	},
	{ 
		.type = ARG_TYPE_VAR,
		.buffer = carg2_buf,
		.len = sizeof(carg2_buf)
	}
};
static cmd_info_t call2_info = {
	.name = "call2",
	.description = "call2 <function> <arg1> <arg2> -> call function(arg1,arg2).",
	.func = cmd_call2,
	.argc = 3,
	.argv = call2_argv
};

/** Data and methods for 'call3' command. */
static int cmd_call3(cmd_arg_t *argv);
static cmd_arg_t call3_argv[] = {
	{
		.type = ARG_TYPE_STRING,
		.buffer = call0_buf,
		.len = sizeof(call0_buf)
	},
	{ 
		.type = ARG_TYPE_VAR,
		.buffer = carg1_buf,
		.len = sizeof(carg1_buf)
	},
	{ 
		.type = ARG_TYPE_VAR,
		.buffer = carg2_buf,
		.len = sizeof(carg2_buf)
	},
	{ 
		.type = ARG_TYPE_VAR,
		.buffer = carg3_buf,
		.len = sizeof(carg3_buf)
	}

};
static cmd_info_t call3_info = {
	.name = "call3",
	.description = "call3 <function> <arg1> <arg2> <arg3> -> call function(arg1,arg2,arg3).",
	.func = cmd_call3,
	.argc = 4,
	.argv = call3_argv
};

/** Data and methods for 'halt' command. */
static int cmd_halt(cmd_arg_t *argv);
static cmd_info_t halt_info = {
	.name = "halt",
	.description = "Halt the kernel.",
	.func = cmd_halt,
	.argc = 0
};

/** Data and methods for 'ptlb' command. */
static int cmd_ptlb(cmd_arg_t *argv);
cmd_info_t ptlb_info = {
	.name = "ptlb",
	.description = "Print TLB of current processor.",
	.help = NULL,
	.func = cmd_ptlb,
	.argc = 0,
	.argv = NULL
};


/** Data and methods for 'zones' command */
static int cmd_zones(cmd_arg_t *argv);
static cmd_info_t zones_info = {
	.name = "zones",
	.description = "List of memory zones.",
	.func = cmd_zones,
	.argc = 0
};

/** Data and methods for 'zone' command */
static int cmd_zone(cmd_arg_t *argv);
static char zone_buf[MAX_CMDLINE+1];
static cmd_arg_t zone_argv = {
	.type = ARG_TYPE_INT,
	.buffer = zone_buf,
	.len = sizeof(zone_buf)
};


static cmd_info_t zone_info = {
	.name = "zone",
	.description = "Show memory zone structure.",
	.func = cmd_zone,
	.argc = 1,
	.argv = &zone_argv
};



/** Initialize command info structure.
 *
 * @param cmd Command info structure.
 *
 */
void cmd_initialize(cmd_info_t *cmd)
{
	spinlock_initialize(&cmd->lock, "cmd");
	link_initialize(&cmd->link);
}

/** Initialize and register commands. */
void cmd_init(void)
{
	cmd_initialize(&help_info);
	if (!cmd_register(&help_info))
		panic("could not register command %s\n", help_info.name);

	cmd_initialize(&desc_info);
	if (!cmd_register(&desc_info))
		panic("could not register command %s\n", desc_info.name);

	cmd_initialize(&exit_info);
	if (!cmd_register(&exit_info))
		panic("could not register command %s\n", exit_info.name);
	
	cmd_initialize(&symaddr_info);
	if (!cmd_register(&symaddr_info))
		panic("could not register command %s\n", symaddr_info.name);

	cmd_initialize(&call0_info);
	if (!cmd_register(&call0_info))
		panic("could not register command %s\n", call0_info.name);

	cmd_initialize(&call1_info);
	if (!cmd_register(&call1_info))
		panic("could not register command %s\n", call1_info.name);

	cmd_initialize(&call2_info);
	if (!cmd_register(&call2_info))
		panic("could not register command %s\n", call2_info.name);

	cmd_initialize(&call3_info);
	if (!cmd_register(&call3_info))
		panic("could not register command %s\n", call3_info.name);

	cmd_initialize(&set4_info);
	if (!cmd_register(&set4_info))
		panic("could not register command %s\n", set4_info.name);
	
	cmd_initialize(&halt_info);
	if (!cmd_register(&halt_info))
		panic("could not register command %s\n", halt_info.name);

	cmd_initialize(&ptlb_info);
	if (!cmd_register(&ptlb_info))
		panic("could not register command %s\n", ptlb_info.name);

	cmd_initialize(&zones_info);
	if (!cmd_register(&zones_info))
		panic("could not register command %s\n", zones_info.name);

	cmd_initialize(&zone_info);
	if (!cmd_register(&zone_info))
		panic("could not register command %s\n", zone_info.name);


}


/** List supported commands.
 *
 * @param argv Argument vector.
 *
 * @return 0 on failure, 1 on success.
 */
int cmd_help(cmd_arg_t *argv)
{
	link_t *cur;

	spinlock_lock(&cmd_lock);
	
	for (cur = cmd_head.next; cur != &cmd_head; cur = cur->next) {
		cmd_info_t *hlp;
		
		hlp = list_get_instance(cur, cmd_info_t, link);
		spinlock_lock(&hlp->lock);
		
		printf("%s - %s\n", hlp->name, hlp->description);

		spinlock_unlock(&hlp->lock);
	}
	
	spinlock_unlock(&cmd_lock);	

	return 1;
}

/** Describe specified command.
 *
 * @param argv Argument vector.
 *
 * @return 0 on failure, 1 on success.
 */
int cmd_desc(cmd_arg_t *argv)
{
	link_t *cur;

	spinlock_lock(&cmd_lock);
	
	for (cur = cmd_head.next; cur != &cmd_head; cur = cur->next) {
		cmd_info_t *hlp;
		
		hlp = list_get_instance(cur, cmd_info_t, link);
		spinlock_lock(&hlp->lock);

		if (strncmp(hlp->name, (const char *) argv->buffer, strlen(hlp->name)) == 0) {
			printf("%s - %s\n", hlp->name, hlp->description);
			if (hlp->help)
				hlp->help();
			spinlock_unlock(&hlp->lock);
			break;
		}

		spinlock_unlock(&hlp->lock);
	}
	
	spinlock_unlock(&cmd_lock);	

	return 1;
}

/** Search symbol table */
int cmd_symaddr(cmd_arg_t *argv)
{
	symtab_print_search(argv->buffer);
	
	return 1;
}

/** Call function with zero parameters */
int cmd_call0(cmd_arg_t *argv)
{
	__address symaddr;
	char *symbol;
	__native (*f)(void);

	symaddr = get_symbol_addr(argv->buffer);
	if (!symaddr)
		printf("Symbol %s not found.\n", argv->buffer);
	else if (symaddr == (__address) -1) {
		symtab_print_search(argv->buffer);
		printf("Duplicate symbol, be more specific.\n");
	} else {
		symbol = get_symtab_entry(symaddr);
		printf("Calling f(): 0x%p: %s\n", symaddr, symbol);
		f =  (__native (*)(void)) symaddr;
		printf("Result: 0x%p\n", f());
	}
	
	return 1;
}

/** Call function with one parameter */
int cmd_call1(cmd_arg_t *argv)
{
	__address symaddr;
	char *symbol;
	__native (*f)(__native);
	__native arg1 = argv[1].intval;

	symaddr = get_symbol_addr(argv->buffer);
	if (!symaddr)
		printf("Symbol %s not found.\n", argv->buffer);
	else if (symaddr == (__address) -1) {
		symtab_print_search(argv->buffer);
		printf("Duplicate symbol, be more specific.\n");
	} else {
		symbol = get_symtab_entry(symaddr);
		printf("Calling f(0x%x): 0x%p: %s\n", arg1, symaddr, symbol);
		f =  (__native (*)(__native)) symaddr;
		printf("Result: 0x%p\n", f(arg1));
	}
	
	return 1;
}

/** Call function with two parameters */
int cmd_call2(cmd_arg_t *argv)
{
	__address symaddr;
	char *symbol;
	__native (*f)(__native,__native);
	__native arg1 = argv[1].intval;
	__native arg2 = argv[2].intval;

	symaddr = get_symbol_addr(argv->buffer);
	if (!symaddr)
		printf("Symbol %s not found.\n", argv->buffer);
	else if (symaddr == (__address) -1) {
		symtab_print_search(argv->buffer);
		printf("Duplicate symbol, be more specific.\n");
	} else {
		symbol = get_symtab_entry(symaddr);
		printf("Calling f(0x%x,0x%x): 0x%p: %s\n", 
		       arg1, arg2, symaddr, symbol);
		f =  (__native (*)(__native,__native)) symaddr;
		printf("Result: 0x%p\n", f(arg1, arg2));
	}
	
	return 1;
}

/** Call function with three parameters */
int cmd_call3(cmd_arg_t *argv)
{
	__address symaddr;
	char *symbol;
	__native (*f)(__native,__native,__native);
	__native arg1 = argv[1].intval;
	__native arg2 = argv[2].intval;
	__native arg3 = argv[3].intval;

	symaddr = get_symbol_addr(argv->buffer);
	if (!symaddr)
		printf("Symbol %s not found.\n", argv->buffer);
	else if (symaddr == (__address) -1) {
		symtab_print_search(argv->buffer);
		printf("Duplicate symbol, be more specific.\n");
	} else {
		symbol = get_symtab_entry(symaddr);
		printf("Calling f(0x%x,0x%x, 0x%x): 0x%p: %s\n", 
		       arg1, arg2, arg3, symaddr, symbol);
		f =  (__native (*)(__native,__native,__native)) symaddr;
		printf("Result: 0x%p\n", f(arg1, arg2, arg3));
	}
	
	return 1;
}


/** Print detailed description of 'describe' command. */
void desc_help(void)
{
	printf("Syntax: describe command_name\n");
}

/** Halt the kernel.
 *
 * @param argv Argument vector (ignored).
 *
 * @return 0 on failure, 1 on success (never returns).
 */
int cmd_halt(cmd_arg_t *argv)
{
	halt();
	return 1;
}

/** Command for printing TLB contents.
 *
 * @param argv Not used.
 *
 * @return Always returns 1.
 */
int cmd_ptlb(cmd_arg_t *argv)
{
	tlb_print();
	return 1;
}

/** Write 4 byte value to address */
int cmd_set4(cmd_arg_t *argv)
{
	__u32 *addr ;
	__u32 arg1 = argv[1].intval;
	bool pointer = false;

	if (((char *)argv->buffer)[0] == '*') {
		addr = (__u32 *) get_symbol_addr(argv->buffer+1);
		pointer = true;
	} else if (((char *)argv->buffer)[0] >= '0' && 
		   ((char *)argv->buffer)[0] <= '9')
		addr = (__u32 *)atoi((char *)argv->buffer);
	else
		addr = (__u32 *)get_symbol_addr(argv->buffer);

	if (!addr)
		printf("Symbol %s not found.\n", argv->buffer);
	else if (addr == (__u32 *) -1) {
		symtab_print_search(argv->buffer);
		printf("Duplicate symbol, be more specific.\n");
	} else {
		if (pointer)
			addr = (__u32 *)(*(__native *)addr);
		printf("Writing 0x%x -> 0x%p\n", arg1, addr);
		*addr = arg1;
		
	}
	
	return 1;
}


int cmd_zones(cmd_arg_t * argv) {
	printf("Zones listing not implemented\n");
	return 1;
}
int cmd_zone(cmd_arg_t * argv) {
	printf("Zone details not implemented\n");
	return 1;
}

