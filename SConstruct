# -*- coding: utf-8 -*-

import os
import glob
import string
import platform


AddOption("--with-randomdevice", dest="withrandom", type="string", nargs=0, action="store", help="installation with random device support")
AddOption("--with-mpi", dest="withmpi", type="string", nargs=0, action="store", help="installation with MPI support")
AddOption("--with-multilanguage", dest="withmultilanguage", type="string", nargs=0, action="store", help="installation with multilanguage support")
AddOption("--with-sources", dest="withsources", type="string", nargs=0, action="store", help="installation with source like nntp or something else")
AddOption("--with-files", dest="withfiles", type="string", nargs=0, action="store", help="installation with file reading support for CSV & HDF")
AddOption("--create-language", dest="createlang", type="string", nargs=0, action="store", help="reads the data for translation and add them to the different language files")
AddOption("--compile-language", dest="compilelang", type="string", nargs=0, action="store", help="compiles the language files")
AddOption("--create-documentation", dest="createdocu", type="string", nargs=0, action="store", help="creates the doxygen documentation (doxygen must be within the path)")


#=== function for os configuration ===================================================================================================
# configuration for OSX build
def configuration_macosx(config, version, architecture) :
    # check the OSX build for set the correct architecture
    arch = architecture
    ver  = version.split(".")
    
    if ver[0] == "10" and ver[1] == "6" :
        arch = "x86_64"
    
    
    config["linkerflags"]       = ""
    config["include"]           = os.environ["CPPPATH"]
    config["librarypath"]       = os.environ["LIBRARY_PATH"]
    config["compileflags"]      = "-O2 -pipe -Wall -pthread -finline-functions -arch "+arch+" -D NDEBUG -D BOOST_UBLAS_NDEBUG -D BOOST_NUMERIC_BINDINGS_BLAS_CBLAS"
    config["linkto"]            = ["boost_system", "boost_thread", "boost_iostreams", "ginac", "atlas", "lapack", "ptcblas"]
    
    
    if optionExist("withmpi") :
        config["compiler"]          = "mpic++"
        config["compileflags"]      += " -D ML_CLUSTER"
        config["linkto"].extend( ["boost_mpi", "boost_serialization"] )
    else :
        config["compiler"]          =  "g++"

                
    if optionExist("withrandom") :   
        config["compileflags"]      += " -D ML_RANDOMDEVICE"
        config["linkto"].append("boost_random");
            
    if optionExist("withmultilanguage") :
        config["compileflags"]      += " -D ML_MULTILANGUAGE"
        config["linkto"].append("intl");
        
    if optionExist("withsources") :
        config["compileflags"]      += " -D ML_SOURCES"
        config["linkto"].extend( ["xml2", "boost_regex"] )
        
    if optionExist("withfiles") :
        config["compileflags"]      += " -D ML_FILES"
        config["linkto"].extend( ["hdf5_cpp", "hdf5"] )

    

# configuration for Posix (Linux) build
def configuration_posix(config, version, architecture) :
    config["linkerflags"]       = ""
    config["include"]           = os.environ["CPPPATH"]
    config["librarypath"]       = os.environ["LIBRARY_PATH"]
    config["compileflags"]      = "-O2 -pipe -Wall -pthread -finline-functions -D NDEBUG -D BOOST_UBLAS_NDEBUG -D BOOST_NUMERIC_BINDINGS_BLAS_CBLAS"
    config["linkto"]            = ["boost_system", "boost_thread", "boost_iostreams", "ginac", "atlas", "lapack", "ptcblas", "ptf77blas"]
    
    
    if optionExist("withmpi") :
        config["compiler"]          = "mpic++"
        config["compileflags"]      += " -D ML_CLUSTER"
        config["linkto"].extend( ["boost_mpi", "boost_serialization"] )
    else :
        config["compiler"]          =  "g++"

                
    if optionExist("withrandom") :   
        config["compileflags"]      += " -D ML_RANDOMDEVICE"
        config["linkto"].append("boost_random");
            
    if optionExist("withmultilanguage") :
        config["compileflags"]      += " -D ML_MULTILANGUAGE"
        config["linkto"].append("intl");
        
    if optionExist("withsources") :
        config["compileflags"]      += " -D ML_SOURCES"
        config["linkto"].extend( ["xml2", "boost_regex"] )
        
    if optionExist("withfiles") :
        config["compileflags"]      += " -D ML_FILES"
        config["linkto"].extend( ["hdf5_cpp", "hdf5"] )
        
        

# configuration for Windows build
def configuration_win32(config, version, architecture) :
    config = []
#=======================================================================================================================================



#=== function for configuration creation================================================================================================
# checks if option exists and returns a boolean
def optionExist(name) :
    return GetOption(name) != None


