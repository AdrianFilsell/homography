
// homographyDlg.h : header file
//

#pragma once

#include "matrix.h"
#include "2d.h"
#include "splitterwnd.h"
#include <map>

// ChomographyDlg dialog
class ChomographyDlg : public CDialogEx
{
// Construction
public:
	ChomographyDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_HOMOGRAPHY_DIALOG };
	CString m_csPath;

	protected:
	virtual void OnCancel(void)override;
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	af2d::rect m_rCtrlRectMapClient;
	std::map<int,af2d::rect> m_CtrlRectMap;
	std::shared_ptr<splitterwnd> m_spSplitter;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLoad();
	afx_msg void OnReset();
	DECLARE_MESSAGE_MAP()

	af2d::rect getrect(const int nID)const;
	void updatelayout(void);
};
