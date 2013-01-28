// Commands.cpp : implementation file
//

#include "stdafx.h"
#include "msVim.h"
#include "Commands.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int MAX_CLASSNAME_LENGTH = 30;

int nSwitchTextWndProc = 2;
static LPTSTR g_child_classname = NULL;
// map<vim_wnd, child_wnd>
typedef std::map<HWND, HWND> MDI_CHILDS;
MDI_CHILDS g_childs;
MDI_CHILDS::iterator g_child_iter = NULL;

HHOOK g_hChildHook = 0;

WNDPROC prevTextWndProc = 0;

LRESULT CALLBACK CallKeyProc(
  int nCode,      // hook code
  WPARAM wParam,  // current-process flag
  LPARAM lParam   // message data
) {
	if (nCode == HC_ACTION) {
		int nVK = wParam;
		int nInfo = lParam;

		HWND hFocus = ::GetFocus();
		g_child_iter = g_childs.find( hFocus );
		if (g_child_iter == g_childs.end())
			return ::CallNextHookEx(g_hChildHook, nCode, wParam, lParam);

		// handle WM_CHAR message
		//return 0;
		return ::CallNextHookEx(g_hChildHook, nCode, wParam, lParam);
	} else if (nCode < 0) {
		return ::CallNextHookEx(g_hChildHook, nCode, wParam, lParam);
	}
}

LRESULT MDITextWndHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CHAR:
		{
		}
		break;
	}

	return ::CallWindowProc(prevTextWndProc, hWnd, msg, wParam, lParam);
}

LRESULT MDIClientHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hMDIClient = CCommands::MDIClientWnd();
	LRESULT lres = ::CallWindowProc(CCommands::m_prevMDIClientWndProc, hWnd, msg, wParam, lParam);

	if (nSwitchTextWndProc == 1)
	{
		nSwitchTextWndProc = 0;

		HWND hVimWnd = g_childs.begin()->first;
		//prevTextWndProc = (WNDPROC)::SetClassLong(hVimWnd, GCL_WNDPROC, (LONG)MDITextWndHook);

		DWORD dwChildThreadId = ::GetWindowThreadProcessId(hVimWnd, NULL);
		g_hChildHook = ::SetWindowsHookEx(WH_KEYBOARD, CallKeyProc, NULL, dwChildThreadId);
		if (g_hChildHook == NULL)
		{
			OutputDebugString(_T("Hook MDI Child KeyProc failed!"));
			return lres;
		}
		
		g_child_iter = g_childs.begin();
		for (; g_child_iter != g_childs.end(); g_child_iter++)
		{
			//::SetWindowLong(g_child_iter->second, GWL_WNDPROC, (LONG)MDITextWndHook);
		}
	}

	if (hWnd == hMDIClient)
	{
		switch (msg)
		{
		case WM_MDICREATE:
			{
				HWND hVimWnd = NULL;
				HWND hChild = ::FindWindowEx(hMDIClient, NULL, NULL, NULL);
				while (hChild)
				{
					hVimWnd = ::FindWindowEx(hChild, NULL, _T("AfxMDIFrame42"), NULL);
					if (hVimWnd)
					{
						hVimWnd = ::FindWindowEx(hVimWnd, NULL, _T("Afx:400000:8"), NULL);
					}
					g_childs [ hVimWnd ] = hChild;
					hChild = ::FindWindowEx(hMDIClient, hChild, NULL, NULL);
				}
				
				if (nSwitchTextWndProc == 2)
				{
					if (!g_childs.empty())
					{
						nSwitchTextWndProc = 1;
					}
				}
			}
			break;

		case WM_MDIDESTROY:
			break;
		}
	}

	return lres;
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
}

void CCommands::SetApplicationObject(IApplication* pApplication)
{
	// This function assumes pApplication has already been AddRef'd
	//  for us, which CDSAddIn did in its QueryInterface call
	//  just before it called us.
	m_pApplication = pApplication;

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
	return S_OK;
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

	CWnd* pCurrWnd = /*FindCurrEditorWnd()*/FindCurrVimWnd();
	if (pCurrWnd == NULL)
		return S_FALSE;

	Caret(pCurrWnd->m_hWnd, 0);

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

CWnd* CCommands::FindCurrEditorWnd()
{
	CComPtr<ITextWindow> editor_window;
	if (FAILED(m_pApplication->get_ActiveWindow((IDispatch**)&editor_window)) || editor_window == NULL)
		return NULL;
	
	CComBSTR bstrCaption;
	if (FAILED(editor_window->get_Caption(&bstrCaption)))
		return NULL;
	CString csCurrCaption(bstrCaption.m_str);

	CWnd* pMainFrm = AfxGetMainWnd();
	if (pMainFrm == NULL)
		return NULL;
	
	HWND hMDIClient = MDIClientWnd();
	if (hMDIClient == NULL)
		return NULL;
	
	CString csTitle;

	BOOL bFindCurr = FALSE;
	HWND hChild = ::FindWindowEx(hMDIClient, NULL, NULL, NULL);
	while (hChild)
	{
		csTitle.Empty();
		
		int nLen = ::GetWindowTextLength(hChild);
		::GetWindowText(hChild, csTitle.GetBufferSetLength(nLen), nLen+1);
		csTitle.ReleaseBuffer();
		
		if (csTitle == csCurrCaption) {
			bFindCurr = TRUE;
			break;
		}
		hChild = ::FindWindowEx(hMDIClient, hChild, NULL, NULL);
	}
	
	if (!bFindCurr)
		return NULL;

	return pMainFrm->FromHandle(hChild);
}

CWnd* CCommands::FindCurrEditorClient()
{
	CWnd* pCurr = FindCurrEditorWnd();
	if (pCurr == NULL)
		return NULL;
	
	HWND hChild = ::FindWindowEx(pCurr->m_hWnd, NULL, "AfxMDIFrame42", NULL);
	if (hChild != NULL)
		return pCurr->FromHandle(hChild);

	return NULL;
}

CWnd* CCommands::FindCurrVimWnd()
{
	CWnd* pCurr = FindCurrEditorClient();
	if (pCurr == NULL)
		return NULL;

	HWND hChild = ::FindWindowEx(pCurr->m_hWnd, NULL, "Afx:400000:8", NULL);
	if (hChild != NULL)
		return pCurr->FromHandle(hChild);

	return NULL;
}

void CCommands::Caret( HWND hWnd, int caret )
{
	CComPtr<ITextEditor> editor;
	if (FAILED(m_pApplication->get_TextEditor((IDispatch**)&editor)))
		return;

	if (caret == 1) {
		::DestroyCaret();
		::CreateCaret(hWnd, (HBITMAP)1, 0, 0);
		ShowCaret(hWnd);
		return;
	}

	CWnd* pWnd = /*FindCurrEditorWnd()*/FindCurrVimWnd();
	if (pWnd != NULL)
	{
		CDC* pDC = pWnd->GetDC();

		CSize fontSize = pDC->GetTextExtent("1");
		::DestroyCaret();
		::CreateCaret(hWnd, (HBITMAP)caret, fontSize.cx, fontSize.cy);
		ShowCaret(hWnd);

		pWnd->ReleaseDC(pDC);
	}
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
