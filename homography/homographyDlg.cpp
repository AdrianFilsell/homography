
// homographyDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "homography.h"
#include "homographyDlg.h"
#include "afxdialogex.h"
#include "dib.h"
#include "jpeg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// ChomographyDlg dialog
ChomographyDlg::ChomographyDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HOMOGRAPHY_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ChomographyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX,IDC_IMAGE_PATH_EDIT,m_csPath);
}

BEGIN_MESSAGE_MAP(ChomographyDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_IMAGE_LOAD,ChomographyDlg::OnLoad)
	ON_BN_CLICKED(IDC_RESET_BTN,ChomographyDlg::OnReset)
END_MESSAGE_MAP()


// ChomographyDlg message handlers

BOOL ChomographyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CRect rcClientRect;
	GetClientRect(rcClientRect);
	m_rCtrlRectMapClient={{rcClientRect.left,rcClientRect.top},{rcClientRect.right,rcClientRect.bottom}};
	m_CtrlRectMap[IDC_INPUT_GRP]=getrect(IDC_INPUT_GRP);
	m_CtrlRectMap[IDC_IMAGE_PATH_EDIT]=getrect(IDC_IMAGE_PATH_EDIT);
	m_CtrlRectMap[IDC_IMAGE_LOAD]=getrect(IDC_IMAGE_LOAD);
	m_CtrlRectMap[IDC_IMAGE_GRP]=getrect(IDC_IMAGE_GRP);
	m_CtrlRectMap[IDC_IMAGE_RECT]=getrect(IDC_IMAGE_RECT);
	m_CtrlRectMap[IDC_RESET_BTN]=getrect(IDC_RESET_BTN);
	m_CtrlRectMap[IDOK]=getrect(IDOK);
	m_CtrlRectMap[IDCANCEL]=getrect(IDCANCEL);

	m_spSplitter=std::shared_ptr<splitterwnd>(new splitterwnd);
	m_spSplitter->create(this);

	updatelayout();
	
	CRect rcClient;
	m_spSplitter->GetClientRect(rcClient);

	const int nIdeal = rcClient.Width()/2, nMin = 100;
	m_spSplitter->SetColumnInfo(0,nIdeal,nMin);
	m_spSplitter->SetColumnInfo(1,nIdeal,nMin);

	m_spSplitter->SetActivePane(0,0,m_spSplitter->getpanes()[0].get());
	m_spSplitter->RecalcLayout();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void ChomographyDlg::OnCancel(void)
{
	if(m_spSplitter && m_spSplitter->getpanes().size()==2)
	{
		if(m_spSplitter->getpanes()[0]->isdragging())
		{
			m_spSplitter->getpanes()[0]->canceldrag();
			return;
		}
		if(m_spSplitter->getpanes()[1]->isdragging())
		{
			m_spSplitter->getpanes()[1]->canceldrag();
			return;
		}
	}
	CDialogEx::OnCancel();
}

void ChomographyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void ChomographyDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR ChomographyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void ChomographyDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	// Call the base class
	lpMMI->ptMinTrackSize.x = 600;
	lpMMI->ptMinTrackSize.y = 500;
	CDialogEx::OnGetMinMaxInfo( lpMMI );
}

void ChomographyDlg::OnSize(UINT nType, int cx, int cy)
{
	// Call the base class
	CDialogEx::OnSize(nType, cx, cy);

	// Update the layout
	updatelayout();
}

void ChomographyDlg::OnReset()
{
	if(!m_spSplitter || m_spSplitter->getpanes().size() != 2 || !m_spSplitter->getpanes()[0]->getdib() || !m_spSplitter->getpanes()[1]->getdib())
		return;

	const double dWidth=m_spSplitter->getpanes()[0]->getdib()->getwidth();
	const double dHeight=m_spSplitter->getpanes()[0]->getdib()->getheight();

	std::vector<std::shared_ptr<const af2d::quad<>>> v={std::shared_ptr<af2d::quad<>>(new af2d::quad<>({{0,0},{dWidth,0},{dWidth,dHeight},{0,dHeight}})),
														std::shared_ptr<af2d::quad<>>(new af2d::quad<>({{0,0},{dWidth,0},{dWidth,dHeight},{0,dHeight}}))};

	m_spSplitter->getpanes()[0]->setquad(v[0]);
	m_spSplitter->getpanes()[1]->setquad(v[1]);
}

void ChomographyDlg::OnLoad()
{
	CFileDialog dlg(true);
	if(dlg.DoModal()!=IDOK)
		return;

	const CString csPath=dlg.GetPathName();
	std::shared_ptr<afdib::dib> spDib = afdib::jpeg::load8bpp(csPath);
	if(!spDib)
		return;
	const double dWidth=spDib->getwidth();
	const double dHeight=spDib->getheight();

	std::vector<std::shared_ptr<const af2d::quad<>>> v={std::shared_ptr<af2d::quad<>>(new af2d::quad<>({{0,0},{dWidth,0},{dWidth,dHeight},{0,dHeight}})),
														std::shared_ptr<af2d::quad<>>(new af2d::quad<>({{0,0},{dWidth,0},{dWidth,dHeight},{0,dHeight}}))};
	
	m_spSplitter->getpanes()[0]->setother(m_spSplitter->getpanes()[1].get());
	m_spSplitter->getpanes()[1]->setother(m_spSplitter->getpanes()[0].get());

	m_spSplitter->getpanes()[0]->setdib(spDib);
	m_spSplitter->getpanes()[1]->setdib(spDib);

	m_spSplitter->getpanes()[0]->setquad(v[0]);
	m_spSplitter->getpanes()[1]->setquad(v[1]);

	m_csPath=csPath;
	UpdateData(false); // update ui
}