# function for reading os configuration
# and setting environment
def getConfig():
    env = Environment()
    config = {}
    
    if env['PLATFORM'].lower() == "darwin" :
        configuration_macosx(config, platform.mac_ver()[0], platform.machine())
    elif env['PLATFORM'].lower() == "win32" :
        configuration_win32(config, "", platform.machine())
    elif env['PLATFORM'].lower() == "posix" :
        configuration_posix(config, "", platform.machine())
    else :
        print "configuration for ["+env['PLATFORM']+"] not exists"
        exit(1) 
    
    if not(config) :
        print "Configuration is empty"
        exit(1)

    for i in [ "compiler", "compileflags", "linkerflags", "include", "librarypath", "linkto" ] :
        if not(config.has_key(i)) :
            print "field ["+i+"] is not set in the configuration"
            exit(1)

    for i in [ "compiler", "compileflags", "linkerflags", "include", "librarypath" ] :
        if type(config[i]) <> type("str") :
            print "field ["+i+"] must be a string type"
            exit(1)

    for i in [ "linkto" ] :
        if type(config[i]) <> type([]) :
            print "field ["+i+"] must be a list type"
            exit(1)

    if not(config["compiler"]) :
        print "no compiler is set, using default"
        if not(env["CXX"]) :
            print "no compiler set"
            exit(1)
    else :
        env.Replace(CXX         = config["compiler"])
    
    env.Replace(CXXFLAGS    = config["compileflags"])
    env.Replace(CPPPATH     = config["include"].split(os.pathsep))
    env.Replace(LINKFLAGS   = config["linkerflags"])
    env.Replace(LIBS        = config["linkto"])
    env.Replace(LIBPATH     = config["librarypath"])
    env.Replace(CPPSUFFIXES = [".hpp", ".h", ".cpp"])
           
    env.BuildDir("build", ".", duplicate=0)
    env.Append(CPPPATH=["."])
           
    #dict = env.Dictionary()
    #for i,j in dict.iteritems():
    #    print i, j		   
    
    return env
    
    
       
# get rekursiv all files
def getRekusivFiles(startdir, ending, pdontuse=[], pShowPath=True, pAbsPath=False) :
    lst = []
    
    for root, dirs, filenames in os.walk(startdir) :
        for filename in filenames :
            if filename.endswith(ending) :
                if pShowPath :
                    
                    if pAbsPath :
                        lst.append( os.path.abspath(os.path.join(root, filename)) )  
                    else :
                        lst.append( os.path.join(root, filename) )
                else :
                    
                    if pAbsPath :
                        lst.append(os.path.abspath(filename))
                    else :
                        lst.append(filename)

    clst = []
    if not pdontuse :
        clst.extend(lst)
    else :
        ldontuse = [os.path.join(startdir, i) for i in pdontuse]
        for n in ldontuse :
            for i in lst :
                if not(i.startswith(n)) :
                    clst.append(i)

    return clst
        
       
# build languagefiles
def createLanguage(env, onlycompile=False) :
    # compiling with: msgfmt -v -o target.mo source.po
    # add new data: msgmerge --no-wrap --update old_file.po newer_file.pot

    po = getRekusivFiles(os.curdir, ".po", ["examples"])
    
    if onlycompile :
        # compiling all files
        for i in po :
            os.system( "msgfmt -v -o " + os.path.join(os.path.dirname(i),"ml.mo") +" "+ i )
        
    else :
        sources = []
        for i in env["CPPSUFFIXES"] :
            sources.extend( getRekusivFiles(os.curdir, i, ["examples"]) )

        cmd = "xgettext --output="+os.path.join("tools", "language", "language.po")+" --keyword=_ --language=c++ ";
        for i in sources :
            cmd = cmd + i + " "
        os.system(cmd);
        
        
        # get all language files in the subdirs and add the new texts
        for i in po :
            os.system( "msgmerge --no-wrap --update " + i + " "+os.path.join("tools", "language", "language.po") )
            
        os.remove("tools/language/language.po")
        
        # compiling all files
        for i in po :
            os.system( "msgfmt -v -o " + os.path.join(os.path.dirname(i),"ml.mo") +" "+ i )
    

#=======================================================================================================================================

#=== build targets =====================================================================================================================

# create compiling options
def createTarget(env, alias, path, sources, framework) :
    lst = []
    if sources.count == 0 :
        return
        
    for i in sources :
        compilesource = [os.path.join(path, i)]
        compilesource.extend(framework)
        
        lst.append( env.Program( target=os.path.join("#build", os.path.splitext(i)[0]), source=compilesource ) )

    env.Alias(alias, lst)




def target_sources(env, framework) :
    if not optionExist("withsources") :
        return

    path = os.path.join(".", "examples", "sources")
    sources = ["newsgroup.cpp", "wikipedia.cpp"]
    
    if optionExist("withfiles") :
        sources.append( "cloud.cpp" )

    createTarget(env, "sources", path, sources, framework)
    
    
def target_clustering(env, framework) :
    path = os.path.join(".", "examples", "clustering")
    sources = []
    
    if optionExist("withfiles") :
        sources.extend( ["kmeans.cpp", "neuralgas.cpp"] )

    createTarget(env, "clustering", path, sources, framework)
    
    
def target_reducing(env, framework) :
    path = os.path.join(".", "examples", "reducing")
    sources = []
    
    if optionExist("withfiles") :
        sources.extend( ["lda.cpp", "mds.cpp", "pca.cpp"] )

    createTarget(env, "reducing", path, sources, framework)
    

def target_distance(env, framework) :
    path = os.path.join(".", "examples", "distance")
    sources = []

    if optionExist("withfiles") :
        sources.extend( ["ncd.cpp"] )

    createTarget(env, "distance", path, sources, framework)
#=======================================================================================================================================





#=== create environment and compiling ==================================================================================================
env = getConfig()

# add files for deleting
files = []
files.extend( getRekusivFiles(os.curdir, env["OBJSUFFIX"]) )
files.extend( getRekusivFiles(os.curdir, ".po~") )
env.Clean("clean", files)

# get all cpp-files and compile and create language file
if optionExist("createlang") :
    createLanguage(env)
elif optionExist("compilelang") :
    createLanguage(env, True)
elif optionExist("createdocu") :
    os.system("doxygen documentation.doxyfile")
else :
        
    # catch all cpps within the framework directories and compile them to objectfiles into the builddir
    framework = getRekusivFiles(os.curdir, ".cpp", ["examples"])  

    # create building targets
    target_sources( env, framework )
    target_clustering( env, framework )
    target_reducing( env, framework )
    target_distance(env, framework )


    