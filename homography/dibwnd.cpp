// traincostgraphwnd.cpp : implementation file
//

#include "pch.h"
#include "dibwnd.h"
#include "core.h"

// dibwnd

IMPLEMENT_DYNAMIC(dibwnd, CWnd)

int dibwnd::m_nChequerDim=10;
int dibwnd::m_nDibBorderGap=10;

unsigned char dibwnd::cWhite[3]={255,255,255};
unsigned char dibwnd::cGrey[3]={200,200,200};

int dibwnd::m_nHandleDim=11;

dibwnd::dibwnd(const type t)
{
	m_Type=t;
	m_pOther=nullptr;
	m_bCaptured=false;
	m_ht=ht_null;
	m_htDrag=ht_null;
}

dibwnd::~dibwnd()
{
}


BEGIN_MESSAGE_MAP(dibwnd, CWnd)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// dibwnd message handlers
void dibwnd::OnSize(UINT nType, int cx, int cy)
{
	// base class
	CWnd::OnSize(nType, cx, cy);

	m_spCanvas=std::shared_ptr<afdib::dib>(new afdib::dib);
	m_spCanvas->create(cx,cy,afdib::dib::pt_b8g8r8);

	composetransforms2d(m_spWndToDib2d,m_spDibToWnd2d,m_spCanvas,m_spDib);
	{
		Eigen::Matrix<double,3,3> homography;
		std::shared_ptr<const Eigen::Projective2d> spWndToDib2dProj;
		std::shared_ptr<const Eigen::Projective2d> spDibToWnd2dProj;
		if(m_Type==t_to && m_pOther && m_pOther->getdibquad() && afhomography::matrix<>::get(*m_pOther->getdibquad(),*getdibquad(),homography) )
			composetransforms2dproj(homography,spWndToDib2dProj,spDibToWnd2dProj,m_spCanvas,m_spDib);
		m_spWndToDib2dProj=spWndToDib2dProj;
		m_spDibToWnd2dProj=spDibToWnd2dProj;
	}

	const af2d::rect r({{0,0},{m_spCanvas->getwidth(),m_spCanvas->getheight()}});
	composebkgnd(r,m_spCanvas);
	if(m_spWndToDib2dProj)
		composedib(m_spDibToWnd2dProj.get(),m_spWndToDib2dProj.get(),r,m_spDib,m_spCanvas);
	else
		composedib(m_spDibToWnd2d.get(),m_spWndToDib2d.get(),r,m_spDib,m_spCanvas);
}

BOOL dibwnd::OnEraseBkgnd(CDC *pDC)
{
	return m_spCanvas ? TRUE : CWnd::OnEraseBkgnd(pDC);
}

void dibwnd::OnPaint(void)
{
	CPaintDC dc(this);

	CRect rcClientRect;
	GetClientRect(rcClientRect);

	if(m_spCanvas)
	{
		const CRect rcDst = rcClientRect;
		const CRect rcSrc(0,0,m_spCanvas->getwidth(),m_spCanvas->getheight());
		
		BITMAPINFO *pBmi = m_spCanvas->createbitmapinfo();
		const int n = ::StretchDIBits(dc.GetSafeHdc(),
									  rcDst.TopLeft().x,rcDst.TopLeft().y,rcDst.Width(),rcDst.Height(),
									  rcSrc.TopLeft().x,rcSrc.TopLeft().y,rcSrc.Width(),rcSrc.Height(),
									  m_spCanvas->getscanline(0),pBmi,DIB_RGB_COLORS,SRCCOPY);
		m_spCanvas->tidybmi(pBmi);
	}
	else
		dc.FillSolidRect(rcClientRect,::GetSysColor(COLOR_WINDOW));

	CPoint dibpts[8], wndpts[8];
	if(gethandles(dibpts,wndpts))
	{
		const int nWidth=2;
		CPen p(PS_SOLID,nWidth,RGB(255,0,0));
		CPen *pOldPen=dc.SelectObject(&p);
		CPoint pts[5]={wndpts[0],wndpts[1],wndpts[2],wndpts[3],wndpts[0]};
		dc.Polyline(pts,5); // polyline NOT inclusive	
		for(int n=0;n<4;++n)
		{
			{
				CPoint cpTL=wndpts[n]-CPoint(m_nHandleDim/2,m_nHandleDim/2);
				CPoint cpBR=cpTL+CPoint(m_nHandleDim,m_nHandleDim);
				dc.Ellipse(CRect(cpTL,cpBR));
			}

			{
				CPoint cpTL=wndpts[4+n]-CPoint(m_nHandleDim/2,m_nHandleDim/2);
				CPoint cpBR=cpTL+CPoint(m_nHandleDim,m_nHandleDim);
				dc.Ellipse(CRect(cpTL,cpBR));
			}
		}

		dc.SelectObject(pOldPen);
	}
}

