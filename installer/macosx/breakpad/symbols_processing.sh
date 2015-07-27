SYMBOLS_DIR=symbols_$VERSION

dump_symbols() {
    filename=`basename "${1}"`
    breakpad/dump_syms -a ${ARCHITECTURE} "$1" > "${SYMBOLS_DIR}/$filename.sym"
}

function extract_symbols {
    rm -rf ${SYMBOLS_DIR}

    mkdir "${SYMBOLS_DIR}"

    find ${TARGET_APP_DIR} | while read fff; do
        islib=`file $fff | grep -e 'Mach-O .* library' -e 'Mach-O .* executable'`
        if [ -n "${islib}" ]; then
            dump_symbols "$fff"
        fi
    done
}
