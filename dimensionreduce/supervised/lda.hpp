/** 
 @cond
 #########################################################################
 # GPL License                                                           #
 #                                                                       #
 # This file is part of the Machine Learning Framework.                  #
 # Copyright (c) 2010, Philipp Kraus, <philipp.kraus@flashpixx.de>       #
 # This program is free software: you can redistribute it and/or modify  #
 # it under the terms of the GNU General Public License as published by  #
 # the Free Software Foundation, either version 3 of the License, or     #
 # (at your option) any later version.                                   #
 #                                                                       #
 # This program is distributed in the hope that it will be useful,       #
 # but WITHOUT ANY WARRANTY; without even the implied warranty of        #
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
 # GNU General Public License for more details.                          #
 #                                                                       #
 # You should have received a copy of the GNU General Public License     #
 # along with this program.  If not, see <http://www.gnu.org/licenses/>. #
 #########################################################################
 @endcond
 **/


#ifndef MACHINELEARNING_DIMENSIONREDUCE_SUPERVISED_LDA_HPP
#define MACHINELEARNING_DIMENSIONREDUCE_SUPERVISED_LDA_HPP


#include <map>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include "../dimensionreduce.hpp"
#include "../../exception/exception.h"
#include "../../tools/tools.h"


namespace machinelearning { namespace dimensionreduce { namespace supervised {
    
    namespace ublas  = boost::numeric::ublas;

    
    
    /** class for projection the (Fisher) lineare discriminant analysis (LDA) **/
    template<typename T, typename L> class lda : public supervisedreduce<T,L> {
        
        public :
        
            lda( const std::size_t& );
            ublas::matrix<T> map( const ublas::matrix<T>&, const std::vector<L>& );
            ublas::matrix<T> getMapping( void ) const;
            std::size_t getDimension( void ) const;
        
        
        private :
        
            /** target dimension **/
            const std::size_t m_dim;
            /** matrix with projection vectors (row orientated) **/
            ublas::matrix<T> m_project;
    };
    
    
    /** constructor
     * @param p_dim dimension
    **/
    template<typename T, typename L> inline lda<T,L>::lda( const std::size_t& p_dim) :
        m_dim(p_dim),
        m_project()
    {
        if (p_dim == 0)
            throw exception::parameter(_("dimension must be greater than zero"));
    }
    
    
    /** returns the target dimensione size
     * @return number of dimension
     **/
    template<typename T, typename L> inline std::size_t lda<T, L>::getDimension( void ) const
    {
        return m_dim;
    }
    
    
    /** returns the project vectors (largest eigenvectors)
     * @return matrix with eigenvector
     **/
    template<typename T, typename L> inline ublas::matrix<T> lda<T, L>::getMapping( void ) const
    {
        return m_project;
    }
    
    
    /** caluate and project the input data
     * @param p_data input datamatrix
     * @param p_label labeling for matrix rows
     * @return matrix with mapped points
     **/
    template<typename T, typename L> inline ublas::matrix<T> lda<T, L>::map( const ublas::matrix<T>& p_data, const std::vector<L>& p_label )
    {
        if (p_data.size1() != p_label.size())
            throw exception::matrix(_("matrix rows and label size are not equal"));
        
        
        // create a unique label vector
        std::vector<L> l_uniquelabel = tools::vector::unique<L>(p_label);
        
        // we can only reduce to length(classes)-1
        if (m_dim >= l_uniquelabel.size())
            throw exception::parameter(_("target dimension must be less than unique data classes"));
        
		
        // centering the data
        ublas::matrix<T> l_center = tools::matrix::centering<T>(p_data);		
		
		// we create a map for indexing the rows of the matrix with their labels
		std::multimap<L, std::size_t> l_index;
		for(std::size_t i=0; i < p_label.size(); ++i)
			l_index.insert( std::make_pair(p_label[i], i) );

        // calculate covarianz for every class and all data
        ublas::matrix<T> l_st = tools::matrix::cov(l_center);
        ublas::matrix<T> l_sw(l_st.size1(), l_st.size2());
		const T l_classes = static_cast<T>(l_uniquelabel.size()-1);
	
        for(std::size_t i=0; i < l_uniquelabel.size(); ++i) {
            
            
			// extract index from map | typename must be first, because is a exotic structure from C++ :-)
            // create a matrix whitch holds the rows for every class. Create a matrix with index elements for the rows
            std::size_t n=0;
            ublas::matrix<T> l_cluster(  std::distance(l_index.lower_bound(l_uniquelabel[i]), l_index.upper_bound(l_uniquelabel[i])),  l_center.size2()  );
            for( typename std::multimap<L, std::size_t>::iterator it = l_index.lower_bound(l_uniquelabel[i]); it != l_index.upper_bound(l_uniquelabel[i]); ++it )
                ublas::row(l_cluster, n++) = ublas::row(l_center, it->second);
            
            // calculate covarianz for the cluster
            l_sw     += l_cluster.size1() / l_classes * tools::matrix::cov(l_cluster);
            
        }

        // calculate the eigenvalues & -vectors
        ublas::vector<T> l_eigenvalues;
        ublas::matrix<T> l_eigenvectors;
        tools::lapack::eigen<T>(l_st-l_sw, l_sw, l_eigenvalues, l_eigenvectors);

        
        // rank the eigenvalues
        const ublas::indirect_array< std::vector<std::size_t> > l_rank = tools::vector::rankIndex( l_eigenvalues );
        
        // create projection (largest eigenvectors correspondends with the largest eigenvalues -> last values in rank)
        m_project = ublas::matrix<T>( l_eigenvectors.size2(), m_dim );
        for(std::size_t i=0; i < m_dim; ++i)
            ublas::column(m_project, i) = ublas::column(l_eigenvectors, l_rank(l_rank.size()-i-1));
        
        return ublas::prod(p_data, m_project);
        
        
    }

};};};
#endif