void dibwnd::OnMouseMove( UINT nFlags, CPoint point )
{
	CWnd::OnMouseMove( nFlags, point );

	if(m_htDrag==ht_null)
	{
		m_ht=gethittest(point);

		if(m_spWndToDib2dProj)
		{
			double dSrcX,dSrcY;
			gettransformxy(m_spWndToDib2dProj.get(),point.x,point.y,dSrcX,dSrcY);
			TRACE(_T("wnd %li,%li -> %f,%f\r\n"),point.x,point.y,dSrcX,dSrcY);
		}
	}
	else
		drag(point);
	setcursor();
}

void dibwnd::OnLButtonDown( UINT nFlags, CPoint point )
{
	CWnd::OnLButtonDown( nFlags, point );

	if(m_htDrag==ht_null)
	{
		if(m_ht==ht_null)
			return;
		setcapture();
		setcursor();
		m_htDrag=m_ht;
		m_cpWndDragFrom=point;
		m_spFromDragDibQuad=m_spDibQuad;
	}
}

void dibwnd::OnLButtonUp( UINT nFlags, CPoint point )
{
	CWnd::OnLButtonUp( nFlags, point );

	if(m_htDrag==ht_null)
	{
	}
	else
	{
		drag(point);
		releasecapture();
		m_htDrag=ht_null;
		m_ht=gethittest(point);
		setcursor();
		m_spFromDragDibQuad=nullptr;
	}
}

void dibwnd::canceldrag(void)
{
	if(m_htDrag==ht_null)
		return;

	CPoint cp;
	::GetCursorPos(&cp);
	::MapWindowPoints(NULL,m_hWnd,&cp,1);

	releasecapture();
	m_htDrag=ht_null;
	m_ht=gethittest(cp);
	setcursor();
	setquad(m_spFromDragDibQuad);
	m_spFromDragDibQuad=nullptr;
}

void dibwnd::setcapture(void)
{
	if(m_bCaptured)
		return;
	SetCapture();
	m_bCaptured=true;
}

void dibwnd::releasecapture(void)
{
	if(m_bCaptured)
	{
		ReleaseCapture();
		m_bCaptured=false;
	}
}

dibwnd::hittest dibwnd::gethittest(const CPoint& cpWnd)const
{
	const std::vector<hittest> vCorners={ht_tl,ht_tr,ht_br,ht_bl};
	const std::vector<hittest> vMids={ht_tltr_mid,ht_trbr_mid,ht_blbr_mid,ht_tlbl_mid};
	
	CPoint dib[8],wnd[8];
	gethandles(dib,wnd);

	auto fn=[](const CPoint& cpTo,const CPoint& cpFrom)->bool
	{
		const double dR=11/2.0;
		const double dX=cpTo.x-cpFrom.x;
		const double dY=cpTo.y-cpFrom.y;
		const double dSqLen=(dX*dX)+(dY*dY);
		const double dLen=dSqLen==0?0:sqrt(dSqLen);
		return (dLen>dR)?false:true;
	};
	
	int n=0;
	auto i=vCorners.cbegin(),end=vCorners.cend();
	for(;i!=end;++i,++n)
		if(fn(cpWnd,wnd[n]))
			return *i;
	for(i=vMids.cbegin(),end=vMids.cend();i!=end;++i,++n)
		if(fn(cpWnd,wnd[n]))
			return *i;

	return ht_null;
}

void dibwnd::setcursor(void)const
{
	switch(m_ht)
	{
		case ht_tl:
		case ht_tr:
		case ht_br:
		case ht_bl:
		case ht_tltr_mid:
		case ht_trbr_mid:
		case ht_blbr_mid:
		case ht_tlbl_mid: ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));break;
		case ht_null: ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));break;
	}
}

