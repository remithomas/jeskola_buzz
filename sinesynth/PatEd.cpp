// PatEd.cpp : implementation file
//

#include "stdafx.h"
#include "SineSynth.h"
#include "PatEd.h"

int const ValueWidth = 2;
int const NoteHeight = 20;

// CPatEd

IMPLEMENT_DYNAMIC(CPatEd, CScrollWnd)

CPatEd::CPatEd()
{
	pPattern = NULL;
	drawing = false;
	playPos = 0;
	drawPlayPos = -1;
	SetLineSize(CSize(ValueWidth * ValuesPerTick));
	SetPageSize(CSize(4 * ValueWidth * ValuesPerTick));
}

CPatEd::~CPatEd()
{
}


BEGIN_MESSAGE_MAP(CPatEd, CScrollWnd)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
END_MESSAGE_MAP()



void CPatEd::Create(CWnd *parent)
{
	CWnd::Create(NULL, "pated", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL, CRect(0, 0, 100, 100), parent, 0);
	SetTimer(1, 20, NULL);
}

void CPatEd::SetPattern(CMachinePattern *p)
{
	pPattern = p;
	InvalidateRect(NULL);
	PatternLengthChanged(p);
}

void CPatEd::PatternLengthChanged(CMachinePattern *p)
{
	if (p != NULL && p == pPattern)
		SetCanvasSize(CPoint(p->data.size() * ValueWidth + 1, NoteCount * NoteHeight + 1));
}

void CPatEd::SetPlayPos(CMachinePattern *pat, int pos)
{	
	pPlayingPattern = pat;
	playPos = pos;
}


void CPatEd::OnDraw(CDC *pDC)
{
	COLORREF bgcolor = pCB->GetThemeColor("PE BG");
	COLORREF bgdcolor = pCB->GetThemeColor("PE BG Dark");
	COLORREF bgvdcolor = pCB->GetThemeColor("PE BG Very Dark");
	COLORREF textcolor = pCB->GetThemeColor("PE Text");

	CRect clientr;
	GetClientRect(&clientr);

	CRect cr = GetCanvasRect();

	CRect bgrect;
	bgrect.UnionRect(cr, clientr);
//	pDC->FillSolidRect(bgrect, RGB(rand(), rand(), rand()));
	pDC->FillSolidRect(bgrect, bgcolor);

	if (pPattern == NULL)
		return;

	CPen dpen(PS_SOLID, 1, bgdcolor);
	CPen vdpen(PS_SOLID, 1, bgvdcolor);
	CPen textpen(PS_SOLID, 1, textcolor);
	CPen *pOldPen = pDC->SelectObject(&dpen);
	
	int width = pPattern->data.size() * ValueWidth;
	int height = NoteCount * NoteHeight;

	for (int i = 0; i <= NoteCount; i++)
	{
		int y = i * NoteHeight;
		pDC->MoveTo(0, y);
		pDC->LineTo(width, y);

	}

	for (int i = 0; i <= (int)pPattern->data.size() / ValuesPerTick; i++)
	{
		int x = i * ValueWidth * ValuesPerTick;
		pDC->MoveTo(x, 0);
		pDC->LineTo(x, height);

	}

	pDC->SelectObject(&vdpen);

	for (int i = 0; i <= NoteCount; i += 12)
	{
		int y = i * NoteHeight;
		pDC->MoveTo(0, y);
		pDC->LineTo(width, y);

	}

	for (int i = 0; i <= (int)pPattern->data.size() / ValuesPerTick; i += 4)
	{
		int x = i * ValueWidth * ValuesPerTick;
		pDC->MoveTo(x, 0);
		pDC->LineTo(x, height);

	}

	pDC->SelectObject(&textpen);

	bool innote = false;

	for (int i = 0; i < (int)pPattern->data.size() - 1; i++)
	{
		int x = i * ValueWidth;
		float data = pPattern->data[i];
		float y = NoteHeight * NoteCount - (pPattern->data[i] - FirstNote) * NoteHeight;

		if (x == 0 || data == 0 || !innote)
		{
			pDC->MoveTo(x, (int)y);
		}
		else
		{
			pDC->LineTo(x, (int)y);
		}

		innote = data != 0;

	}

	pDC->SelectObject(pOldPen);


	if (drawPlayPos >= 0)
	{
		CRect ppr = cr;
		ppr.left = drawPlayPos * ValueWidth;
		ppr.right = ppr.left + 1;

//		pDC->InvertRect(&ppr);
		CBrush br(RGB(255, 255, 0));
		pDC->FillRect(&ppr, &br);
	}


}

void CPatEd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPatEd::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (pPattern == NULL)
		return;

	SetCapture();
	lastDrawPos = point;
	drawing = true;

	CWnd::OnLButtonDown(nFlags, point);
}

void CPatEd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (drawing)
	{
		drawing = false;
		ReleaseCapture();
	}

	CScrollWnd::OnLButtonUp(nFlags, point);
}

void CPatEd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!drawing)
		return;

	CPoint sp = GetScrollPos();

	CPoint p1, p2;
	
	if (point.x < lastDrawPos.x)
	{
		p1 = point;
		p2 = lastDrawPos;
	}
	else
	{
		p1 = lastDrawPos;
		p2 = point;
	}

	for (int x = p1.x; x <= p2.x; x++)
	{
		float a = (float)(x - p1.x) / (p2.x - p1.x + 1);
		float y = p1.y + a * (p2.y - p1.y);

		int time = max(0, min((x + sp.x) / ValueWidth, (int)pPattern->data.size() - 1));
		float note = FirstNote + max(0, (float)(NoteHeight * NoteCount - (y + sp.y)) / NoteHeight);
	
		if (nFlags & MK_CONTROL)
			pPattern->data[time] = 0;
		else
			pPattern->data[time] = note;

	}


	CRect cr = GetCanvasRect();
	InvalidateRect(CRect(p1.x - 5, 0, p2.x + 5, cr.Height()));

	lastDrawPos = point;

	pCB->SetModifiedFlag();

	CScrollWnd::OnMouseMove(nFlags, point);
}

void CPatEd::OnTimer(UINT_PTR nIDEvent)
{
	if (!IsWindow(GetSafeHwnd()))
		return;

	CRect r = GetCanvasRect();
	CPoint sp = GetScrollPos();

	if (drawPlayPos >= 0 && drawPlayPos != playPos)
	{
		r.left = drawPlayPos * ValueWidth - sp.x;
		r.right = r.left + 1;
		drawPlayPos = -1;
		InvalidateRect(r);
	}

	if (pPlayingPattern == pPattern)
	{
		drawPlayPos = playPos;		// playPos is written to in another thread so read it only once
		r.left = playPos * ValueWidth - sp.x;
		r.right = r.left + 1;
		InvalidateRect(r);
	}

	CScrollWnd::OnTimer(nIDEvent);
}

