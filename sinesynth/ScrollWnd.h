#pragma once


// CScrollWnd

class CScrollWnd : public CWnd
{
	DECLARE_DYNAMIC(CScrollWnd)

public:
	CScrollWnd();
	virtual ~CScrollWnd();

	void SetLineSize(CSize s) { lineSize = s; }
	void SetPageSize(CSize s) { pageSize = s; }
	void SetCanvasSize(CSize s);
	CPoint GetScrollPos() const { return scrollPos; }

	CRect GetCanvasRect() const { return CRect(CPoint(0, 0), canvasSize); }


	virtual void OnDraw(CDC *pDC) {}
	virtual void PreScrollWindow() {}

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:
	void ScrollTo(CPoint pos);
	void ScrollDelta(CPoint dpos);

private:
	CPoint scrollPos;
	CSize canvasSize;
	CSize windowSize;
	CSize lineSize;
	CSize pageSize;

public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
};