bool dibwnd::gethandles(CPoint (&dib)[8],CPoint (&wnd)[8])const
{
	std::shared_ptr<const af2d::quad<>> spQ = m_spDibQuad;
	if(!spQ || spQ->isempty() || !m_spDibToWnd2d )
		return false;
	
	const af2d::quad<> q=*spQ;
	const af2d::quad<> qT=gettransformquad(m_spDibToWnd2d.get(),*spQ);
	
	dib[0]={int(q.get(af2d::quad<>::tl).getx()),int(q.get(af2d::quad<>::tl).gety())};
	dib[1]={int(q.get(af2d::quad<>::tr).getx()),int(q.get(af2d::quad<>::tr).gety())};
	dib[2]={int(q.get(af2d::quad<>::br).getx()),int(q.get(af2d::quad<>::br).gety())};
	dib[3]={int(q.get(af2d::quad<>::bl).getx()),int(q.get(af2d::quad<>::bl).gety())};
	
	wnd[0]={int(qT.get(af2d::quad<>::tl).getx()),int(qT.get(af2d::quad<>::tl).gety())};
	wnd[1]={int(qT.get(af2d::quad<>::tr).getx()),int(qT.get(af2d::quad<>::tr).gety())};
	wnd[2]={int(qT.get(af2d::quad<>::br).getx()),int(qT.get(af2d::quad<>::br).gety())};
	wnd[3]={int(qT.get(af2d::quad<>::bl).getx()),int(qT.get(af2d::quad<>::bl).gety())};

	const std::vector<af2d::quad<>::vertex> vFrom={af2d::quad<>::tl,af2d::quad<>::tr,af2d::quad<>::br,af2d::quad<>::bl};
	const std::vector<af2d::quad<>::vertex> vTo={af2d::quad<>::tr,af2d::quad<>::br,af2d::quad<>::bl,af2d::quad<>::tl};
	auto iFrom=vFrom.cbegin();
	auto end=vFrom.cend();
	auto iTo=vTo.cbegin();
	for(int n=0;iFrom!=end;++iFrom,++iTo,++n)
	{
		const auto& from=q.get(*iFrom);
		const auto& to=q.get(*iTo);

		double dDirX,dDirY;
		dDirX=(to.getx()-from.getx());
		dDirY=(to.gety()-from.gety());

		double dLen=(dDirX*dDirX)+(dDirY*dDirY);
		dLen=dLen?sqrt(dLen):dLen;
		dDirX=dLen?dDirX/dLen:0;
		dDirY=dLen?dDirY/dLen:0;
		
		double dMidX,dMidY;
		dMidX=(from.getx()+(dDirX*dLen*0.5));
		dMidY=(from.gety()+(dDirY*dLen*0.5));
		
		dib[4+n]=CPoint(int(dMidX),int(dMidY));

		Eigen::Vector2d e=*m_spDibToWnd2d*Eigen::Vector2d(dMidX,dMidY);
		wnd[4+n]=CPoint(int(e.x()),int(e.y()));
	}
	return true;
}

