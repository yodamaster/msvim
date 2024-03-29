#ifndef VIM_H
#define VIM_h

#include <map>

#define WM_ESC WM_USER+100
#define WM_ENTER WM_ESC+1
/* Params for WM_ARROWKEY are as the same as WM_KEYDOWN */
#define WM_ARROWKEY WM_ENTER+1


typedef enum _VIM_MODE_ {
	vm_insert,
	vm_command,
	vm_command_line,
	vm_visual,
	vm_mode_num
} VIM_MODE;

typedef struct _VIMProp_ {
	HWND mdiChild;
	CComPtr<ITextWindow> pDoc;
	LONG caret_start_x;
	POINT caret_pos;
	VIM_MODE input_mode;
	WNDPROC prev_wndproc;
} VIMProp, *PVIMProp;
// map<vim_wnd, VIMProp>
typedef std::map<HWND, VIMProp> MDI_CHILDS;


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

extern VIM_MODE g_init_inputmode;
extern KEY_MAPPING_TABLE g_key_mapping_table[vm_mode_num];

// vim_library_cfg
HRESULT LoadVimCfg();
LPCTSTR QueryAction(VIM_MODE mode, LPCTSTR key);

// four control handler
// framework decide to invoke which handler's BeginActive/Active/EndActive function via
// testing GetToolNum and GetTool functions.

void Caret( HWND hWnd, PVIMProp );

// the user input character-stream interpreter
int VimInterpreter(HWND, UINT, WPARAM, LPARAM, PVIMProp);

void VimStart();

#endif // VIM_H