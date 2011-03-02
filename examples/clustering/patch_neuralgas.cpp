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

#include <algorithm>
#include <map>

#include <machinelearning.h>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/numeric/ublas/matrix.hpp>




namespace ublas     = boost::numeric::ublas;
namespace cluster   = machinelearning::clustering::nonsupervised;
namespace distance  = machinelearning::distances;
namespace tools     = machinelearning::tools;
#ifdef MACHINELEARNING_MPI
namespace mpi       = boost::mpi;
#endif


bool cliArguments( int argc, char* argv[], std::map<std::string, boost::any>& p_args ) {
    
    if (argc < 2) {
        std::cout << "--inputfile" << "\t" << "one or many input HDF5 file(s)" << std::endl;
        std::cout << "--inputpath" << "\t" << "one path (read in each file) or same number of pathes like input files" << std::endl;
        std::cout << "--outfile" << "\t" << "output HDF5 file" << std::endl;
        std::cout << "--iteration" << "\t" << "number of iteration" << std::endl;
        std::cout << "--prototype" << "\t" << "number of prototypes" << std::endl;
        std::cout << "--log" << "\t" << "'on' for enable logging" << std::endl;
        
        return false;
    }
    
    
    // set default arguments
    std::map<std::string, std::vector<std::string> > l_argmap;
    l_argmap["inputfile"] = std::vector<std::string>();
    l_argmap["inputpath"] = std::vector<std::string>();
    l_argmap["outfile"]   = std::vector<std::string>();
    l_argmap["iteration"] = std::vector<std::string>();
    l_argmap["prototype"] = std::vector<std::string>();
    l_argmap["log"]       = std::vector<std::string>();
    
    
    // read all arguments
    std::size_t n=1;
    for(int i=1; i < argc; i+=n) {
        n = 1;
        std::string lc(argv[i]);
        if (lc.length() < 2)
            continue;
        
        lc = lc.substr(2);
        boost::to_lower( lc );
        
        if (l_argmap.find(lc) != l_argmap.end()) {
            std::vector<std::string> lv;
            
            for(int l=i+1; l < argc; ++l) {
                std::string lc2(argv[l]);
                if ( (lc2.length() >= 2) && (lc2.substr(0,2) == "--") )
                    break;
                
                lv.push_back(lc2);
                n++;
            }
            
            l_argmap[lc] = lv;
        }
        
    }

    
    //check map values and convert them
    if ( (l_argmap["iteration"].size() != 1) || (l_argmap["prototype"].size() != 1) || (l_argmap["outfile"].size() != 1) || (l_argmap["inputfile"].size() == 0) || (l_argmap["inputpath"].size() == 0) )
        throw std::runtime_error("number of arguments are incorrect");

    #ifndef MACHINELEARNING_MPI
    if (!( ((l_argmap["inputpath"].size() == 1) && (l_argmap["inputfile"].size() > 0)) ||
           ((l_argmap["inputpath"].size() > 0) && (l_argmap["inputfile"].size() == 1)) ||
           (l_argmap["inputpath"].size() == l_argmap["inputfile"].size())
       )) 
        throw std::runtime_error("number of pathes / files are inclorrect");
    #endif
    
    p_args["log"]           = l_argmap["log"].size() > 0;
    p_args["inputfile"]     = l_argmap["inputfile"];
    p_args["inputpath"]     = l_argmap["inputpath"];
    p_args["outfile"]       = l_argmap["outfile"][0];
    
    try {
        p_args["iteration"] = boost::lexical_cast<std::size_t>( l_argmap["iteration"][0] );
        p_args["prototype"]  = boost::lexical_cast<std::size_t>( l_argmap["prototype"][0] );
    } catch (...) {
        throw std::runtime_error("numerical data can not extracted");
    }  

    return true;
}


