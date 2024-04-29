#pragma once

#include "dibwnd.h"
#include <memory>

class splitterwnd : public CSplitterWnd
{
public:
	splitterwnd(){}
	virtual ~splitterwnd(){}

	const std::vector<std::shared_ptr<dibwnd>>& getpanes(void)const{return m_vPanes;}

	void create(CWnd *pParent);

	virtual CWnd* GetActivePane(int* pRow = NULL, int* pCol = NULL) override;
	virtual void SetActivePane( int row, int col, CWnd* pWnd = NULL) override;
protected:
	std::vector<std::shared_ptr<dibwnd>> m_vPanes;

	// Generated message map functions
	DECLARE_MESSAGE_MAP()

	virtual void StartTracking(int ht) override;
	virtual void StopTracking(BOOL bAccept) override;
};
