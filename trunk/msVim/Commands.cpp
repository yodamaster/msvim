// Commands.cpp : implementation file
//

#include "stdafx.h"
#include "msVim.h"
#include "Commands.h"
#include "vim/vim.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

MDI_CHILDS g_childs;
HWND g_focus_wnd = NULL;
HHOOK g_keybd_hook = NULL;
BOOL g_vim_good = TRUE;

LRESULT MDITextWnd(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MDI_CHILDS::iterator iter = g_childs.find( hWnd );
	if (iter == g_childs.end()) {
		return -1;
	}

	switch (msg)
	{
	case WM_CHAR:
		{
			VimInterpreter(hWnd, WM_CHAR, wParam, lParam, &iter->second);
			return 0;
		}
		break;

	case WM_KILLFOCUS:
		{
			g_focus_wnd = NULL;
		}
		break;

	case WM_SETFOCUS:
		{
			::CallWindowProc(iter->second.prev_wndproc, hWnd, msg, wParam, lParam);
			g_focus_wnd = hWnd;	// iter->first;
			/* show caret */
			if (iter->second.input_mode == vm_command) {
				Caret(hWnd, &iter->second);
			}
			return 0;
		}
		break;

	case WM_ESC:
		{
			VimInterpreter(hWnd, WM_ESC, wParam, lParam, &iter->second);
			return 0;
		}

	case WM_ENTER:
		{
			VimInterpreter(hWnd, WM_CHAR, VK_RETURN, lParam, &iter->second);
			return 1;
		}

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_ARROWKEY:
		{
			LRESULT lr = ::CallWindowProc(iter->second.prev_wndproc, hWnd, msg, wParam, lParam);
			VimInterpreter(hWnd, WM_ARROWKEY, wParam, lParam, &iter->second);
			return lr;
		}

	default:
		break;
	}

	return ::CallWindowProc(iter->second.prev_wndproc, hWnd, msg, wParam, lParam);
}

LRESULT MDIClientHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	LRESULT lres = ::CallWindowProc(CCommands::m_prevMDIClientWndProc, hWnd, msg, wParam, lParam);

	switch (msg) {
	case WM_MDIDESTROY:
		{
			MDI_CHILDS::iterator iter = g_childs.begin();
			for (; iter != g_childs.end(); iter++) {
				if (iter->second.mdiChild == (HWND)wParam) {
					g_childs.erase(iter);
					break;
				}
			}
		}
		break;
	}

	return lres;
}

