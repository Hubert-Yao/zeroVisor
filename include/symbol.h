// Dynamically added from /proc/kallsyms
#define KALLSYMS_LOOKUP_NAME_ADDR 0xffffffffb6966840
char* g_kernel_version[] = 
{
	"#1 SMP Wed Jul 23 01:35:44 CST 2025",
};

struct zv_symbol_table g_symbol_table_array[] =
{
	//#1 SMP Wed Jul 23 01:35:44 CST 2025
	{
		{
			{"_text", 0xffffffff81000000},
			{"walk_system_ram_range", 0xffffffff810a8be0},
			{"wake_up_new_task", 0xffffffff810e1c60},
			{"free_module", 0xffffffff81162600},
			{"ftrace_module_init", 0xffffffff811b1970},
			{"_etext", 0xffffffff82002a72},
			{"__start_rodata", 0xffffffff82200000},
			{"__start___ex_table", 0xffffffff82737dd0},
			{"__stop___ex_table", 0xffffffff8273981c},
			{"__end_rodata", 0xffffffff8273a000},
			{"tasklist_lock", 0xffffffff82806080},
			{"modules", 0xffffffff829ac560},
			{"init_mm", 0xffffffff829dc520},
		},
	},
};