void dibwnd::drag(const CPoint& cpWnd)
{
	if(m_htDrag==ht_null || !m_spFromDragDibQuad)
		return;
	af2d::point<> dibquadpts[2];
	bool bMid=false;
	switch(m_htDrag)
	{
		case ht_tl:dibquadpts[0]=m_spFromDragDibQuad->get(af2d::quad<>::tl);break;
		case ht_tr:dibquadpts[0]=m_spFromDragDibQuad->get(af2d::quad<>::tr);break;
		case ht_br:dibquadpts[0]=m_spFromDragDibQuad->get(af2d::quad<>::br);break;
		case ht_bl:dibquadpts[0]=m_spFromDragDibQuad->get(af2d::quad<>::bl);break;
		case ht_tltr_mid:
			bMid=true;
			dibquadpts[0]=m_spFromDragDibQuad->get(af2d::quad<>::tl);
			dibquadpts[1]=m_spFromDragDibQuad->get(af2d::quad<>::tr);
		break;
		case ht_trbr_mid:
			bMid=true;
			dibquadpts[0]=m_spFromDragDibQuad->get(af2d::quad<>::tr);
			dibquadpts[1]=m_spFromDragDibQuad->get(af2d::quad<>::br);
		break;
		case ht_blbr_mid:
			bMid=true;
			dibquadpts[0]=m_spFromDragDibQuad->get(af2d::quad<>::br);
			dibquadpts[1]=m_spFromDragDibQuad->get(af2d::quad<>::bl);
		break;
		case ht_tlbl_mid:
			bMid=true;
			dibquadpts[0]=m_spFromDragDibQuad->get(af2d::quad<>::bl);
			dibquadpts[1]=m_spFromDragDibQuad->get(af2d::quad<>::tl);
		break;
		
		default:return;
	}

	const Eigen::Vector2d dibdragfrom = *m_spWndToDib2d * Eigen::Vector2d(m_cpWndDragFrom.x+0.5,m_cpWndDragFrom.y+0.5);
	const Eigen::Vector2d dibdragto = *m_spWndToDib2d * Eigen::Vector2d(cpWnd.x+0.5,cpWnd.y+0.5);

	const double dDibDeltaX = dibdragto.x()-dibdragfrom.x();
	const double dDibDeltaY = dibdragto.y()-dibdragfrom.y();

	dibquadpts[0].setx(dibquadpts[0].getx()+dDibDeltaX);
	dibquadpts[0].sety(dibquadpts[0].gety()+dDibDeltaY);
	if(bMid)
	{
		dibquadpts[1].setx(dibquadpts[1].getx()+dDibDeltaX);
		dibquadpts[1].sety(dibquadpts[1].gety()+dDibDeltaY);
	}

	std::shared_ptr<af2d::quad<>> spDragDibQuad=std::shared_ptr<af2d::quad<>>(new af2d::quad<>(*m_spFromDragDibQuad));
	switch(m_htDrag)
	{
		case ht_tl:spDragDibQuad->set(af2d::quad<>::tl,dibquadpts[0]);break;
		case ht_tr:spDragDibQuad->set(af2d::quad<>::tr,dibquadpts[0]);break;
		case ht_br:spDragDibQuad->set(af2d::quad<>::br,dibquadpts[0]);break;
		case ht_bl:spDragDibQuad->set(af2d::quad<>::bl,dibquadpts[0]);break;
		case ht_tltr_mid:
			spDragDibQuad->set(af2d::quad<>::tl,dibquadpts[0]);
			spDragDibQuad->set(af2d::quad<>::tr,dibquadpts[1]);
		break;
		case ht_trbr_mid:
			spDragDibQuad->set(af2d::quad<>::tr,dibquadpts[0]);
			spDragDibQuad->set(af2d::quad<>::br,dibquadpts[1]);
		break;
		case ht_blbr_mid:
			spDragDibQuad->set(af2d::quad<>::br,dibquadpts[0]);
			spDragDibQuad->set(af2d::quad<>::bl,dibquadpts[1]);
		break;
		case ht_tlbl_mid:
			spDragDibQuad->set(af2d::quad<>::bl,dibquadpts[0]);
			spDragDibQuad->set(af2d::quad<>::tl,dibquadpts[1]);
		break;
		
		default:return;
	}

	setquad(spDragDibQuad);
}

