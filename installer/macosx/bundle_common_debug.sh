function add-plugin {
    plugin=$1
    echo "Registering plugin: ${plugin}"

    PLUGIN_LIB="lib${plugin}d.dylib"
    PLUGIN_DESC="${plugin}d.plugin"

    if [ ! -f ${DEBUG_DIR}/plugins/${PLUGIN_LIB} ] ;  
    then  
        echo "Plugin library file not found: ${PLUGIN_LIB} !"
        exit 1
    fi

    if [ ! -f ${DEBUG_DIR}/plugins/${PLUGIN_DESC} ] ; 
    then
        echo "Plugin descriptor file not found: ${PLUGIN_DESC} !"
        exit 1
    fi
    
    cp "${DEBUG_DIR}/plugins/${PLUGIN_LIB}"  "${TARGET_EXE_DIR}/plugins/"
    cp "${DEBUG_DIR}/plugins/${PLUGIN_DESC}" "${TARGET_EXE_DIR}/plugins/"
    #changeQtInstallNames "plugins/${PLUGIN_LIB}"
    changeCoreInstallNames "plugins/${PLUGIN_LIB}"
}

function add-library {
    lib=$1
    echo "Adding lib: ${lib}"

    LIB_FILE="lib${lib}d.1.dylib"

    if [ ! -f ${DEBUG_DIR}/${LIB_FILE} ] ;  
    then  
        echo "Library file not found: ${LIB_FILE} !"
        exit 1
    fi

    cp "${DEBUG_DIR}/${LIB_FILE}"  "${TARGET_EXE_DIR}/"
    #changeQtInstallNames "${LIB_FILE}"
    changeCoreInstallNames "${LIB_FILE}"		
}


#This function sets correct relative pathes for linking UGENE core libraries
changeCoreInstallNames () {
   if [ "$1" ]
   then
        echo "Changing core libs install names for $1"
         	       
        install_name_tool -change libU2Algorithmd.1.dylib  @executable_path/libU2Algorithmd.1.dylib "$TARGET_EXE_DIR"/$1
       	install_name_tool -change libU2Cored.1.dylib  @executable_path/libU2Cored.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Designerd.1.dylib  @executable_path/libU2Designerd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Formatsd.1.dylib  @executable_path/libU2Formatsd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Guid.1.dylib  @executable_path/libU2Guid.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Langd.1.dylib  @executable_path/libU2Langd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Privated.1.dylib  @executable_path/libU2Privated.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Remoted.1.dylib  @executable_path/libU2Remoted.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Testd.1.dylib  @executable_path/libU2Testd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Viewd.1.dylib  @executable_path/libU2Viewd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libugenedbd.1.dylib  @executable_path/libugenedbd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libgtestd.1.dylib  @executable_path/libgtestd.1.dylib "$TARGET_EXE_DIR"/$1
  
   else
       echo "changeCoreInstallNames: no parameter passed."
   fi

   return 0
}



#This function sets correct relative pathes for linking qt libraries
changeQtInstallNames () {
   if [ "$1" ]
   then
        echo "Changing qt install names for $1"

       install_name_tool -change $PATH_TO_QT/libQtCore_debug.4.dylib  @executable_path/../Frameworks/libQtCore_debug.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtScript_debug.4.dylib  @executable_path/../Frameworks/libQtScript_debug.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtXml_debug.4.dylib  @executable_path/../Frameworks/libQtXml_debug.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtXmlPatterns_debug.4.dylib  @executable_path/../Frameworks/libQtXmlPatterns_debug.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtGui_debug.4.dylib  @executable_path/../Frameworks/libQtGui_debug.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtNetwork_debug.4.dylib  @executable_path/../Frameworks/libQtNetwork_debug.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtWebKit_debug.4.dylib  @executable_path/../Frameworks/libQtWebKit_debug.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libphonon_debug.4.dylib  @executable_path/../Frameworks/libphonon_debug.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtTest_debug.4.dylib  @executable_path/../Frameworks/libQtTest_debug.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtOpenGL_debug.4.dylib @executable_path/../Frameworks/libQtOpenGL_debug.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtSvg_debug.4.dylib  @executable_path/../Frameworks/libQtSvg_debug.4.dylib "$TARGET_EXE_DIR"/$1

   else
       echo "changeQtInstallNames: no parameter passed."
   fi

   return 0
}

