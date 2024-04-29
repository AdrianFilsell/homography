#pragma once

#include "2d.h"
#include "../Eigen/Dense"

namespace afhomography
{

template <typename T=double> class matrix
{
public:
	matrix(){}
	virtual ~matrix(){}
	static bool get(const af2d::quad<T>& from,const af2d::quad<T>& to,Eigen::Matrix<double,3,3>& homography)
	{
		// Function to compute homography matrix using DLT
		if(from.isempty()||to.isempty())
			return false;
		Eigen::Matrix<double,8,9> m;
		m.setConstant(0);
		set(from,to,af2d::quad<T>::tl,0,m);
		set(from,to,af2d::quad<T>::tr,2,m);
		set(from,to,af2d::quad<T>::br,4,m);
		set(from,to,af2d::quad<T>::bl,6,m);

		// For small matrice (<16), it is thus preferable to directly use JacobiSVD.
		// For larger ones, BDCSVD is highly recommended and can several order of magnitude faster - this algorithm is unlikely to provide accurate result when compiled with unsafe math optimizations.
		const Eigen::JacobiSVD<Eigen::MatrixXd> svd(m, Eigen::ComputeFullV);
		const Eigen::Matrix<double,9,9>& V = svd.matrixV();
		homography << V(0,8), V(1,8), V(2,8),
					  V(3,8), V(4,8), V(5,8),
					  V(6,8), V(7,8), V(8,8);

		return true;
	}
protected:
	static void set(const af2d::quad<T>& fromQ,const af2d::quad<T>& toQ,const af2d::quad<T>::template vertex v, int nRow, Eigen::Matrix<double,8,9>& m)
	{
		int nCol = 0;		
		const af2d::point<T>& f=fromQ.get(v);
		const af2d::point<T>& t=toQ.get(v);
		m.coeffRef(nRow,nCol++)=-f.getx();
		m.coeffRef(nRow,nCol++)=-f.gety();
		m.coeffRef(nRow,nCol++)=-1;
		nCol += 3;
		m.coeffRef(nRow,nCol++)=f.getx()*t.getx();
		m.coeffRef(nRow,nCol++)=f.gety()*t.getx();
		m.coeffRef(nRow,nCol)=t.getx();

		++nRow;
		nCol = 3;
		m.coeffRef(nRow,nCol++)=-f.getx();
		m.coeffRef(nRow,nCol++)=-f.gety();
		m.coeffRef(nRow,nCol++)=-1;
		m.coeffRef(nRow,nCol++)=f.getx()*t.gety();
		m.coeffRef(nRow,nCol++)=f.gety()*t.gety();
		m.coeffRef(nRow,nCol)=t.gety();
	}
};

}
