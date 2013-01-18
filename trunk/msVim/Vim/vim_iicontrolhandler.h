#ifndef VIM_IICONTROLHANDLER_H
#define VIM_IICONTROLHANDLER_H

// Control handler
//LPCTSTR s_input_modes[] = 
class IIControlHandler
{
public:
	virtual ULONG GetToolNum() = 0;
	// get index of tool (return -1 if not exist)
	virtual int GetTool(LPCTSTR action) = 0;
	virtual HRESULT BeginActive(
		ITextWindow* wnd,
		ITextSelection* sel,
		int tool_num,
		LONG x,
		LONG y) = 0;
	virtual HRESULT Active(
		ITextWindow* wnd,
		ITextSelection* sel,
		int tool_num,
		LONG x,
		LONG y) = 0;
	virtual HRESULT EndActive(
		ITextWindow* wnd,
		ITextSelection* sel,
		int tool_num,
		LONG x,
		LONG y) = 0;
};

#endif // VIM_IICONTROLHANDLER_H