/** 
 @cond
 ############################################################################
 # LGPL License                                                             #
 #                                                                          #
 # This file is part of the Machine Learning Framework.                     #
 # Copyright (c) 2010, Philipp Kraus, <philipp.kraus@flashpixx.de>          #
 # This program is free software: you can redistribute it and/or modify     #
 # it under the terms of the GNU Lesser General Public License as           #
 # published by the Free Software Foundation, either version 3 of the       #
 # License, or (at your option) any later version.                          #
 #                                                                          #
 # This program is distributed in the hope that it will be useful,          #
 # but WITHOUT ANY WARRANTY; without even the implied warranty of           #
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            #
 # GNU Lesser General Public License for more details.                      #
 #                                                                          #
 # You should have received a copy of the GNU Lesser General Public License #
 # along with this program. If not, see <http://www.gnu.org/licenses/>.     #
 ############################################################################
 @endcond
 **/



#ifndef MACHINELEARNING_TEXTPROCESS_STOPWORDREDUCTION_H
#define MACHINELEARNING_TEXTPROCESS_STOPWORDREDUCTION_H

#include <string>

#include "../exception/exception.h"



namespace machinelearning { namespace textprocess {
    
    
    class stopwordreduction {
        
        /**
         **/
        public:
        
            stopwordreduction( const std::vector<std::string>&, const bool& );
        
            remove( std::string& ) const;
            remove( std::vector<std::string>& ) const;
        
        
        private:
        
            /** vector with stop word **/
            const std::vector<std::string> m_stopwords;
            /** bool for case-sensitive / case-insensitive wordlist **/
            const bool m_caseinsensitive;
        
    };


};};