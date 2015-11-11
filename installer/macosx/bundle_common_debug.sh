function add-plugin {
    plugin=$1
    echo "Registering plugin: ${plugin}"

    PLUGIN_LIB="lib${plugin}d.dylib"
    PLUGIN_DESC="${plugin}d.plugin"
    PLUGIN_LICENSE="${plugin}d.license"

    if [ ! -f ${DEBUG_DIR}/plugins/${PLUGIN_LIB} ] ;  
    then  
        echo "Plugin library file not found: ${PLUGIN_LIB} !"
        echo "Plugin is skipped"
        return
    fi

    if [ ! -f ${DEBUG_DIR}/plugins/${PLUGIN_DESC} ] ; 
    then
        echo "Plugin descriptor file not found: ${PLUGIN_DESC} !"
        echo "Plugin is skipped"
        return
    fi

    if [ ! -f ${DEBUG_DIR}/plugins/${PLUGIN_LICENSE} ] ; 
    then
        echo "Plugin descriptor file not found: ${PLUGIN_LICENSE} !"
        echo "Plugin is skipped"
        return
    fi

    cp "${DEBUG_DIR}/plugins/${PLUGIN_LIB}"  "${TARGET_EXE_DIR}/plugins/"
    cp "${DEBUG_DIR}/plugins/${PLUGIN_DESC}" "${TARGET_EXE_DIR}/plugins/"
    cp "${DEBUG_DIR}/plugins/${PLUGIN_LICENSE}" "${TARGET_EXE_DIR}/plugins/"
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
        install_name_tool -change libU2Scriptd.1.dylib  @executable_path/libU2Scriptd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Testd.1.dylib  @executable_path/libU2Testd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libU2Viewd.1.dylib  @executable_path/libU2Viewd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libugenedbd.1.dylib  @executable_path/libugenedbd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libgtestd.1.dylib  @executable_path/libgtestd.1.dylib "$TARGET_EXE_DIR"/$1
        install_name_tool -change libhumimitd.1.dylib  @executable_path/libhumimitd.1.dylib "$TARGET_EXE_DIR"/$1
  
   else
       echo "changeCoreInstallNames: no parameter passed."
   fi

   return 0
}