void dibwnd::setquad(std::shared_ptr<const af2d::quad<>> sp)
{
	if(!sp)
		return;

	std::shared_ptr<const af2d::quad<>> spPreQuads=m_spDibQuad;
	std::shared_ptr<const Eigen::Projective2d> spPreWndToDib2dProj=m_spWndToDib2dProj;
	std::shared_ptr<const Eigen::Projective2d> spPreDibToWnd2dProj=m_spDibToWnd2dProj;
	std::shared_ptr<const Eigen::Affine2d> spPreWndToDib2d=m_spWndToDib2d;
	std::shared_ptr<const Eigen::Affine2d> spPreDibToWnd2d=m_spDibToWnd2d;
	const bool bPreConvex=m_spDibQuad && m_spDibQuad->isconvex() && m_pOther && m_pOther->getdibquad() && m_pOther->getdibquad()->isconvex();

	m_spDibQuad=nullptr;
	m_spWndToDib2dProj=nullptr;
	m_spDibToWnd2dProj=nullptr;
	
	m_spDibQuad=sp;
	const bool bPostConvex=m_spDibQuad && m_spDibQuad->isconvex() && m_pOther && m_pOther->getdibquad() && m_pOther->getdibquad()->isconvex();

	Eigen::Matrix<double,3,3> homography;
	if(m_Type==t_to && bPostConvex && afhomography::matrix<>::get(*m_pOther->getdibquad(),*getdibquad(),homography))
		composetransforms2dproj(homography,m_spWndToDib2dProj,m_spDibToWnd2dProj,m_spCanvas,m_spDib);

	if(m_spCanvas && m_Type==t_to)
	{
		const af2d::rect rCanvas({{0,0},{m_spCanvas->getwidth(),m_spCanvas->getheight()}});

		composebkgnd(rCanvas,m_spCanvas);
			
		if(m_spWndToDib2dProj)
			composedib(m_spDibToWnd2dProj.get(),m_spWndToDib2dProj.get(),rCanvas,m_spDib,m_spCanvas);
		else
			composedib(m_spDibToWnd2d.get(),m_spWndToDib2d.get(),rCanvas,m_spDib,m_spCanvas);
	}

	if(GetSafeHwnd())
		Invalidate();

	{
		if(m_pOther && m_spDibToWnd2dProj)
		{
			const af2d::quad<> q=*m_pOther->getdibquad();
			const af2d::quad<> qT=gettransformquad(m_spDibToWnd2dProj.get(),q);
			TRACE(_T("tl:%f,%f -> %f,%f\r\n"),q.get(af2d::quad<>::tl).getx(),q.get(af2d::quad<>::tl).gety(),
										  qT.get(af2d::quad<>::tl).getx(),qT.get(af2d::quad<>::tl).gety());
			TRACE(_T("tr:%f,%f -> %f,%f\r\n"),q.get(af2d::quad<>::tr).getx(),q.get(af2d::quad<>::tr).gety(),
										  qT.get(af2d::quad<>::tr).getx(),qT.get(af2d::quad<>::tr).gety());
			TRACE(_T("br:%f,%f -> %f,%f\r\n"),q.get(af2d::quad<>::br).getx(),q.get(af2d::quad<>::br).gety(),
										  qT.get(af2d::quad<>::br).getx(),qT.get(af2d::quad<>::br).gety());
			TRACE(_T("bl:%f,%f -> %f,%f\r\n\r\n"),q.get(af2d::quad<>::bl).getx(),q.get(af2d::quad<>::bl).gety(),
										  qT.get(af2d::quad<>::bl).getx(),qT.get(af2d::quad<>::bl).gety());

			const af2d::quad<> qTT=gettransformquad(m_spWndToDib2dProj.get(),qT);
			TRACE(_T("tl:%f,%f -> %f,%f\r\n"),qT.get(af2d::quad<>::tl).getx(),qT.get(af2d::quad<>::tl).gety(),
										  qTT.get(af2d::quad<>::tl).getx(),qTT.get(af2d::quad<>::tl).gety());
			TRACE(_T("tr:%f,%f -> %f,%f\r\n"),qT.get(af2d::quad<>::tr).getx(),qT.get(af2d::quad<>::tr).gety(),
										  qTT.get(af2d::quad<>::tr).getx(),qTT.get(af2d::quad<>::tr).gety());
			TRACE(_T("br:%f,%f -> %f,%f\r\n"),qT.get(af2d::quad<>::br).getx(),qT.get(af2d::quad<>::br).gety(),
										  qTT.get(af2d::quad<>::br).getx(),qTT.get(af2d::quad<>::br).gety());
			TRACE(_T("bl:%f,%f -> %f,%f\r\n\r\n"),qT.get(af2d::quad<>::bl).getx(),qT.get(af2d::quad<>::bl).gety(),
										  qTT.get(af2d::quad<>::bl).getx(),qTT.get(af2d::quad<>::bl).gety());
		}
	}
	
	if(isfrom() && m_pOther && m_pOther->getdibquad())
		m_pOther->setquad(m_pOther->getdibquad());
}

void dibwnd::setdib(std::shared_ptr<const afdib::dib> sp)
{
	releasecapture();
	m_htDrag=ht_null;
	m_spDib=sp;
	m_spDibQuad=nullptr;
	m_spWndToDib2d=nullptr;
	m_spDibToWnd2d=nullptr;
	m_spWndToDib2dProj=nullptr;
	m_spDibToWnd2dProj=nullptr;

	composetransforms2d(m_spWndToDib2d,m_spDibToWnd2d,m_spCanvas,m_spDib);
	
	if(m_spCanvas)
	{
		const af2d::rect r({{0,0},{m_spCanvas->getwidth(),m_spCanvas->getheight()}});
		composebkgnd(r,m_spCanvas);
		if(m_spWndToDib2d)
			composedib(m_spDibToWnd2d.get(),m_spWndToDib2d.get(),r,m_spDib,m_spCanvas);
	}
	
	if(GetSafeHwnd())
		Invalidate();

	CPoint cp;
	::GetCursorPos(&cp);
	::MapWindowPoints(NULL,m_hWnd,&cp,1);

	m_ht=gethittest(cp);
	setcursor();
}