int main(int argc, char* argv[]) {
    
    std::map<std::string, boost::any> l_args;
    if (!cliArguments(argc, argv, l_args))
        return EXIT_SUCCESS;
    
    #ifdef MACHINELEARNING_MPI
    mpi::environment loMPIenv(argc, argv);
    mpi::communicator loMPICom;
    
    // we check CPU size and number of files
    if ( static_cast<std::size_t>(loMPICom.size()) != boost::any_cast< std::vector<std::string> >(l_args["inputfile"]).size())
        throw std::runtime_error("number of process and number of source files must be equal");
    #endif

    
    
    // read input data
    std::vector<std::string> lafiles = boost::any_cast< std::vector<std::string> >(l_args["inputfile"]);
    std::vector<std::string> lapath  = boost::any_cast< std::vector<std::string> >(l_args["inputpath"]);
    
    
    
    #ifdef MACHINELEARNING_MPI
    tools::files::hdf source( lafiles[static_cast<std::size_t>(loMPICom.rank())] );
    #else
    tools::files::hdf source( lafiles[0] );
    #endif
    ublas::matrix<double> data = source.readMatrix<double>(lapath[0], H5::PredType::NATIVE_DOUBLE);
    
    
    
    // create ng objects
    distance::euclid<double> d;
    cluster::neuralgas<double> ng(d, boost::any_cast<std::size_t>(l_args["prototype"]), data.size2());
    ng.setLogging( boost::any_cast<bool>(l_args["log"]) );
    
    
    
    
    #ifdef MACHINELEARNING_MPI
    
    //create target file (only on the first process)
    tools::files::hdf* target = NULL;
    
    if (loMPICom.rank() == 0)
        target = new tools::files::hdf( boost::any_cast<std::string>(l_args["outfile"]), true );
    
    
    // do the first patch
    ng.trainpatch( loMPICom, data, boost::any_cast<std::size_t>(l_args["iteration"]) );
    
    ublas::vector<double> weights                   = ng.getPrototypeWeights(loMPICom);
    ublas::matrix<double> protos                    = ng.getPrototypes(loMPICom);
    ublas::vector<double> qerror                    = tools::vector::copy(ng.getLoggedQuantizationError(loMPICom));
    std::vector< ublas::matrix<double> > logproto   = ng.getLoggedPrototypes(loMPICom);
    
    if (target) {
        target->write<double>( "/patch0/weights",  weights, H5::PredType::NATIVE_DOUBLE );
        target->write<double>( "/patch0/protos",  protos, H5::PredType::NATIVE_DOUBLE );    
        
        if (ng.getLogging()) {
            target->write<double>( "/patch0/error", qerror, H5::PredType::NATIVE_DOUBLE );
            
            for(std::size_t i=0; i < logproto.size(); ++i)
                target->write<double>("/patch0/log" + boost::lexical_cast<std::string>( i )+"/protos", logproto[i], H5::PredType::NATIVE_DOUBLE );
        }
    }
    
    
    
    for (std::size_t i=1; i < lapath.size(); ++i) {
        data   = source.readMatrix<double>( lapath[i], H5::PredType::NATIVE_DOUBLE);
        
        ng.trainpatch( loMPICom, data, boost::any_cast<std::size_t>(l_args["iteration"]) );
        
        
        weights    = ng.getPrototypeWeights(loMPICom);
        protos     = ng.getPrototypes(loMPICom);
        qerror     = tools::vector::copy(ng.getLoggedQuantizationError(loMPICom));
        logproto   = ng.getLoggedPrototypes(loMPICom);
        
        if (target) {
            std::string patchpath = "/patch" + boost::lexical_cast<std::string>(i);
            
            target->write<double>( patchpath+"/weights",  weights, H5::PredType::NATIVE_DOUBLE );
            target->write<double>( patchpath+"/protos",  protos, H5::PredType::NATIVE_DOUBLE );    
            
            if (ng.getLogging()) {
                target->write<double>( patchpath+"/error", qerror, H5::PredType::NATIVE_DOUBLE );
                
                for(std::size_t j=0; j < logproto.size(); ++j)
                    target->write<double>( patchpath+"/log" + boost::lexical_cast<std::string>( j )+"/protos", logproto[j], H5::PredType::NATIVE_DOUBLE );
            }
        }
    }
    
 
    weights    = ng.getPrototypeWeights(loMPICom);
    protos     = ng.getPrototypes(loMPICom);
    if (target) {
        target->write<double>( "/numprotos",   boost::any_cast<std::size_t>(l_args["prototype"]), H5::PredType::NATIVE_DOUBLE );
        target->write<double>( "/protos",  protos, H5::PredType::NATIVE_DOUBLE );    
        target->write<double>( "/weights",  weights, H5::PredType::NATIVE_DOUBLE );    
        target->write<std::size_t>( "/iteration",   boost::any_cast<std::size_t>(l_args["iteration"]), H5::PredType::NATIVE_ULONG );
    }
    
    
    delete(target);
    
    
    
    #else
    
    //create target file
    tools::files::hdf target( boost::any_cast<std::string>(l_args["outfile"]), true );
    
    // do the first patch
    ng.trainpatch( data, boost::any_cast<std::size_t>(l_args["iteration"]) );
    
    target.write<double>( "/patch0/weights",  ng.getPrototypeWeights(), H5::PredType::NATIVE_DOUBLE );
    target.write<double>( "/patch0/protos",  ng.getPrototypes(), H5::PredType::NATIVE_DOUBLE );    
    
    if (ng.getLogging()) {
        target.write<double>( "/patch0/error",  tools::vector::copy(ng.getLoggedQuantizationError()), H5::PredType::NATIVE_DOUBLE );
        
        std::vector< ublas::matrix<double> > logproto =  ng.getLoggedPrototypes();
        for(std::size_t i=0; i < logproto.size(); ++i)
            target.write<double>("/patch0/log" + boost::lexical_cast<std::string>( i )+"/protos", logproto[i], H5::PredType::NATIVE_DOUBLE );
    }
    
    
    // do the rest patches
    for (std::size_t i=1; i < std::max(lafiles.size(), lapath.size()); ++i) {
        source.open( (lafiles.size() == 1) ? lafiles[0] : lafiles[i] );
        data   = source.readMatrix<double>( (lapath.size() == 1) ? lapath[0] : lapath[i], H5::PredType::NATIVE_DOUBLE);
        
        ng.trainpatch( data, boost::any_cast<std::size_t>(l_args["iteration"]) );

        std::string patchpath = "/patch" + boost::lexical_cast<std::string>(i);
        target.write<double>( patchpath+"/weights",  ng.getPrototypeWeights(), H5::PredType::NATIVE_DOUBLE );
        target.write<double>( patchpath+"/protos",  ng.getPrototypes(), H5::PredType::NATIVE_DOUBLE );    
        
        if (ng.getLogging()) {
            target.write<double>( patchpath+"/error",  tools::vector::copy(ng.getLoggedQuantizationError()), H5::PredType::NATIVE_DOUBLE );
            
            std::vector< ublas::matrix<double> > logproto =  ng.getLoggedPrototypes();
            for(std::size_t j=0; j < logproto.size(); ++j)
                target.write<double>( patchpath+"/log"+boost::lexical_cast<std::string>(j)+"/protos", logproto[j], H5::PredType::NATIVE_DOUBLE );
        }
    }

    target.write<double>( "/numprotos",   boost::any_cast<std::size_t>(l_args["prototype"]), H5::PredType::NATIVE_DOUBLE );
    target.write<double>( "/protos",  ng.getPrototypes(), H5::PredType::NATIVE_DOUBLE );    
    target.write<double>( "/weights",  ng.getPrototypeWeights(), H5::PredType::NATIVE_DOUBLE );    
    target.write<std::size_t>( "/iteration",   boost::any_cast<std::size_t>(l_args["iteration"]), H5::PredType::NATIVE_ULONG );
    
    #endif
    
    
    
    
    // show info
    #ifdef MACHINELEARNING_MPI
    if (loMPICom.rank() == 0) {
    #endif
    
    std::cout << "structure of the output file" << std::endl;
    std::cout << "/numprotos" << "\t\t" << "number of prototypes" << std::endl;
    std::cout << "/protos" << "\t\t" << "prototype matrix (row orientated)" << std::endl;
    std::cout << "/weight" << "\t\t" << "prototype weight" << std::endl;
    std::cout << "/iteration" << "\t\t" << "number of iterations" << std::endl;
    
    if (ng.getLogging()) {
        std::cout << "/patch<0 to number of patches-1>/weights" << "\t\t" << "prototype weights after calculating patch" << std::endl;
        std::cout << "/patch<0 to number of patches-1>/protos" << "\t\t" << "prototypes after calculating patch" << std::endl;
        std::cout << "/patch<0 to number of patches-1>/error" << "\t\t" << "quantization error on each patch iteration" << std::endl;
        std::cout << "/patch<0 to number of patches-1>/log<0 to number of iteration-1>/protos" << "\t\t" << "prototypes on each patch iteration" << std::endl;
    }
    
    #ifdef MACHINELEARNING_MPI
    }
    #endif
        
    return EXIT_SUCCESS;
}