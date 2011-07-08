#!/bin/bash

. ./config.sh $1

echo "------------"
echo "work set:" ${SRC_FILES[@]}
echo "dst file:" $DST_DIR
echo "exl list:" ${EXCLUDE_LIST[@]}
echo "------------"



LIMIT=0
copy_Function_for_file()
{
CUR_FILENAME="$1"
flag=0
    for EXCLUDE_FILENAME in "${EXCLUDE_LIST[@]}"
    do
        if [[ "$CUR_FILENAME" == $EXCLUDE_FILENAME ]]
        then    
        flag=1
        fi
    done
    #------------------------------------------------
    
    if [[ $flag == 0 ]]
    then
            #if file is in the list -> add header
            flag_to_copy=1
            for i in ${LIMIT[@]}
            do
                CUR_FILENAME_FORMATED="${CUR_FILENAME##*../}"
				CUR_FILEPATH="${CUR_FILENAME_FORMATED%/*}"
				mkdir -p "$DST_DIR$CUR_FILEPATH"
            done
                    if [[ $flag_to_copy == 1 ]]
                    then
                        #просто скопировать
					   cp "$CUR_FILENAME" "$DST_DIR$CUR_FILENAME_FORMATED"              
                    fi  
    fi
}
copy_Function()
{
for CUR_FILENAME in "$1"*
do
    flag=0
    for EXCLUDE_FILENAME in "${EXCLUDE_LIST[@]}"
    do

        if [[ "$CUR_FILENAME" == $EXCLUDE_FILENAME ]]
        then    
        flag=1
        fi
    done
    #------------------------------------------------
    
    if [[ $flag == 0 ]]
    then
        if [[ -d "$CUR_FILENAME" ]]
        then # if its dir
           CUR_FILENAME_FORMATED="${CUR_FILENAME##*../}"
			mkdir -p  "$DST_DIR$CUR_FILENAME_FORMATED"   
		   copy_Function "$CUR_FILENAME/"
        else # if its file
            #if file is in list -> add header
            flag_to_copy=1
            for i in ${LIMIT[@]}
            do
                if [[ $CUR_FILENAME == ${FILE_TYPE[$i]} ]]
                then
                    #echo "head: $CUR_FILENAME --> $DST_DIR$CUR_FILENAME"
					CUR_FILENAME_FORMATED="${CUR_FILENAME##*../}"
                    cat "$CUR_FILENAME" > "$DST_DIR$CUR_FILENAME_FORMATED" 
                fi
            done
                    if [[ $flag_to_copy == 1 ]]
                    then
                        #just copy
						CUR_FILENAME_FORMATED="${CUR_FILENAME##*../}"
                        cp "$CUR_FILENAME" "$DST_DIR$CUR_FILENAME_FORMATED"              
                    fi  

        fi
    fi
done    
}
#------------------------------------------------------------------------------------------------------------------------------------------------

#TODO: check for svn, sed, awk
echo "---checking that all required tools installed"

TEMP_PAPKA_NAME="tmpdir/"
TEMP_ARHIV_NAME="temparch.tgz"

FILE_FOR_TESTING="config.sh"
FILE_FOR_TESTING_TWO="gogi.sh"

rm -rf $TEMP_PAPKA_NAME
mkdir -p  $TEMP_PAPKA_NAME
    if [[ ! -d $TEMP_PAPKA_NAME ]]
    then
        echo "mkdir -p  - failed"
        exit 1
    fi  
cp "$FILE_FOR_TESTING" "$TEMP_PAPKA_NAME$FILE_FOR_TESTING"      
    if [[ ! -s $TEMP_PAPKA_NAME$FILE_FOR_TESTING ]]
    then
        echo "cp - failed"
        exit 1
    fi
cat "$FILE_FOR_TESTING" > "$TEMP_PAPKA_NAME$FILE_FOR_TESTING_TWO"   
    if [[ ! -s $TEMP_PAPKA_NAME$FILE_FOR_TESTING_TWO ]]
    then
        echo "cat - failed"
        exit 1
    fi
tar -${TAR_ARG} $TEMP_ARHIV_NAME $TEMP_PAPKA_NAME
    if [[ ! -s $TEMP_ARHIV_NAME ]]
    then
        echo "tar - failed"
        exit 1
    fi
rm -irf $TEMP_PAPKA_NAME
rm -irf $TEMP_ARHIV_NAME
    if [[ -s $TEMP_ARHIV_NAME ]]
    then
        echo "rm - failed"
        exit 1
    fi
echo "---check for required tools finished succesfully"

#------------------------------------------------------------------------------------------------------------------------------------------------
#read headers mapping
list_flag=0
temp_per=1
for TEMP_LIST in "${MAP_LIST[@]}"
    do  
    if [[ $list_flag == 0 ]]
        then
            #FILE_TYPE=( "$FILE_TYPE" "$TEMP_LIST" )    
            FILE_TYPE[temp_per]="$TEMP_LIST"
            list_flag=1
        else
            HEADER_FILE_NAME[temp_per]="$TEMP_LIST"
            list_flag=0
            if [[ ! -s "${HEADER_FILE_NAME[$temp_per]}" ]] #if header file found..
                then
                echo "${HEADER_FILE_NAME[$temp_per]} not found"
                exit
            fi
        #----------------------------------------------------------------------------
            LIMIT[$temp_per]=$temp_per
            let "temp_per += 1"
        #----------------------------------------------------------------------------
        fi
    done
    
echo "--------------------------------------------"

for FILENAME in "${SRC_FILES[@]}"
#iterate over all files in dir
    do 
    #echo $FILENAME*
        if [[ ! -d "$FILENAME" ]] #if dir found..
        then
            if [[ ! -s "$FILENAME" ]]
            then
            echo "not found $FILENAME"
            exit 1
            fi
        fi
    done

mkdir -p  "$DST_DIR"    
    
for FILENAME in "${SRC_FILES[@]}"
#iterate over all files in dir
    do 
    #echo $FILENAME*
        if [[ -d "$FILENAME" ]] # if dir found..
        then
			FILENAME_FORMATED="${FILENAME##*../}"
            mkdir -p  "$DST_DIR$FILENAME_FORMATED"
            copy_Function "$FILENAME"   
        else
            if [[ -s "$FILENAME" ]]
            then
            copy_Function_for_file "$FILENAME"
            fi
        fi
    done
    
    # set ugene exclude list enabled for qmake configuration	
    sed -i 's/UGENE_EXCLUDE_LIST_ENABLED = 0/UGENE_EXCLUDE_LIST_ENABLED = 1/g' "$DST_DIR/src/ugene_globals.pri"    
    # delete previous src tar
    rm -rf *.tar.gz
   
    # add dst file
    tar -${TAR_ARG} "$DST_FILE" "$DST_DIR" #add to archive
    if [[ $REMOVE_FLAG == 1 ]]
    then
        rm -irf "$DST_DIR"
    fi
    #mv "$DST_FILE" "$DST_DIR" # move archive to dst folder
    