LRESULT CALLBACK KeyboardProc(
							  int code,
							  WPARAM wParam,
							  LPARAM lParam
							  ) {
	if (code < 0) {
		return ::CallNextHookEx(g_keybd_hook, code, wParam, lParam);
	}
	
	if (g_focus_wnd != NULL) {
		switch (wParam) {
		case VK_ESCAPE:
			{
				WORD hi=HIWORD(lParam);
				if ((hi & KF_UP) == 0 && 
					(hi & KF_REPEAT) == 0) {
					::CallNextHookEx(g_keybd_hook, code, wParam, lParam);
					return ::SendMessage(g_focus_wnd, WM_ESC, 0, 0);
				}
			}
			break;

		case VK_RETURN:
			{
				WORD hi=HIWORD(lParam);
				if ((hi & KF_UP) == 0 &&
					(hi & KF_ALTDOWN) == 0) {
					::CallNextHookEx(g_keybd_hook, code, wParam, lParam);
					return ::SendMessage(g_focus_wnd, WM_ENTER, 0, 0);
				}
			}
			break;

		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			{
				::CallNextHookEx(g_keybd_hook, code, wParam, lParam);
				return ::SendMessage(g_focus_wnd, WM_ARROWKEY, wParam, lParam);
			}
			break;

		default:
			break;
		}
	}

	return ::CallNextHookEx(g_keybd_hook, code, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CCommands
HWND CCommands::m_hMDIClientWnd = NULL;
WNDPROC CCommands::m_prevMDIClientWndProc = NULL;

CCommands::CCommands()
{
	m_pApplication = NULL;
	m_pApplicationEventsObj = NULL;
	m_pDebuggerEventsObj = NULL;
}

CCommands::~CCommands()
{
	ASSERT (m_pApplication != NULL);
	m_pApplication->Release();
	if (g_keybd_hook != NULL) {
		::UnhookWindowsHookEx(g_keybd_hook);
		g_keybd_hook = NULL;
	}
}

void CCommands::SetApplicationObject(IApplication* pApplication)
{
	// This function assumes pApplication has already been AddRef'd
	//  for us, which CDSAddIn did in its QueryInterface call
	//  just before it called us.
	m_pApplication = pApplication;

	// initialization work
	VimStart();

	// Create Application event handlers
	XApplicationEventsObj::CreateInstance(&m_pApplicationEventsObj);
	m_pApplicationEventsObj->AddRef();
	m_pApplicationEventsObj->Connect(m_pApplication);
	m_pApplicationEventsObj->m_pCommands = this;

	// Create Debugger event handler
	CComPtr<IDispatch> pDebugger;
	if (SUCCEEDED(m_pApplication->get_Debugger(&pDebugger)) 
		&& pDebugger != NULL)
	{
		XDebuggerEventsObj::CreateInstance(&m_pDebuggerEventsObj);
		m_pDebuggerEventsObj->AddRef();
		m_pDebuggerEventsObj->Connect(pDebugger);
		m_pDebuggerEventsObj->m_pCommands = this;
	}
}

void CCommands::UnadviseFromEvents()
{
	ASSERT (m_pApplicationEventsObj != NULL);
	m_pApplicationEventsObj->Disconnect(m_pApplication);
	m_pApplicationEventsObj->Release();
	m_pApplicationEventsObj = NULL;

	if (m_pDebuggerEventsObj != NULL)
	{
		// Since we were able to connect to the Debugger events, we
		//  should be able to access the Debugger object again to
		//  unadvise from its events (thus the VERIFY_OK below--see stdafx.h).
		CComPtr<IDispatch> pDebugger;
		VERIFY_OK(m_pApplication->get_Debugger(&pDebugger));
		ASSERT (pDebugger != NULL);
		m_pDebuggerEventsObj->Disconnect(pDebugger);
		m_pDebuggerEventsObj->Release();
		m_pDebuggerEventsObj = NULL;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Event handlers

// TODO: Fill out the implementation for those events you wish handle
//  Use m_pCommands->GetApplicationObject() to access the Developer
//  Studio Application object

// Application events

HRESULT CCommands::XApplicationEvents::BeforeBuildStart()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::BuildFinish(long nNumErrors, long nNumWarnings)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::BeforeApplicationShutDown()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// 在这儿还原对MDIClient的Subclassing
	HRESULT hr = S_OK;
	/*if (m_prevMDIClientWndProc != NULL) {
		(WNDPROC)::SetWindowLong(hMDIClient, GWL_WNDPROC, (LONG)m_prevMDIClientWndProc);
	}*/
	return hr;
}

HRESULT CCommands::XApplicationEvents::DocumentOpen(IDispatch* theDocument)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::BeforeDocumentClose(IDispatch* theDocument)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::DocumentSave(IDispatch* theDocument)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::NewDocument(IDispatch* theDocument)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WindowActivate(IDispatch* theWindow)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (m_prevMDIClientWndProc == NULL) {
		if (FAILED(CCommands::SubclassingMDIClient())) {
			return E_FAIL;
		}
	}

	CComPtr<ITextWindow> textWnd;
	if (FAILED(theWindow->QueryInterface(IID_ITextWindow, (void**)&textWnd)) || textWnd == NULL) {
		return S_OK;
	}
	if (g_vim_good) {
		HWND hMDIChildWnd = CCommands::FindActiveMDIChildWnd();
		HWND hWnd = CCommands::FindVimWnd(hMDIChildWnd);
		g_focus_wnd = hWnd;
		if (g_keybd_hook == NULL) {
			g_keybd_hook = ::SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, NULL, ::GetWindowThreadProcessId(hWnd, NULL));
			if (g_keybd_hook == NULL) {
				g_vim_good = FALSE;
				return S_OK;
			}
		}
		MDI_CHILDS::iterator iter = g_childs.find( hWnd );
		if (iter == g_childs.end()) {
			VIMProp prop;
			prop.mdiChild = hMDIChildWnd;
			prop.pDoc = textWnd;
			::GetCaretPos(&prop.caret_pos);
			prop.caret_start_x = prop.caret_pos.x;
			prop.input_mode = g_init_inputmode;
			prop.prev_wndproc = (WNDPROC)::SetWindowLong(hWnd, GWL_WNDPROC, (LONG)MDITextWnd);
			g_childs[ hWnd ] = prop;

			Caret(g_focus_wnd, &prop);
		}
		else {
			Caret(g_focus_wnd, &iter->second);
		}
	}

	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WindowDeactivate(IDispatch* theWindow)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WorkspaceOpen()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WorkspaceClose()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::NewWorkspace()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}

// Debugger event

HRESULT CCommands::XDebuggerEvents::BreakpointHit(IDispatch* pBreakpoint)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CCommands methods

STDMETHODIMP CCommands::MsVimCommandMethod() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: Replace this with the actual code to execute this command
	//  Use m_pApplication to access the Developer Studio Application object,
	//  and VERIFY_OK to see error strings in DEBUG builds of your add-in
	//  (see stdafx.h)

	CComPtr<ITextWindow> editor_window;
	if (FAILED(m_pApplication->get_ActiveWindow((IDispatch**)&editor_window)) || editor_window == NULL)
		return E_FAIL;

	CComPtr<ITextSelection> editor_sel;
	if (FAILED(editor_window->get_Selection((IDispatch**)&editor_sel)) || editor_sel == NULL)
		return S_FALSE;

	POINT pos;
	if (FAILED(editor_sel->get_CurrentLine(&pos.x)))
		return E_FAIL;
	if (FAILED(editor_sel->get_CurrentColumn(&pos.y)))
		return E_FAIL;


	HWND test = ::GetFocus();
	Caret(test, 0);

	VERIFY_OK(m_pApplication->EnableModeless(VARIANT_FALSE));
	//::MessageBox(NULL, csPos, "MsVim", MB_OK | MB_ICONINFORMATION);
	//::MessageBox(NULL, (CString)docPath.m_str, "MsVim", MB_OK | MB_ICONINFORMATION);
	//::MessageBox(NULL, "MsVim Command invoked.", "MsVim", MB_OK | MB_ICONINFORMATION);
	VERIFY_OK(m_pApplication->EnableModeless(VARIANT_TRUE));
	return S_OK;
}

