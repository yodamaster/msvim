#include "StdAfx.h"
#include "vim.h"

VIM_MODE g_init_inputmode = vm_insert;
KEY_MAPPING_TABLE g_key_mapping_table[vm_mode_num];

HRESULT LoadVimCfg()
{
	KEY_MAPPING& cmd_map_ref			= g_key_mapping_table[vm_command].mapping;
	KEY_MAPPING& cmdline_map_ref		= g_key_mapping_table[vm_command_line].mapping;
	KEY_MAPPING& insert_map_ref			= g_key_mapping_table[vm_insert].mapping;

	cmd_map_ref [ _T("f") ]				= _T("find right in line");
	cmd_map_ref [ _T("F") ]				= _T("find left in line");
	cmd_map_ref [ _T("dd") ]			= _T("delete one line");
	cmd_map_ref [ _T("i") ]				= _T("enter insert mode before here");
	cmd_map_ref [ _T("I") ]				= _T("enter insert mode line head");
	cmd_map_ref [ _T("a") ]				= _T("enter insert mode after here");
	cmd_map_ref [ _T("A") ]				= _T("enter insert mode line tail");

	insert_map_ref [ _T("ESC") ]		= _T("enter command mode here");

	return S_OK;
}

LPCTSTR QueryAction(VIM_MODE mode, LPCTSTR key)
{
	KEY_MAPPING::iterator iter = g_key_mapping_table [mode] .mapping .find ( key );
	if (iter != g_key_mapping_table [mode] .mapping .end())
		return iter->second;
	return NULL;
}
