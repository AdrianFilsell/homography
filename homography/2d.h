#pragma once

#include <vector>
#include "core.h"

namespace af2d
{

template <typename T=double> class point
{
public:
	point(){}
	point(const T x,const T y):m_X(x),m_Y(y){}
	virtual ~point(){}
	T getx(void)const{return m_X;}
	T gety(void)const{return m_Y;}
	void setx(const T d){m_X=d;}
	void sety(const T d){m_Y=d;}
	void offset(const T dX,const T dY){m_X+=dX;m_Y+=dY;}
	point& operator =(const point& o){m_X=o.m_X;m_Y=o.m_Y;return *this;}
protected:
	T m_X;
	T m_Y;
};

template <typename T> class pointvec
{
public:
	pointvec(){}
	pointvec(const std::vector<point<T>>& v):m_Vertices(v){}
	virtual ~pointvec(){}
	bool isempty(void)const{return m_Vertices.size()==0;}
	bool isconvex(void)const
	{
		if(m_Vertices.size()<3)
			return true;
		auto nA=m_Vertices.size()-2;
		auto nB=m_Vertices.size()-1;
		auto nC=0;
		auto dSignCheck = crossproduct(m_Vertices[nA],m_Vertices[nB],m_Vertices[nC]);
		bool b = dSignCheck>0.0;
		bool bHaveSign=dSignCheck!=0.0;

		nA=m_Vertices.size()-1,nB=0,nC=1;
		dSignCheck = crossproduct(m_Vertices[nA],m_Vertices[nB],m_Vertices[nC]);
		if(bHaveSign)
		{
			if((dSignCheck>0.0) != b)
				return false;
		}
		else
			bHaveSign=dSignCheck!=0.0;

		auto i=m_Vertices.cbegin(),end=m_Vertices.cend();
		--end;
		--end;
		for(;i!=end;++i)
		{
			auto dSignCheck = crossproduct(*i,*(i+1),*(i+2));
			if(bHaveSign)
			{
				if((dSignCheck>0.0) != b)
					return false;
			}
			else
				bHaveSign=dSignCheck!=0.0;
		}
		return true;
	}
	pointvec& operator =(const pointvec& o){m_Vertices=o.m_Vertices;return *this;}
protected:
	std::vector<point<T>> m_Vertices;
	const std::vector<point<T>>& get(void)const{return m_Vertices;}
	void set(const int n,const point<T>& p){m_Vertices[n]=p;}								// assume derived class gives correct index
	void offset(const int n,const point<T>& p){m_Vertices[n].offset(p.getx(),p.gety());}	// assume derived class gives correct index
	T crossproduct(const point<T>& a,const point<T>& b,const point<T>& c)const { return ((b.getx() - a.getx()) * (c.gety() - b.gety()) - (b.gety() - a.gety()) * (c.getx() - b.getx())); }
};

class rect : public pointvec<int>
{
public:
	enum vertex {tl=0,br=1};
	rect(){}
	rect(const point<int>& tl,const point<int>& br):pointvec<int>({tl,br}){}
	virtual ~rect(){}
	const point<int>& get(const vertex v)const{return pointvec<int>::get()[v];}
	bool ishorznormalised(void)const{return get(tl).getx()<=get(br).getx();}
	bool isvertnormalised(void)const{return get(tl).gety()<=get(br).gety();}
	rect getnormalised(void)const
	{
		if( isempty() )
			return (*this);

		return {{af::minval<int>(get(tl).getx(), get(br).getx() ),af::minval<int>( get(tl).gety(), get(br).gety() )},
				{af::maxval<int>(get(tl).getx(), get(br).getx() ), af::maxval<int>( get(tl).gety(), get(br).gety() )}};
	}
	rect getunion(const rect& other)const
	{
		if( isempty() )
			return other;
		if( other.isempty() )
		return (*this);
		
		const bool bHorzNormalisedA = ishorznormalised();
		const bool bVertNormalisedA = isvertnormalised();
		const bool bHorzNormalisedB = other.ishorznormalised();
		const bool bVertNormalisedB = other.isvertnormalised();

		if( bHorzNormalisedA && bHorzNormalisedB && bVertNormalisedA && bVertNormalisedB )
			return {{af::minval( get(tl).getx(), other.get(tl).getx() ), af::minval( get(tl).gety(), other.get(tl).gety() )},{af::maxval( get(br).getx(), other.get(br).getx() ), af::maxval( get(br).gety(), other.get(br).gety() )}};

		rect res = ( bHorzNormalisedA && bVertNormalisedA ) ? getunion(bHorzNormalisedB && bVertNormalisedB ? other : other.getnormalised()) :
															  getnormalised().getunion(bHorzNormalisedB && bVertNormalisedB ? other : other.getnormalised());
		if( !bHorzNormalisedA )
		{
			auto n = res.get(tl).getx();
			res.set(tl,{res.get(br).getx(),res.get(tl).gety()});
			res.set(br,{n,res.get(br).gety()});
		}
		if( !bVertNormalisedA )
		{
			auto n = res.get(tl).gety();
			res.set(tl,{res.get(tl).getx(),res.get(br).gety()});
			res.set(br,{res.get(br).getx(),n});
		}
		return res;
	}
	rect getintersect(const rect& other)const
	{
		rect r;

		if( isempty() || other.isempty() )
			return r;
	
		const bool bHorzNormalisedA = ishorznormalised();
		const bool bVertNormalisedA = isvertnormalised();
		const bool bHorzNormalisedB = other.ishorznormalised();
		const bool bVertNormalisedB = other.isvertnormalised();

		if( bHorzNormalisedA && bHorzNormalisedB && bVertNormalisedA && bVertNormalisedB )
		{
			// if inclusive then checking for an intersect uses the 'interior' therfore this will NOT detect a 'shared' edge
			bool bIntersect = true;
			if( other.get(tl).gety() >= get(br).gety() )
				bIntersect = false;
			else
			if( get(tl).gety() >= other.get(br).gety() )
				bIntersect = false;
			else
			if( other.get(tl).getx() >= get(br).getx() )
				bIntersect = false;
			else
			if( get(tl).getx() >= other.get(br).getx() )
				bIntersect = false;
			if( bIntersect )
				return {{af::maxval<int>( get(tl).getx(), other.get(tl).getx()),af::maxval<int>( get(tl).gety(), other.get(tl).gety())},
						{af::minval<int>( get(br).getx(), other.get(br).getx()),af::minval<int>( get(br).gety(), other.get(br).gety())}};
			return r;
		}

		if( bHorzNormalisedA && bVertNormalisedA )
			r = getintersect( bHorzNormalisedB && bVertNormalisedB ? other : other.getnormalised() );
		else
			r = getnormalised().getintersect( bHorzNormalisedB && bVertNormalisedB ? other : other.getnormalised() );
		if( !bHorzNormalisedA )
			r={{r.get(br).getx(),r.get(tl).gety()},{r.get(tl).getx(),r.get(br).gety()}};
		if( !bVertNormalisedA )
			r={{r.get(tl).getx(),r.get(br).gety()},{r.get(br).getx(),r.get(tl).gety()}};
		return r;
	}
	void set(const vertex v,const point<int>& p){pointvec<int>::set(v,p);}
	void offset(const point<int>& p){offset(tl,p);offset(br,p);}
	void offset(const vertex v,const point<int>& p){pointvec<int>::offset(v,p);}
	rect& operator =(const rect& o){pointvec<int>::operator=(o);return *this;}

