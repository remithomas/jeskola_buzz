// ScrollWnd.cpp : implementation file
//

#include "stdafx.h"
#include "SineSynth.h"
#include "ScrollWnd.h"


// CScrollWnd

IMPLEMENT_DYNAMIC(CScrollWnd, CWnd)

CScrollWnd::CScrollWnd()
{
	scrollPos = CPoint(0, 0);
	canvasSize = CSize(0, 0);
	windowSize = CSize(0, 0);
	lineSize = CSize(1, 1);
	pageSize = CSize(10, 10);
}

CScrollWnd::~CScrollWnd()
{
}


BEGIN_MESSAGE_MAP(CScrollWnd, CWnd)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_PAINT()
END_MESSAGE_MAP()




void CScrollWnd::SetCanvasSize(CSize s)
{
	canvasSize = s;
	ScrollTo(CPoint(0, 0));
}



void CScrollWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	windowSize = CSize(cx, cy);
	ScrollTo(CPoint(0, 0));
}


void CScrollWnd::ScrollTo(CPoint pos)
{
	PreScrollWindow();

	pos.x = max(0, min(pos.x, canvasSize.cx - windowSize.cx));
	pos.y = max(0, min(pos.y, canvasSize.cy - windowSize.cy));
	
	ScrollWindowEx(scrollPos.x - pos.x, scrollPos.y - pos.y, NULL, NULL, NULL, NULL, SW_INVALIDATE);
				   
	scrollPos = pos;

    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPage = windowSize.cx;
    si.nMin = 0;
	si.nMax = canvasSize.cx - 1;     
	si.nPos = scrollPos.x;
    SetScrollInfo(SB_HORZ, &si, TRUE);

    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPage = windowSize.cy;
    si.nMin = 0;
	si.nMax = canvasSize.cy - 1;     
	si.nPos = scrollPos.y;
    SetScrollInfo(SB_VERT, &si, TRUE);
}

void CScrollWnd::ScrollDelta(CPoint dpos)
{
	ScrollTo(scrollPos + dpos);
}

void CScrollWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	switch (nSBCode)
	{
	case SB_LINELEFT:       ScrollDelta(CPoint(-lineSize.cx, 0)); break;
	case SB_LINERIGHT:      ScrollDelta(CPoint(lineSize.cx, 0)); break;
    case SB_PAGELEFT:       ScrollDelta(CPoint(-pageSize.cx, 0)); break;
	case SB_PAGERIGHT:      ScrollDelta(CPoint(pageSize.cx, 0)); break;
    case SB_THUMBPOSITION:  ScrollTo(CPoint(nPos, scrollPos.y)); break;
    case SB_THUMBTRACK:     ScrollTo(CPoint(nPos, scrollPos.y)); break;
    case SB_LEFT:           ScrollTo(CPoint(0, scrollPos.y)); break;
    case SB_RIGHT:          ScrollTo(CPoint(MAXLONG, scrollPos.y)); break;
	}

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CScrollWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	switch (nSBCode)
	{
	case SB_LINEUP:         ScrollDelta(CPoint(0, -lineSize.cy)); break;
	case SB_LINEDOWN:       ScrollDelta(CPoint(0, lineSize.cy)); break;
    case SB_PAGEUP:         ScrollDelta(CPoint(0, -pageSize.cy)); break;
	case SB_PAGEDOWN:       ScrollDelta(CPoint(0, pageSize.cy)); break;
    case SB_THUMBPOSITION:  ScrollTo(CPoint(scrollPos.x, nPos)); break;
    case SB_THUMBTRACK:     ScrollTo(CPoint(scrollPos.x, nPos)); break;
    case SB_TOP:            ScrollTo(CPoint(scrollPos.x, 0)); break;
    case SB_BOTTOM:         ScrollTo(CPoint(scrollPos.x, MAXLONG)); break;
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CScrollWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	dc.SetWindowOrg(scrollPos.x, scrollPos.y);

	CRect r;
	dc.GetClipBox(&r);		

	if (r.IsRectEmpty())
		return;

	CXMemDC memDC(dc, &r);
	CDC *pDC = &memDC.GetDC();
	OnDraw(pDC);
}
