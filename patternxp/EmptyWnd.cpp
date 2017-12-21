// EmptyWnd.cpp : implementation file
//

#include "stdafx.h"
#include "EmptyWnd.h"


// CEmptyWnd

IMPLEMENT_DYNAMIC(CEmptyWnd, CScrollWnd)

CEmptyWnd::CEmptyWnd()
{

}

CEmptyWnd::~CEmptyWnd()
{
}

BEGIN_MESSAGE_MAP(CEmptyWnd, CScrollWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CEmptyWnd message handlers



void CEmptyWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	COLORREF bgcolor = pCB->GetThemeColor("PE BG");

	CRect cr;
	GetClientRect(&cr);
	dc.FillSolidRect(&cr, bgcolor);
}