void CCommands::DebugStr( LPCSTR lpStr )
{
	if (m_pApplication != NULL) {
		CComBSTR bstrStr(lpStr);
		m_pApplication->PrintToOutputWindow(bstrStr);
	}
}

// 找到指定MDIChild窗口中的编辑窗口
HWND CCommands::FindVimWnd(HWND hMDIChild)
{
	HWND hChild = ::FindWindowEx(hMDIChild, NULL, "AfxMDIFrame42", NULL);
	if (hChild != NULL) {
		return ::FindWindowEx(hChild, NULL, "Afx:400000:8", NULL);
	}
	return NULL;
}

// 找到当前活动MDIChild窗口中的编辑窗口
HWND CCommands::FindActiveVimWnd() {
	HWND hMDIChild = (HWND)::SendMessage(MDIClientWnd(), WM_MDIGETACTIVE, 0, 0);
	return FindVimWnd(hMDIChild);
}

// 找到当前活动MDIChild窗口
HWND CCommands::FindActiveMDIChildWnd() {
	return (HWND)::SendMessage(MDIClientWnd(), WM_MDIGETACTIVE, 0, 0);
}

STDMETHODIMP CCommands::HookMDIClient( )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HWND hMDIClient = MDIClientWnd();
	if (hMDIClient == NULL)
		return E_FAIL;

	m_prevMDIClientWndProc = (WNDPROC)::SetWindowLong(hMDIClient, GWL_WNDPROC, (LONG)MDIClientHook);

	return S_OK;
}

// 获取MDIClient窗口，而不是MDIFrame窗口；这两个窗口均只有一个。
HWND CCommands::MDIClientWnd()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (m_hMDIClientWnd == NULL) {
		CWnd* pMainFrm = AfxGetMainWnd();
		if (pMainFrm == NULL)
			return NULL;
		
		m_hMDIClientWnd = ::FindWindowEx(pMainFrm->m_hWnd, NULL, "MDIClient", NULL);
		return m_hMDIClientWnd;
	}

	return m_hMDIClientWnd;
}

HRESULT CCommands::SubclassingMDIClient() {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HWND hMDIClient = MDIClientWnd();
	if (hMDIClient == NULL) {
		return E_FAIL;
	}

	m_prevMDIClientWndProc = (WNDPROC)::SetWindowLong(hMDIClient, GWL_WNDPROC, (LONG)MDIClientHook);
	if (m_prevMDIClientWndProc != NULL) {
		return S_OK;
	}
	else {
		return E_FAIL;
	}
}