void dibwnd::composetransforms2dproj(const Eigen::Matrix<double,3,3>& homography,std::shared_ptr<const Eigen::Projective2d>& spWndToDib,std::shared_ptr<const Eigen::Projective2d>& spDibToWnd,std::shared_ptr<const afdib::dib> spCanvas,std::shared_ptr<const afdib::dib> spDib)const
{
	spWndToDib=nullptr;
	spDibToWnd=nullptr;
	if(!spDib || !spCanvas)
		return;
	double dS;

	std::shared_ptr<Eigen::Projective2d> sp=std::shared_ptr<Eigen::Projective2d>(new Eigen::Projective2d);

	af2d::rect::getrectscale(0,0,spDib->getwidth(),spDib->getheight(),0,0,spCanvas->getwidth()-(m_nDibBorderGap*2),spCanvas->getheight()-(m_nDibBorderGap*2),true,dS);
		
	*sp=Eigen::Translation<double,2>((spCanvas->getwidth()/2.0),(spCanvas->getheight()/2.0)) *
		Eigen::DiagonalMatrix<double,2>(dS,dS) *
		Eigen::Translation<double,2>(-(spDib->getwidth()/2.0),-(spDib->getheight()/2.0)) *
		homography;
	
	spDibToWnd=sp;

	sp=std::shared_ptr<Eigen::Projective2d>(new Eigen::Projective2d);
	*sp=spDibToWnd->inverse();
	spWndToDib=sp;
}

void dibwnd::composetransforms2d(std::shared_ptr<const Eigen::Affine2d>& spWndToDib,std::shared_ptr<const Eigen::Affine2d>& spDibToWnd,std::shared_ptr<const afdib::dib> spCanvas,std::shared_ptr<const afdib::dib> spDib)const
{
	spWndToDib=nullptr;
	spDibToWnd=nullptr;
	if(!spDib || !spCanvas)
		return;
	double dS;

	std::shared_ptr<Eigen::Affine2d> sp=std::shared_ptr<Eigen::Affine2d>(new Eigen::Affine2d);

	af2d::rect::getrectscale(0,0,spDib->getwidth(),spDib->getheight(),0,0,spCanvas->getwidth()-(m_nDibBorderGap*2),spCanvas->getheight()-(m_nDibBorderGap*2),true,dS);
		
	*sp=Eigen::Translation<double,2>((spCanvas->getwidth()/2.0),(spCanvas->getheight()/2.0)) *
		Eigen::DiagonalMatrix<double,2>(dS,dS) *
		Eigen::Translation<double,2>(-(spDib->getwidth()/2.0),-(spDib->getheight()/2.0));
	
	spDibToWnd=sp;

	sp=std::shared_ptr<Eigen::Affine2d>(new Eigen::Affine2d);
	*sp=spDibToWnd->inverse();
	spWndToDib=sp;
}

void dibwnd::composebkgnd(const af2d::rect& r,std::shared_ptr<afdib::dib> spDst)const
{
	// assume rect clipped to canvas
	if(!spDst)
		return;

	bkgndinfo horzinfo,vertinfo;
	getbkgndinfo(r.get(af2d::rect::tl).getx(),r.get(af2d::rect::br).getx()-1,horzinfo);
	getbkgndinfo(r.get(af2d::rect::tl).gety(),r.get(af2d::rect::br).gety()-1,vertinfo);
	if(horzinfo.nPixels==0 || vertinfo.nPixels==0)
		return;

	for(int n=0;n<2 && n<vertinfo.nWholeChunks;++n)
		composebkgndrow(horzinfo,vertinfo,n+vertinfo.nChunkFrom,spDst);

	const int nScanlineBytes = spDst->getbytesperscanline();
	unsigned char *pScanline=spDst->getscanline(vertinfo.nChunkFrom*m_nChequerDim);
	for(int nChunk=2;nChunk<vertinfo.nWholeChunks;++nChunk)
		for(int n=0;n<m_nChequerDim;++n)
		{
			memcpy(pScanline+(3*m_nChequerDim*horzinfo.nChunkFrom)+(nScanlineBytes*m_nChequerDim*2),pScanline+(3*m_nChequerDim*horzinfo.nChunkFrom),(horzinfo.nWholeChunks*m_nChequerDim*3)+(3*horzinfo.nPartialChunkPixels));
			pScanline+=nScanlineBytes;
		}

	if(vertinfo.nPartialChunkPixels>0)
		composebkgndrow(horzinfo,vertinfo,vertinfo.nChunkFrom+vertinfo.nWholeChunks,spDst);
}

