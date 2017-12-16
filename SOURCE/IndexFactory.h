/*
 * CategoryFactory.h
 *
 *  Created on: 2011/05/20
 *      Author: Hiroki Sudo
 */

#ifndef INDEXFACTORY_H_
#define INDEXFACTORY_H_
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>


/*!
 * ユニークな番号を生成するクラスです。生成可能な範囲は、int型の範囲に依存します。
 *
 * boost/serializationに対応しています。
 */
class IndexFactory {
public:
	int index_counter;
	IndexFactory():index_counter(0){};

	/*!
	 * 数字を生成します。
	 * \code
	 * IndexFactory idxf;
	 * idxf.genenrate();
	 * \endcode
	 */
	int generate(void);

	IndexFactory& operator=(const IndexFactory& dst);
private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int /* file_version */){
    	ar & BOOST_SERIALIZATION_NVP(index_counter);
    }
};

#endif /* INDEXFACTORY_H_ */
