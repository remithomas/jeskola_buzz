#include "stdafx.h"
#include "App.h"
#include "EditorWnd.h"
#include "PatEd.h"
#include "ImageList.h"

void CMachineDataOutput::Write(void *pbuf, int const numbytes) {}
void CMachineDataInput::Read(void *pbuf, int const numbytes) {}


IMPLEMENT_DYNAMIC(CEditorWnd, CWnd)

CEditorWnd::CEditorWnd()
{
	pPattern = NULL;

	MidiEditMode = false;
}

CEditorWnd::~CEditorWnd()
{
}


BEGIN_MESSAGE_MAP(CEditorWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_COLUMNS_BUTTON, OnColumns)
	ON_COMMAND(ID_SELECT_FONT, OnSelectFont)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateClipboard)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateClipboard)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateClipboard)
	ON_BN_CLICKED(IDC_COLUMNS_BUTTON, &CEditorWnd::OnBnClickedColumnsButton)
	ON_BN_CLICKED(IDC_FONT_BUTTON, &CEditorWnd::OnBnClickedFontButton)
	ON_BN_CLICKED(IDC_MIDI_EDIT, &CEditorWnd::OnBnClickedMidiEdit)
END_MESSAGE_MAP()



void CEditorWnd::Create(HWND parent)
{
	CWnd::Create(NULL, "editor", WS_CHILD | WS_VISIBLE, CRect(0, 0, 100, 100), CWnd::FromHandle(parent), 0);
}

void CEditorWnd::SetPattern(CMachinePattern *p)
{
	if (p == pPattern)
		return;

	pPattern = p;
	pe.PatternChanged();
	UpdateCanvasSize();

}

void CEditorWnd::SetPatternLength(CMachinePattern *p, int length)
{
	p->SetLength(length, pCB);

	if (p == pPattern)
	{
		pe.PatternChanged();
		UpdateCanvasSize();
	}
}

void CEditorWnd::SetPlayPos(MapIntToPlayingPattern &pp, CMasterInfo *pmi)
{
	pe.SetPlayPos(pp, pmi);
}

void CEditorWnd::UpdateCanvasSize()
{
	if (pPattern == NULL)
		return;

	int height = pPattern->GetRowCount();

	int width = -1;
	for (ColumnVector::iterator i = pPattern->columns.begin(); i != pPattern->columns.end(); i++)
		width += (*i)->GetWidth();
	
	pe.SetCanvasSize(CSize(width, height));
	leftwnd.SetCanvasSize(CSize(5, height));
	topwnd.SetCanvasSize(CSize(width, 2));

}



int CEditorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	byte *data;
	int nbytes;
	pCB->GetProfileBinary("Font", &data, &nbytes);
	if (nbytes == sizeof(LOGFONT))
	{
		font.CreateFontIndirect((LOGFONT *)data);
		pCB->FreeProfileBinary(data);
	}
	else
	{
		font.CreatePointFont(90, "Fixedsys");
	}


//	reBar.Create(this, 0);
//	toolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	/*
	toolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	toolBar.LoadToolBar(IDR_TOOLBAR);

	CImageList il;
	CreateImageList(il, MAKEINTRESOURCE(IDB_TOOLBAR24), 16, 4, RGB(255,0,255));
	toolBar.SendMessage(TB_SETIMAGELIST, 0, (LPARAM)il.m_hImageList);
	il.Detach();
*/

	dlgBar.Create(this, IDD_DIALOGBAR, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, AFX_IDW_TOOLBAR);
	








	pe.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, 1, NULL);
	pe.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);
	pe.AlwaysShowVerticalScrollBar(true);
	pe.AlwaysShowHorizontalScrollBar(true);
	pe.pew = this;
	pe.pCB = pCB;

	topwnd.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, 1, NULL);
	topwnd.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);
	topwnd.AlwaysShowVerticalScrollBar(true);
	topwnd.HideHorizontalScrollBar(true);
	topwnd.pew = this;

	leftwnd.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, 1, NULL);
	leftwnd.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);
	leftwnd.AlwaysShowHorizontalScrollBar(true);
	leftwnd.HideVerticalScrollBar(true);
	leftwnd.pew = this;

	topleftwnd.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, 1, NULL);
	topleftwnd.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);
	topleftwnd.pCB = pCB;

	pe.linkVert = &leftwnd;
	pe.linkHorz = &topwnd;

	FontChanged();

	return 0;
}

void CEditorWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	UpdateWindowSizes();


}

void CEditorWnd::UpdateWindowSizes()
{	
	CRect cr;
//	GetClientRect(&cr);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, cr);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	int cx = cr.Width();
	int cy = cr.Height();

	int top = cr.top + topWndHeight;
	int left = rowNumWndWidth;

	topwnd.MoveWindow(left, cr.top, cr.right - left, top - cr.top);
	leftwnd.MoveWindow(0, top, left, cr.bottom - top);
	topleftwnd.MoveWindow(0, cr.top, left, top - cr.top);
	pe.MoveWindow(left, top, cr.right - left, cr.bottom - top);
}