void dibwnd::composebkgndrow(const bkgndinfo& horzinfo,const bkgndinfo& vertinfo, const int nVertChunk,std::shared_ptr<afdib::dib> spDst)const
{
	// first scanline
	int nComposedChunks = 0;
	for(;nComposedChunks<2 && nComposedChunks<horzinfo.nWholeChunks;++nComposedChunks)
		setbkgndrowpixels({(horzinfo.nChunkFrom+nComposedChunks)*m_nChequerDim,nVertChunk*m_nChequerDim},m_nChequerDim,getbkgndchunkcolour(horzinfo.nChunkFrom+nComposedChunks,nVertChunk),spDst);	
	
	unsigned char *pScanline=spDst->getscanline(nVertChunk*m_nChequerDim);
	while(nComposedChunks<horzinfo.nWholeChunks)
	{
		const int nDstChunk = horzinfo.nChunkFrom + nComposedChunks;
		const int nSrcChunk = horzinfo.nChunkFrom;

		const int nAvailableChunks = (nDstChunk - nSrcChunk)%2?(nDstChunk - nSrcChunk - 1):(nDstChunk - nSrcChunk); // 2 * n
		const int nRemainingWholeChunks = horzinfo.nWholeChunks - nComposedChunks;
		const int nCopyChunks = nAvailableChunks > nRemainingWholeChunks ? nRemainingWholeChunks : nAvailableChunks;
		
		memcpy(pScanline+(3*m_nChequerDim*nDstChunk),pScanline+(3*m_nChequerDim*nSrcChunk),3*m_nChequerDim*nCopyChunks);

		nComposedChunks += nCopyChunks;
	}
	if(horzinfo.nPartialChunkPixels>0)
		setbkgndrowpixels({(horzinfo.nChunkFrom+nComposedChunks)*m_nChequerDim,nVertChunk*m_nChequerDim},horzinfo.nPartialChunkPixels,getbkgndchunkcolour(horzinfo.nChunkFrom+nComposedChunks,nVertChunk),spDst);

	// remaining scanlines
	const int nScanlineBytes = spDst->getbytesperscanline();
	const int nScanlines = (vertinfo.nChunkFrom+vertinfo.nWholeChunks==nVertChunk) ? vertinfo.nPartialChunkPixels : m_nChequerDim;
	for(int n=1;n<nScanlines;++n,pScanline+=nScanlineBytes)
		memcpy(pScanline+nScanlineBytes+(3*m_nChequerDim*horzinfo.nChunkFrom),pScanline+(3*m_nChequerDim*horzinfo.nChunkFrom),(horzinfo.nWholeChunks*m_nChequerDim*3)+(horzinfo.nPartialChunkPixels*3));
}

void dibwnd::setbkgndrowpixels(const af2d::point<int>& p,const int nPixels,const unsigned char *pPixel,std::shared_ptr<afdib::dib> spDst)const
{
	unsigned char *pScanline=spDst->getscanline(p.gety());
	pScanline += p.getx()*3;
	for(int n=0;n<nPixels;++n)
	{
		pScanline[n*3+0]=pPixel[0];
		pScanline[n*3+1]=pPixel[1];
		pScanline[n*3+2]=pPixel[2];
	}
}

