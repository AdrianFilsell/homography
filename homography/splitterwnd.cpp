
#include "pch.h"
#include "splitterwnd.h"
#include "homography.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(splitterwnd, CSplitterWnd)
END_MESSAGE_MAP()


CWnd* splitterwnd::GetActivePane(int* pRow, int* pCol)
{
	if(GetParentFrame())
		return CSplitterWnd::GetActivePane(pRow, pCol);

	ASSERT_VALID(this);
	CWnd* pFocus = GetFocus();
	HWND h = pFocus ? pFocus->GetSafeHwnd() : NULL;
	// make sure the pane is a child pane of the splitter
	return pFocus && IsChildPane(pFocus, pRow, pCol) ? pFocus : NULL;
}

void splitterwnd::SetActivePane( int row, int col, CWnd* pWnd)
{
	// set the focus to the pane
	CWnd* pPane = pWnd == NULL ? GetPane(row, col) : pWnd;
	pPane->SetFocus();
}

void splitterwnd::StartTracking(int ht)
{
	CSplitterWnd::StartTracking(ht);
}

void splitterwnd::StopTracking(BOOL bAccept)
{
	CSplitterWnd::StopTracking(bAccept);
	auto i = m_vPanes.cbegin(), end = m_vPanes.cend();
	for(;i!=end;++i)
		if((*i)->GetSafeHwnd())
			(*i)->Invalidate();
}

void splitterwnd::create(CWnd *pParent)
{
	CreateStatic(pParent,1,2);

	std::shared_ptr<dibwnd> spPane = std::shared_ptr<dibwnd>(new dibwnd(dibwnd::t_from));
	spPane->Create(AfxRegisterWndClass(0,theApp.LoadStandardCursor(IDC_ARROW),::GetSysColorBrush(COLOR_WINDOW)),_T("dib"),WS_VISIBLE|WS_CHILD,CRect(0,0,100,100),this,AFX_IDW_PANE_FIRST);
	m_vPanes.push_back(spPane);

	spPane = std::shared_ptr<dibwnd>(new dibwnd(dibwnd::t_to));
	spPane->Create(AfxRegisterWndClass(0,theApp.LoadStandardCursor(IDC_ARROW),::GetSysColorBrush(COLOR_WINDOW)),_T("dib"),WS_VISIBLE|WS_CHILD,CRect(0,0,100,100),this,AFX_IDW_PANE_FIRST+1);
	m_vPanes.push_back(spPane);
}