void CEditorWnd::FontChanged()
{
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	dc.SelectObject(&font);
	fontSize = dc.GetOutputTextExtent("A");
	rowNumWndWidth = fontSize.cx * 5 + 4;
	topWndHeight = 2 * fontSize.cy + 4;


	pe.SetLineSize(fontSize);
	leftwnd.SetLineSize(fontSize);
	topwnd.SetLineSize(fontSize);
	UpdateCanvasSize();
	UpdateWindowSizes();
	Invalidate();
}


void CEditorWnd::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	pe.SetFocus();
}


void CEditorWnd::OnSelectFont()
{
	LOGFONT lf;
	font.GetLogFont(&lf);

	CFontDialog dlg(&lf, CF_FIXEDPITCHONLY | CF_SCREENFONTS);
	if (dlg.DoModal() != IDOK)
		return;

	dlg.GetCurrentFont(&lf);

	pCB->WriteProfileBinary("Font", (byte *)&lf, sizeof(LOGFONT));

	font.DeleteObject();
	font.CreateFontIndirect(&lf);
	
	FontChanged();
}

void CEditorWnd::AddTrack(int n)
{
	if (pPattern == NULL || (int)pPattern->columns.size() == 0)
		return;

	CMachine *pmac = pe.GetCursorColumn()->GetMachine();
	CMachineInfo const *pmi = pCB->GetMachineInfo(pmac);

	if (pPattern->GetTrackCount(pmac) >= pmi->maxTracks)
		return;

	pPattern->actions.BeginAction(this, "Add Track");
	{
		MACHINE_LOCK;

		for (int i = 0; i < n; i++)
		{
			if (pPattern->GetTrackCount(pmac) >= pmi->maxTracks) break;
			pPattern->AddTrack(pmac, pCB);
		}
	}

	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();
}

void CEditorWnd::DeleteLastTrack()
{
	if (pPattern == NULL || (int)pPattern->columns.size() == 0)
		return;

	CMachine *pmac = pe.GetCursorColumn()->GetMachine();
	CMachineInfo const *pmi = pCB->GetMachineInfo(pmac);

	if (pPattern->GetTrackCount(pmac) <= pmi->minTracks)
		return;

	pPattern->actions.BeginAction(this, "Delete Last Track");
	{
		MACHINE_LOCK;
		pPattern->DeleteLastTrack(pe.GetCursorColumn()->GetMachine(), pCB);
	}

	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();

}

void CEditorWnd::OnColumns()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (pPattern == NULL)
		return;

	CColumnDialog dlg(this);
	dlg.pew = this;
	dlg.m_NameValue = pCB->GetPatternName(pPattern->pPattern);
	dlg.m_LengthValue = pPattern->numBeats;
	dlg.m_RPBValue = pPattern->rowsPerBeat;

	if (dlg.DoModal() != IDOK)
		return;

	pPattern->actions.BeginAction(this, "Change Properties");
	{
		pPattern->EnableColumns(dlg.enabledColumns, pCB, 1, dlg.m_RPBValue);
	}

	pCB->SetPatternName(pPattern->pPattern, dlg.m_NameValue);
	pCB->SetPatternLength(pPattern->pPattern, dlg.m_LengthValue * BUZZ_TICKS_PER_BEAT);


	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();
}

void CEditorWnd::OnEditUndo()
{
	pPattern->actions.Undo(this);

	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();
}

void CEditorWnd::OnEditRedo()
{
	pPattern->actions.Redo(this);

	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();
}

void CEditorWnd::OnUpdateEditUndo(CCmdUI *pCmdUI) { pCmdUI->Enable(pPattern != NULL && pPattern->actions.CanUndo()); }
void CEditorWnd::OnUpdateEditRedo(CCmdUI *pCmdUI) { pCmdUI->Enable(pPattern != NULL && pPattern->actions.CanRedo()); }

bool CEditorWnd::EnableCommandUI(int id)
{
	switch(id)
	{
	case ID_EDIT_CUT: return pe.CanCut();
	case ID_EDIT_COPY: return pe.CanCopy();
	case ID_EDIT_PASTE: return pe.CanPaste();
	case ID_EDIT_UNDO: return pPattern != NULL && pPattern->actions.CanUndo();
	case ID_EDIT_REDO: return pPattern != NULL && pPattern->actions.CanRedo();
	}

	return false;
}

void CEditorWnd::OnUpdateClipboard(CCmdUI *pCmdUI) { pCmdUI->Enable(EnableCommandUI(pCmdUI->m_nID)); }

void CEditorWnd::OnEditCut() { pe.OnEditCut(); }
void CEditorWnd::OnEditCopy() { pe.OnEditCopy(); }
void CEditorWnd::OnEditPaste() { pe.OnEditPaste(); }


void CEditorWnd::OnBnClickedColumnsButton()
{
	OnColumns();
}

void CEditorWnd::OnBnClickedFontButton()
{
	OnSelectFont();
}

void CEditorWnd::OnBnClickedMidiEdit()
{
	CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_MIDI_EDIT);
	MidiEditMode = pc->GetCheck() == BST_CHECKED;
}


int CEditorWnd::GetEditorPatternPosition()
{
	if (pPattern == NULL)
		return 0;
	
	return pe.cursor.row * 4 / pPattern->rowsPerBeat;
}