	static void getrectscale(const double dSrcTLX,const double dSrcTLY,const double dSrcBRX,const double dSrcBRY,
							 const double dDstTLX,const double dDstTLY,const double dDstBRX,const double dDstBRY,
							 const bool bLetterBox,double& dS)
	{
		const double dDstWidth = dDstBRX-dDstTLX;
		const double dDstHeight = dDstBRY-dDstTLY;
		const double dSrcWidth = dSrcBRX-dSrcTLX;
		const double dSrcHeight = dSrcBRY-dSrcTLY;
		const double x = dDstWidth / dSrcWidth, y = dDstHeight / dSrcHeight;	
		if( bLetterBox )
		{
			// make src as large as possible while keeping both sides of src within dst
			const double min = x < y ? x : y;
			dS=min;
		}
		else
		{
			// make src as large as possible while keeping smallest side of src within dst
			const double max = x < y ? y : x;
			dS=max;
		}
	}
};

template <typename T=double> class quad : public pointvec<T>
{
public:
	enum vertex {tl=0,tr=1,br=2,bl=3};
	quad(){}
	quad(const point<T>& tl,const point<T>& tr,const point<T>& br,const point<T>& bl):pointvec<T>({tl,tr,br,bl}){}
	virtual ~quad(){}
	const point<T>& get(const vertex v)const{return pointvec<T>::get()[v];}
	void set(const vertex v,const point<T>& p){pointvec<T>::set(v,p);}
	quad& operator =(const quad& o){pointvec<T>::operator=(o);return *this;}
};

}
