#pragma once

#include "pch.h"
#include "dib.h"
#include "2d.h"
#include "matrix.h"
#include <memory>

// dibwnd

class dibwnd : public CWnd
{
	DECLARE_DYNAMIC(dibwnd)

public:
	enum hittest {ht_tl,ht_tr,ht_br,ht_bl,
				  ht_tltr_mid,ht_trbr_mid,ht_blbr_mid,ht_tlbl_mid,
				  ht_null};
	enum type {t_from,t_to};
	dibwnd(const type t);
	virtual ~dibwnd();

	bool isfrom(void)const{return m_Type==t_from;}
	bool isto(void)const{return !isfrom();}
	bool isdragging(void)const{return m_htDrag!=ht_null;}
	std::shared_ptr<const afdib::dib> getdib(void)const{return m_spDib;}
	std::shared_ptr<const af2d::quad<>> getdibquad(void)const{return m_spDibQuad;}
	
	void setother(dibwnd *pOther){m_pOther=pOther;}
	void setdib(std::shared_ptr<const afdib::dib> sp);
	void setquad(std::shared_ptr<const af2d::quad<>> sp);
	void canceldrag(void);
protected:
	type m_Type;
	dibwnd *m_pOther;
	std::shared_ptr<const afdib::dib> m_spDib;
	std::shared_ptr<const af2d::quad<>> m_spDibQuad;
	std::shared_ptr<afdib::dib> m_spCanvas;
	static int m_nChequerDim;
	static int m_nDibBorderGap;
	static int m_nHandleDim;
	bool m_bCaptured;
	hittest m_ht;
	hittest m_htDrag;
	CPoint m_cpWndDragFrom;
	std::shared_ptr<const af2d::quad<>> m_spFromDragDibQuad;

	hittest getht(const CPoint& cpWnd)const;
	bool gethandles(CPoint (&dib)[8],CPoint (&wnd)[8])const;
	hittest gethittest(const CPoint& cpWnd)const;
	void setcursor(void)const;
	void setcapture(void);
	void releasecapture(void);
	void drag(const CPoint& cpWnd);

	std::shared_ptr<const Eigen::Affine2d> m_spWndToDib2d;
	std::shared_ptr<const Eigen::Affine2d> m_spDibToWnd2d;
	std::shared_ptr<const Eigen::Projective2d> m_spWndToDib2dProj;
	std::shared_ptr<const Eigen::Projective2d> m_spDibToWnd2dProj;

