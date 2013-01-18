#ifndef VIM_H
#define VIM_h

#include <map>

typedef enum {
	vm_insert,
	vm_command,
	vm_command_line,
	vm_mode_num
} VIM_MODE;

// should use Hash table instead
/*typedef struct {
	LPCTSTR  key,
	LPCTSTR  action
} KEY_MAPPING;*/
class CompareInKeyMapping
{
public:
	bool operator () (LPCTSTR lpStr1, LPCTSTR lpStr2) const {
		if (_tcscmp(lpStr1, lpStr2) <= 0)
			return true;
		return false;
	}
};
typedef std::map<LPCTSTR, LPCTSTR, CompareInKeyMapping> KEY_MAPPING;

typedef struct {
	VIM_MODE mode;
	KEY_MAPPING mapping;
} KEY_MAPPING_TABLE;

extern VIM_MODE g_vim_mode;
extern KEY_MAPPING_TABLE g_key_mapping_table[vm_mode_num];

// vim_library_cfg
HRESULT LoadVimCfg();
LPCTSTR QueryAction(VIM_MODE mode, LPCTSTR key);

// four control handler
// framework decide to invoke which handler's BeginActive/Active/EndActive function via
// testing GetToolNum and GetTool functions.

#endif // VIM_H