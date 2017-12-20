/*!
 * Distance.h
 *
 *  Created on: 2012/12/29
 *  \author Hiroki Sudo
 *
 */

#ifndef DISTANCE_H_
#define DISTANCE_H_

#include <vector>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/shared_ptr.hpp>
#include "Element.h"

/*!
 *
 */
namespace {
/*!
 * Distance
 */
namespace Distance {

/*!
 * ElementクラスのVectorインスタンスを引数に2つ取り、それぞれの編集距離(Levenstein距離)を計算します。
 * 返値は引数に渡されるVectorインスタンスの要素数で割られるため、0~1の値を取ります。
 * 値が大きければ距離は離れていて、小さければ距離が近いと言う意味になります。計算量はO((n*m)^(1/2))
 */
template<class E>
double levenstein(std::vector<E> ex1, std::vector<E> ex2) {
    
	boost::numeric::ublas::matrix<int> matrix(0, 0);
	int col_size, row_size;
	int cost_delta, cost1, cost2, cost3, cost;

	if (ex1.size() == 0 && ex2.size() != 0)
		return ex2.size();
	if (ex1.size() != 0 && ex2.size() == 0)
		return ex1.size();
	if (ex1.size() == 0 && ex2.size() == 0)
		return 0;

	row_size = ex1.size() + 1;
	col_size = ex2.size() + 1;

	matrix.resize(row_size, col_size);

	for (int j = 0; j < col_size; j++) {
		matrix(0, j) = j;
	}
	for (int i = 0; i < row_size; i++) {
		matrix(i, 0) = i;
	}

	for (int index_ex1 = 0; index_ex1 < ex1.size(); index_ex1++) {
		for (int index_ex2 = 0; index_ex2 < ex2.size(); index_ex2++) {

			if (ex1[index_ex1].is_sym() && ex2[index_ex2].is_sym()) {
				cost_delta = ex1[index_ex1] == ex2[index_ex2] ? 0 : 1;
			} else if (ex1[index_ex1].is_cat() && ex2[index_ex2].is_cat()) {
				cost_delta = ex1[index_ex1].obj == ex2[index_ex2].obj ? 0 : 1;
			} else {
				cost_delta = 1;
			}

			int x, y;
			y = index_ex1 + 1;
			x = index_ex2 + 1;
			cost1 = matrix(y - 1, x) + 1;
			cost2 = matrix(y, x - 1) + 1;
			cost3 = matrix(y - 1, x - 1) + cost_delta;

			cost = cost1;
			cost = cost2 < cost ? cost2 : cost;
			cost = cost3 < cost ? cost3 : cost;

			matrix(y, x) = cost;
		}
	}

	int len;
	if (row_size > col_size)
		len = row_size;
	else
		len = col_size;

	return (matrix(matrix.size1() - 1, matrix.size2() - 1))
			/ (static_cast<double>(len));
}

/*!
 * Vectorインスタンスの要素数で割らないため，値は0~無限となるLevenstein距離
 */
template<class E>
double levenstein2(std::vector<E> ex1, std::vector<E> ex2) {
    
	boost::numeric::ublas::matrix<int> matrix(0, 0);
	int col_size, row_size;
	int cost_delta, cost1, cost2, cost3, cost;

	if (ex1.size() == 0 && ex2.size() != 0)
		return ex2.size();
	if (ex1.size() != 0 && ex2.size() == 0)
		return ex1.size();
	if (ex1.size() == 0 && ex2.size() == 0)
		return 0;

	row_size = ex1.size() + 1;
	col_size = ex2.size() + 1;

	matrix.resize(row_size, col_size);

	for (int j = 0; j < col_size; j++) {
		matrix(0, j) = j;
	}
	for (int i = 0; i < row_size; i++) {
		matrix(i, 0) = i;
	}

	for (int index_ex1 = 0; index_ex1 < ex1.size(); index_ex1++) {
		for (int index_ex2 = 0; index_ex2 < ex2.size(); index_ex2++) {

			if (ex1[index_ex1].is_sym() && ex2[index_ex2].is_sym()) {
				cost_delta = ex1[index_ex1] == ex2[index_ex2] ? 0 : 1;
			} else if (ex1[index_ex1].is_cat() && ex2[index_ex2].is_cat()) {
				cost_delta = ex1[index_ex1].obj == ex2[index_ex2].obj ? 0 : 1;
			} else {
				cost_delta = 1;
			}

			int x, y;
			y = index_ex1 + 1;
			x = index_ex2 + 1;
			cost1 = matrix(y - 1, x) + 1;
			cost2 = matrix(y, x - 1) + 1;
			cost3 = matrix(y - 1, x - 1) + cost_delta;

			cost = cost1;
			cost = cost2 < cost ? cost2 : cost;
			cost = cost3 < cost ? cost3 : cost;

			matrix(y, x) = cost;
		}
	}

	int len;
	if (row_size > col_size)
		len = row_size;
	else
		len = col_size;

	return (matrix(matrix.size1() - 1, matrix.size2() - 1));
}


template <class E>
int snake(int k, int y, std::vector<E >& ary1, std::vector<E >& ary2, int m, int n) {
	int x;
	x = y - k;
	while (x < m && y < n && ary1[x] == ary2[y]) {
		x++;
		y++;
	}

	return y;
}
/*!
 * O(np)法の実装
 */
template<class E>
double onp_lv(std::vector<E>& ary1, std::vector<E>& ary2, double limit = -1) {
	std::vector<E> temp;
	std::vector<int> fp;
	int size = 0;
	int delta = 0;
	int offset = 0;
	int n = 0;
	int m = 0;
	int p;
	int k;
	int i;

	if (ary1.size() > ary2.size()) {
		temp = ary1;
		ary1 = ary2;
		ary2 = temp;
	}

	m = ary1.size();
	n = ary2.size();

	offset = m + 1;
	delta = n - m;
	size = m + n + 3;

	if (limit == -1) {
		limit = 2 * m + delta;
	}

	fp = std::vector<int>(size, -1);
	p = -1;
	k = 0;

	while (fp[delta + offset] < n) {
		p++;

		if (2 * p + delta >= (limit) * ary2.size() ) {
			//std::cerr << "Cutted" << std::endl;
			return 1.0;
		}

		k = -p;
		while (k < delta) {
			fp[k + offset] = fp[k - 1 + offset] + 1 > fp[k + 1 + offset] ?
				snake(k, fp[k - 1 + offset] + 1, ary1, ary2, m, n) :
				snake(k, fp[k + 1 + offset], ary1, ary2, m, n);
			k++;
		}

		k = delta + p;
		while (k > delta) {
			fp[k + offset] = fp[k - 1 + offset] + 1 > fp[k + 1 + offset] ?
				snake(k, fp[k - 1 + offset] + 1, ary1, ary2, m, n) :
				snake(k, fp[k + 1 + offset], ary1, ary2, m, n);
			k--;
		}

		fp[delta + offset] = fp[delta - 1 + offset] + 1 > fp[delta + 1 + offset] ?
				snake(delta, fp[delta - 1 + offset] + 1, ary1, ary2, m,	n) :
				snake(delta, fp[delta + 1 + offset], ary1, ary2, m, n);
	}

	return (delta + 2 * p)/(static_cast<double>(ary2.size()));
}

template<class E> 
double hamming(std::vector<E> ex1, std::vector<E> ex2){
        
    if(ex1.size()!=ex2.size()){
        std::cerr <<  "Happened error on hamming distance method." << std::endl;
        std::exit(0);
    }
    typename std::vector<E>::iterator ex1_it;

    ex1_it = ex1.begin();
    int cnt = 0;
    double ham_sum=0;
    for(; ex1_it != ex1.end(); ex1_it++) {
        
        if((*ex1_it)!=ex2[cnt])
            ham_sum+=1;
        
        cnt+=1;
    }
    return (ham_sum/(double)cnt);
}

} /*Distance*/
}
#endif /* DISTANCE_H_ */