	afx_msg void OnPaint(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	struct bkgndinfo
	{
		int nPixelFrom;
		int nChunkFrom;
		int nWholeChunks;
		int nPartialChunkPixels;
		int nPixels;
	};

	static unsigned char cWhite[3];
	static unsigned char cGrey[3];
	void composebkgnd(const af2d::rect& r,std::shared_ptr<afdib::dib> spDst)const;
	void composebkgndrow(const bkgndinfo& horzinfo,const bkgndinfo& vertinfo, const int nChunk,std::shared_ptr<afdib::dib> spDst)const;
	void setbkgndrowpixels(const af2d::point<int>& p,const int nPixels,const unsigned char *pPixel,std::shared_ptr<afdib::dib> spDst)const;
	void getbkgndinfo(const int nFrom,const int nInclusiveTo,bkgndinfo& i)const;
	const unsigned char *getbkgndchunkcolour(const int nChunkX,const int nChunkY)const;
	void composedib(const Eigen::Affine2d *pDibToWnd,const Eigen::Affine2d *pWndToDib,const af2d::rect& r,std::shared_ptr<const afdib::dib> spSrc,std::shared_ptr<afdib::dib> spDst)const;
	void composedib(const Eigen::Projective2d *pDibToWnd,const Eigen::Projective2d *pWndToDib,const af2d::rect& r,std::shared_ptr<const afdib::dib> spSrc,std::shared_ptr<afdib::dib> spDst)const;

	void composetransforms2d(std::shared_ptr<const Eigen::Affine2d>& spWndToDib,std::shared_ptr<const Eigen::Affine2d>& spDibToWnd,std::shared_ptr<const afdib::dib> spCanvas,std::shared_ptr<const afdib::dib> spDib)const;
	void composetransforms2dproj(const Eigen::Matrix<double,3,3>& homography,std::shared_ptr<const Eigen::Projective2d>& spWndToDib,std::shared_ptr<const Eigen::Projective2d>& spDibToWnd,std::shared_ptr<const afdib::dib> spCanvas,std::shared_ptr<const afdib::dib> spDib)const;
	__forceinline af2d::quad<double> gettransformquad(const Eigen::Affine2d *pTrns,const af2d::quad<>& q)const
	{
		const auto tl=q.get(af2d::quad<>::tl);
		Eigen::Vector2d tlv=*pTrns*Eigen::Vector2d(tl.getx(),tl.gety());

		const auto tr=q.get(af2d::quad<>::tr);
		Eigen::Vector2d trv=*pTrns*Eigen::Vector2d(tr.getx(),tr.gety());

		const auto br=q.get(af2d::quad<>::br);
		Eigen::Vector2d brv=*pTrns*Eigen::Vector2d(br.getx(),br.gety());

		const auto bl=q.get(af2d::quad<>::bl);
		Eigen::Vector2d blv=*pTrns*Eigen::Vector2d(bl.getx(),bl.gety());

		return af2d::quad<>({{tlv.x(),tlv.y()},{trv.x(),trv.y()},{brv.x(),brv.y()},{blv.x(),blv.y()}});
	}
	__forceinline af2d::quad<double> gettransformquad(const Eigen::Projective2d *pTrns,const af2d::quad<>& q)const
	{
		const auto tl=q.get(af2d::quad<>::tl);
		Eigen::Vector3d tlv=*pTrns*Eigen::Vector3d(tl.getx(),tl.gety(),1);

		const auto tr=q.get(af2d::quad<>::tr);
		Eigen::Vector3d trv=*pTrns*Eigen::Vector3d(tr.getx(),tr.gety(),1);

		const auto br=q.get(af2d::quad<>::br);
		Eigen::Vector3d brv=*pTrns*Eigen::Vector3d(br.getx(),br.gety(),1);

		const auto bl=q.get(af2d::quad<>::bl);
		Eigen::Vector3d blv=*pTrns*Eigen::Vector3d(bl.getx(),bl.gety(),1);

		return af2d::quad<>({{tlv.x()/tlv.z(),tlv.y()/tlv.z()},{trv.x()/trv.z(),trv.y()/trv.z()},{brv.x()/brv.z(),brv.y()/brv.z()},{blv.x()/blv.z(),blv.y()/blv.z()}});
	}
	__forceinline void gettransformx(const Eigen::Affine2d *pTrns,const int nDstX,double& dSrcX)const
	{
		dSrcX=(nDstX*(*pTrns)(0,0))+(*pTrns)(0,2);
	}
	__forceinline void gettransformy(const Eigen::Affine2d *pTrns,const int nDstY,double& dSrcY)const
	{
		dSrcY=(nDstY*(*pTrns)(1,1))+(*pTrns)(1,2);
	}
	__forceinline void gettransformxy(const Eigen::Affine2d *pTrns,const int nDstX,const int nDstY,double& dSrcX,double& dSrcY)const
	{
		const Eigen::Vector2d v(nDstX+0.5,nDstY+0.5);
		dSrcX=(*pTrns*v).x();
		dSrcY=(*pTrns*v).y();
	}
	__forceinline void gettransformxy(const Eigen::Projective2d *pTrns,const int nDstX,const int nDstY,double& dSrcX,double& dSrcY)const
	{
		const Eigen::Vector3d v(nDstX+0.5,nDstY+0.5,1);
		const auto t=(*pTrns*v);
		dSrcX=t.x()/t.z();
		dSrcY=t.y()/t.z();
	}
};
