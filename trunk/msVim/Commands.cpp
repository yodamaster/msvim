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

/////////////////////////////////////////////////////////////////////////////
// CCommands

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

	CWnd* pCurrWnd = FindCurrEditorWnd();
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
	
	HWND hMDIClient = ::FindWindowEx(pMainFrm->m_hWnd, NULL, "MDIClient", NULL);
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

	CWnd* pWnd = FindCurrEditorWnd();
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

	m_prevMDIClientWndProc = (WNDPROC)::GetWindowLong(hMDIClient, GWL_WNDPROC);

	return S_OK;
}

HWND CCommands::MDIClientWnd()
{
	CWnd* pMainFrm = AfxGetMainWnd();
	if (pMainFrm == NULL)
		return NULL;
	
	HWND hMDIClient = ::FindWindowEx(pMainFrm->m_hWnd, NULL, "MDIClient", NULL);
	return hMDIClient;
}
