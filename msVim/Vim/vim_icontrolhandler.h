#ifndef VIM_ICONTROLHANDLER_H
#define VIM_ICONTROLHANDLER_H

#include "vim_iicontrolhandler.h"

class IControlHandler : public IIControlHandler
{
public:
	virtual ~IControlHandler();

public: // IIControlHandler interface
	ULONG GetToolNum();
	// get index of tool (return -1 if not exist)
	int GetTool(LPCTSTR action);
	HRESULT BeginActive(
		ITextWindow* wnd,
		ITextSelection* sel,
		int tool_num,
		LONG x,
		LONG y);
	HRESULT Active(
		ITextWindow* wnd,
		ITextSelection* sel,
		int tool_num,
		LONG x,
		LONG y);
	HRESULT EndActive(
		ITextWindow* wnd,
		ITextSelection* sel,
		int tool_num,
		LONG x,
		LONG y);

protected:
	static LPTSTR s_input[];
	IControlHandler();
};

#endif // VIM_ICONTROLHANDLER_H