af2d::rect ChomographyDlg::getrect(const int nID)const
{
	RECT r;
	auto w = GetDlgItem(nID);
	if(w)
	{
		w->GetWindowRect(&r);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&r,2);
		return {{r.left,r.top},{r.right,r.bottom}};
	}
	return {};
}

void ChomographyDlg::updatelayout(void)
{
	if(m_rCtrlRectMapClient.isempty())
		return;
	CRect rcClientRect;
	GetClientRect(rcClientRect);
	auto swp_ptr=[this](CWnd *p,const af2d::rect& r)->void{if(p)::SetWindowPos(p->GetSafeHwnd(), NULL, r.get(af2d::rect::tl).getx(), r.get(af2d::rect::tl).gety(), r.get(af2d::rect::br).getx() - r.get(af2d::rect::tl).getx(), r.get(af2d::rect::br).gety() - r.get(af2d::rect::tl).gety(), SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOZORDER);};
	auto swp_id=[this,swp_ptr](const int n,const af2d::rect& r)->void{swp_ptr(GetDlgItem(n),r);};
	{
		af2d::rect rGrp = m_CtrlRectMap[IDC_INPUT_GRP];
		{
			rGrp.offset(af2d::rect::br,{rcClientRect.right-(m_rCtrlRectMapClient.get(af2d::rect::br).getx()),0});
			swp_id(IDC_INPUT_GRP,rGrp);
		}
		af2d::rect rBtn = m_CtrlRectMap[IDC_IMAGE_LOAD];
		{
			rBtn.offset({(rGrp.get(af2d::rect::br).getx()-m_CtrlRectMap[IDC_INPUT_GRP].get(af2d::rect::br).getx()),0});
			swp_id(IDC_IMAGE_LOAD,rBtn);
		}
		af2d::rect rEdit = m_CtrlRectMap[IDC_IMAGE_PATH_EDIT];
		{
			rEdit.offset(af2d::rect::br,{(rBtn.get(af2d::rect::br).getx()-m_CtrlRectMap[IDC_IMAGE_LOAD].get(af2d::rect::br).getx()),0});
			swp_id(IDC_IMAGE_PATH_EDIT,rEdit);
		}
	}
	af2d::rect rCancel = m_CtrlRectMap[IDCANCEL];
	{
		rCancel.offset({(rcClientRect.right-m_rCtrlRectMapClient.get(af2d::rect::br).getx()),(rcClientRect.bottom-m_rCtrlRectMapClient.get(af2d::rect::br).gety())});
		swp_id(IDCANCEL,rCancel);
	}
	af2d::rect rOK  = m_CtrlRectMap[IDOK];
	{
		rOK .offset({(rCancel.get(af2d::rect::tl).getx()-m_CtrlRectMap[IDCANCEL].get(af2d::rect::tl).getx()),(rCancel.get(af2d::rect::tl).gety()-m_CtrlRectMap[IDCANCEL].get(af2d::rect::tl).gety())});
		swp_id(IDOK,rOK);
	}
	{
		af2d::rect rGrp = m_CtrlRectMap[IDC_IMAGE_GRP];
		{
			rGrp.offset(af2d::rect::br,{rcClientRect.right-(m_rCtrlRectMapClient.get(af2d::rect::br).getx()),rOK.get(af2d::rect::tl).gety()-(m_CtrlRectMap[IDOK].get(af2d::rect::tl).gety())});
			swp_id(IDC_IMAGE_GRP,rGrp);
		}
		af2d::rect rResetBtn = m_CtrlRectMap[IDC_RESET_BTN];
		{
			rResetBtn.offset({0,rGrp.get(af2d::rect::br).gety()-(m_CtrlRectMap[IDC_IMAGE_GRP].get(af2d::rect::br).gety())});
			swp_id(IDC_RESET_BTN,rResetBtn);
		}
		af2d::rect rImageRect = m_CtrlRectMap[IDC_IMAGE_RECT];
		{
			rImageRect.offset(af2d::rect::br,{rGrp.get(af2d::rect::br).getx()-(m_CtrlRectMap[IDC_IMAGE_GRP].get(af2d::rect::br).getx()),rResetBtn.get(af2d::rect::br).gety()-(m_CtrlRectMap[IDC_RESET_BTN].get(af2d::rect::br).gety())});
			swp_id(IDC_IMAGE_RECT,rImageRect);
		}
		swp_ptr(m_spSplitter.get(),rImageRect);
	}
}
