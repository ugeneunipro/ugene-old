function add-plugin {
    plugin=$1
    echo "Registering plugin: ${plugin}"

    PLUGIN_LIB="lib${plugin}.dylib"
    PLUGIN_DESC="${plugin}.plugin"

    if [ ! -f ${RELEASE_DIR}/plugins/${PLUGIN_LIB} ] ;  
    then  
        echo "Plugin library file not found: ${PLUGIN_LIB} !"
        exit 1
    fi

    if [ ! -f ${RELEASE_DIR}/plugins/${PLUGIN_DESC} ] ; 
    then
        echo "Plugin descriptor file not found: ${PLUGIN_DESC} !"
        exit 1
    fi
    
    cp "${RELEASE_DIR}/plugins/${PLUGIN_LIB}"  "${TARGET_EXE_DIR}/plugins/"
    cp "${RELEASE_DIR}/plugins/${PLUGIN_DESC}" "${TARGET_EXE_DIR}/plugins/"
    changeQtInstallNames "plugins/${PLUGIN_LIB}"
    changeCoreInstallNames "plugins/${PLUGIN_LIB}"
}

function add-library {
    lib=$1
    echo "Adding lib: ${lib}"

    LIB_FILE="lib${lib}.1.dylib"

    if [ ! -f ${RELEASE_DIR}/${LIB_FILE} ] ;  
    then  
        echo "Library file not found: ${LIB_FILE} !"
        exit 1
    fi

    cp "${RELEASE_DIR}/${LIB_FILE}"  "${TARGET_EXE_DIR}/"
    changeQtInstallNames "${LIB_FILE}"
    changeCoreInstallNames "${LIB_FILE}"		
}


#This function sets correct relative pathes for linking UGENE core libraries
changeCoreInstallNames () {
   if [ "$1" ]
   then
        echo "Changing core libs install names for $1"
         	       
        install_name_tool -change libU2Algorithm.1.dylib  @executable_path/libU2Algorithm.1.dylib "$TARGET_EXE_DIR"/$1
       	install_name_tool -change libU2Core.1.dylib  @executable_path/libU2Core.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Designer.1.dylib  @executable_path/libU2Designer.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Formats.1.dylib  @executable_path/libU2Formats.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Gui.1.dylib  @executable_path/libU2Gui.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Lang.1.dylib  @executable_path/libU2Lang.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Misc.1.dylib  @executable_path/libU2Misc.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Private.1.dylib  @executable_path/libU2Private.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Remote.1.dylib  @executable_path/libU2Remote.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Test.1.dylib  @executable_path/libU2Test.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2View.1.dylib  @executable_path/libU2View.1.dylib "$TARGET_EXE_DIR"/$1
         install_name_tool -change libsqlite3.1.dylib  @executable_path/libsqlite3.1.dylib "$TARGET_EXE_DIR"/$1
  
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

       install_name_tool -change $PATH_TO_QT/libQtCore.4.dylib  @executable_path/../Frameworks/libQtCore.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtScript.4.dylib  @executable_path/../Frameworks/libQtScript.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtXml.4.dylib  @executable_path/../Frameworks/libQtXml.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtXmlPatterns.4.dylib  @executable_path/../Frameworks/libQtXmlPatterns.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtGui.4.dylib  @executable_path/../Frameworks/libQtGui.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtNetwork.4.dylib  @executable_path/../Frameworks/libQtNetwork.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtWebKit.4.dylib  @executable_path/../Frameworks/libQtWebKit.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libphonon.4.dylib  @executable_path/../Frameworks/libphonon.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtTest.4.dylib  @executable_path/../Frameworks/libQtTest.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtOpenGL.4.dylib @executable_path/../Frameworks/libQtOpenGL.4.dylib "$TARGET_EXE_DIR"/$1
       install_name_tool -change $PATH_TO_QT/libQtSvg.4.dylib  @executable_path/../Frameworks/libQtSvg.4.dylib "$TARGET_EXE_DIR"/$1

   else
       echo "changeQtInstallNames: no parameter passed."
   fi

   return 0
}

