// TopWnd.cpp : implementation file
//

#include "stdafx.h"
#include "TopWnd.h"
#include "EditorWnd.h"

// CTopWnd

IMPLEMENT_DYNAMIC(CTopWnd, CScrollWnd)

CTopWnd::CTopWnd()
{

}

CTopWnd::~CTopWnd()
{
}


BEGIN_MESSAGE_MAP(CTopWnd, CScrollWnd)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


void CTopWnd::OnDraw(CDC *pDC)
{
	CRect clr;
	GetClientRect(&clr);
	CRect cr = GetCanvasRect();

	CRect ur;
	ur.UnionRect(&cr, &clr);

	
	COLORREF bgcolor = pew->pCB->GetThemeColor("PE BG");
	COLORREF textcolor = pew->pCB->GetThemeColor("PE Text");

	pDC->FillSolidRect(&ur, bgcolor);

	CMachinePattern *ppat = pew->pPattern;

	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CObject *pOldFont = pDC->SelectObject(&pew->font);

	CPen pen(PS_SOLID, 1, Blend(textcolor, bgcolor, 0.5f));
	CObject *pOldPen = pDC->SelectObject(&pen);

	CColumn *lastcolumn = ppat->columns[0].get();
	int startx = 0;
	int macstartx = 0;
	int x = 0;

	for (int col = 0; col <= (int)ppat->columns.size(); col++)
	{
		CColumn *pc = NULL;
		if (col < (int)ppat->columns.size())
			pc = ppat->columns[col].get();

		if (pc == NULL || ((x - startx) > 0 && !pc->MatchGroupAndTrack(*lastcolumn)))
		{
			if (lastcolumn->IsTrackParam())
			{
				// draw track number
				if (pew->pPattern->IsTrackMuted(lastcolumn))
					pDC->SetTextColor(Blend(textcolor, bgcolor, 0.5f));
				else
					pDC->SetTextColor(textcolor);

				CRect r;
				r.left = startx;
				r.right = x - pew->fontSize.cx;
				r.top = pew->fontSize.cy;
				r.bottom = r.top + pew->fontSize.cy;

				CString s;
				s.Format("%d", lastcolumn->GetTrack());

				pDC->DrawText(s, &r, DT_CENTER);

			}
 
			startx = x;
		}

		if (pc == NULL || !pc->MatchMachine(*lastcolumn))
		{
			// draw machine name
			pDC->SetTextColor(textcolor);

			CRect r;
			r.left = macstartx;
			r.right = x - pew->fontSize.cx;
			r.top = 0;
			r.bottom = pew->fontSize.cy;
			pDC->DrawText(ppat->columns[col-1]->GetMachineName(), &r, DT_CENTER | DT_WORD_ELLIPSIS);

			if (pc != NULL)
			{
				// draw a vertical separator between machines
				int sx = x - pew->fontSize.cx / 2;
				pDC->MoveTo(sx, 0);
				pDC->LineTo(sx, clr.bottom);
			}

			macstartx = x;
		}

		if (pc == NULL)
			break;

//		x += (pc->GetDigitCount() + 1) * pew->fontSize.cx;
		x += pew->pe.GetColumnWidth(col);
		
		lastcolumn = pc;

	}

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldFont);
}

void CTopWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	point = ClientToCanvas(point);
	if (point.y < pew->fontSize.cy)
	{
		int col = pew->pe.GetColumnAtX(point.x);
		if (col < 0)
			return;

		pew->pCB->ShowMachineWindow(pew->pPattern->columns[col]->GetMachine(), true);
	}
	else
	{
		MuteTrack(point);
	}

	CScrollWnd::OnLButtonDblClk(nFlags, point);
}

void CTopWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	point = ClientToCanvas(point);
	if (point.y < pew->fontSize.cy)
		return;

	MuteTrack(point);

	CScrollWnd::OnLButtonDown(nFlags, point);
}

void CTopWnd::MuteTrack(CPoint point)
{
	int col = pew->pe.GetColumnAtX(point.x);
	if (col < 0)
		return;

	CColumn *pc = pew->pPattern->columns[col].get();
	if (pc->IsTrackParam())
		pew->pPattern->ToggleTrackMute(pc);

	Invalidate();
	pew->pe.Invalidate();

}