void dibwnd::composedib(const Eigen::Affine2d *pDibToWnd,const Eigen::Affine2d *pWndToDib,const af2d::rect& r,std::shared_ptr<const afdib::dib> spSrc,std::shared_ptr<afdib::dib> spDst)const
{
	// assume rect clipped to canvas
	if(!spDst || !spSrc || !pWndToDib || !pDibToWnd)
		return;

	const auto rCompose=r.getintersect({{0,0},{spDst->getwidth(),spDst->getheight()}});

	const int nSrcWidth=spSrc->getwidth();
	const int nSrcHeight=spSrc->getheight();
	const int nDstYFrom=rCompose.get(af2d::rect::tl).gety();
	const int nDstYInclusiveTo=rCompose.get(af2d::rect::br).gety()-1;
	const int nDstXFrom=rCompose.get(af2d::rect::tl).getx();
	const int nDstXInclusiveTo=rCompose.get(af2d::rect::br).getx()-1;
	
	unsigned char *pDstScanline=spDst->getscanline(nDstYFrom);
	const int nBytesPerScanline=spDst->getbytesperscanline();
	for(int nDstY=nDstYFrom;nDstY<=nDstYInclusiveTo;++nDstY,pDstScanline+=nBytesPerScanline)
	{
		double dSrcX, dSrcY;
		gettransformy(pWndToDib,nDstY,dSrcY);

		if(dSrcY<0)
			continue;
		const int nSrcY = af::posfloor<double,int>(dSrcY);
		if(nSrcY>=nSrcHeight)
			continue;

		const unsigned char *pSrcScanline=spSrc->getscanline(nSrcY);

		for(int nDstX=nDstXFrom;nDstX<=nDstXInclusiveTo;++nDstX)
		{
			gettransformx(pWndToDib,nDstX,dSrcX);

			if(dSrcX<0)
				continue;
			const int nSrcX = af::posfloor<double,int>(dSrcX);
			if(nSrcX>=nSrcWidth)
				continue;
		
			pDstScanline[nDstX*3+0]=pSrcScanline[nSrcX*3+0];
			pDstScanline[nDstX*3+1]=pSrcScanline[nSrcX*3+1];
			pDstScanline[nDstX*3+2]=pSrcScanline[nSrcX*3+2];
		}
	}
}

void dibwnd::composedib(const Eigen::Projective2d *pDibToWnd,const Eigen::Projective2d *pWndToDib,const af2d::rect& r,std::shared_ptr<const afdib::dib> spSrc,std::shared_ptr<afdib::dib> spDst)const
{
	// assume rect clipped to canvas
	if(!spDst || !spSrc || !pWndToDib || !pDibToWnd)
		return;

	const auto rCompose=r.getintersect({{0,0},{spDst->getwidth(),spDst->getheight()}});

	const int nSrcWidth=spSrc->getwidth();
	const int nSrcHeight=spSrc->getheight();
	const int nDstYFrom=rCompose.get(af2d::rect::tl).gety();
	const int nDstYInclusiveTo=rCompose.get(af2d::rect::br).gety()-1;
	const int nDstXFrom=rCompose.get(af2d::rect::tl).getx();
	const int nDstXInclusiveTo=rCompose.get(af2d::rect::br).getx()-1;
	
	for(int nDstY=nDstYFrom;nDstY<=nDstYInclusiveTo;++nDstY)
	{
		for(int nDstX=nDstXFrom;nDstX<=nDstXInclusiveTo;++nDstX)
		{
			double dSrcX, dSrcY;
			gettransformxy(pWndToDib,nDstX,nDstY,dSrcX,dSrcY);

			ASSERT(af::fpvalid(dSrcX));
			ASSERT(af::fpvalid(dSrcY));

			if((dSrcX<INT_MIN)||(dSrcX>INT_MAX))
				continue;

			if((dSrcY<INT_MIN)||(dSrcY>INT_MAX))
				continue;

			if(dSrcY<0)
				continue;
			const int nSrcY = af::posfloor<double,int>(dSrcY);
			if(nSrcY>=nSrcHeight)
				continue;

			if(dSrcX<0)
				continue;
			const int nSrcX = af::posfloor<double,int>(dSrcX);
			if(nSrcX>=nSrcWidth)
				continue;
		
			unsigned char *pDstScanline=spDst->getscanline(nDstY);

			const unsigned char *pSrcScanline=spSrc->getscanline(nSrcY);
			pDstScanline[nDstX*3+0]=pSrcScanline[nSrcX*3+0];
			pDstScanline[nDstX*3+1]=pSrcScanline[nSrcX*3+1];
			pDstScanline[nDstX*3+2]=pSrcScanline[nSrcX*3+2];
		}
	}
}

void dibwnd::getbkgndinfo(const int nFrom,const int nInclusiveTo,bkgndinfo& i)const
{
	i.nPixels = nInclusiveTo - nFrom + 1;
	i.nPixelFrom = nFrom;
	i.nChunkFrom = i.nPixelFrom / m_nChequerDim;
	i.nWholeChunks = i.nPixels / m_nChequerDim;
	i.nPartialChunkPixels = (nInclusiveTo+1) - ((i.nChunkFrom+i.nWholeChunks)*m_nChequerDim);
}

const unsigned char *dibwnd::getbkgndchunkcolour(const int nChunkX,const int nChunkY)const
{
	if(nChunkX % 2)
	{
		if(nChunkY % 2)
			return cWhite;
		return cGrey;
	}
	if(nChunkY % 2)
		return cGrey;
	return cWhite;
}
