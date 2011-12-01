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

# -*- coding: utf-8 -*-
import os
Import("*")

flags = {}

flags["CPPPATH"] = []
if os.environ.has_key("CPPPATH") :
    flags["CPPPATH"].extend(os.environ["CPPPATH"].split(os.pathsep))
elif os.environ.has_key("CPATH") :
    flags["CPPPATH"].extend(os.environ["CPATH"].split(os.pathsep))
    
flags["LIBPATH"] = []
if os.environ.has_key("LIBRARY_PATH") :
    flags["LIBPATH"].extend(os.environ["LIBRARY_PATH"].split(os.pathsep))
elif os.environ.has_key("LD_LIBRARY_PATH") :
    flags["LIBPATH"].extend(os.environ["LD_LIBRARY_PATH"].split(os.pathsep))
    
flags["CXXFLAGS"] = []
if os.environ.has_key("CXXFLAGS") :
    flags["CXXFLAGS"].extend(os.environ["CXXFLAGS"].split(" "))

flags["LINKFLAGS"] = []
if os.environ.has_key("LDFLAGS") :
    flags["LINKFLAGS"].extend(os.environ["LDFLAGS"].split(" "))
    


flags["CXXFLAGS"].extend(["-fopenmp", "-pipe", "-Wall", "-D BOOST_FILESYSTEM_NO_DEPRECATED", "-D BOOST_NUMERIC_BINDINGS_BLAS_CBLAS"])
flags["LINKFLAGS"].extend(["-fopenmp", "-pthread"])
flags["LIBS"]        = ["boost_system", "boost_iostreams", "boost_regex", "gfortran"]



if not("javac" in COMMAND_LINE_TARGETS) :
    flags["LIBS"].extend(["boost_program_options", "boost_exception", "boost_filesystem"])
else :
    flags["LINKFLAGS"].append("-Wl,--rpath=\\$$ORIGIN")
    
if env["atlaslink"] == "multi" :
    flags["LIBS"].extend( ["lapack", "ptcblas", "ptf77blas"] )
elif env["atlaslink"] == "single" :
    flags["LIBS"].extend( ["lapack", "cblas", "f77blas"] )

if env["linkstatic"] :
    flags["LIBS"].append("atlas")
else :
    if env["atlaslink"] == "multi" :
        flags["LIBS"].append("tatlas")
    elif env["atlaslink"] == "single" :
        flags["LIBS"].append("satlas")

if env["withdebug"] :
    flags["CXXFLAGS"].append("-g")
else :
    flags["CXXFLAGS"].extend(["-D NDEBUG", "-D BOOST_UBLAS_NDEBUG"])

if env["withmpi"] :
    flags["CXXFLAGS"].append("-D MACHINELEARNING_MPI")
    flags["LIBS"].extend( ["boost_mpi", "boost_serialization"] )

if env["withrandomdevice"] :
    flags["CXXFLAGS"].append("-D MACHINELEARNING_RANDOMDEVICE")
    flags["LIBS"].append("boost_random");

if env["withmultilanguage"] :
    flags["CXXFLAGS"].append("-D MACHINELEARNING_MULTILANGUAGE")
    flags["LIBS"].append("intl");

if env["withsources"] :
    flags["CXXFLAGS"].extend(["-D MACHINELEARNING_SOURCES", "-D MACHINELEARNING_SOURCES_TWITTER"])
    flags["LIBS"].extend( ["xml2", "json"] )

if env["withfiles"] :
    flags["CXXFLAGS"].extend(["-D MACHINELEARNING_FILES", "-D MACHINELEARNING_FILES_HDF"])
    flags["LIBS"].extend( ["hdf5_cpp", "hdf5"] )
    if env["linkstatic"] :
        flags["LIBS"].append("z")

if env["withsymbolicmath"] :
    flags["CXXFLAGS"].append("-D MACHINELEARNING_SYMBOLICMATH")
    flags["LIBS"].append("ginac")

if env["withoptimize"] :
    flags["CXXFLAGS"].extend(["-O2", "-fomit-frame-pointer", "-finline-functions", "-mtune="+env["cputype"]])
    if env["math"] == "sse3" :
        flags["CXXFLAGS"].extend(["-mfpmath=sse", "-msse3"])
    elif env["math"] == "sse" :
        flags["CXXFLAGS"].extend(["-mfpmath=sse", "-msse"])
    else :
        flags["CXXFLAGS"].extend(["-mfpmath=387"])

if env["withlogger"] :
    flags["CXXFLAGS"].append("-D MACHINELEARNING_LOGGER")
    flags["LIBS"].append("boost_thread")


# if the static option is set, replace all libraries with the static version
if env["linkstatic"] :
    dylink = []
    for i in flags["LIBS"] :
        found = env.FindFile(env["LIBPREFIX"]+i+env["LIBSUFFIX"], flags["LIBPATH"])
        if found <> None :
            flags["LINKFLAGS"].append(found)
        else :
            dylink.append(i);
    flags["LIBS"] = dylink

env.MergeFlags(flags)
