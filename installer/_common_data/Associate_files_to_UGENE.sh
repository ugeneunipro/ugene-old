#!/bin/bash

IS_ROOT="1"
LOG_FILE=associating.log
if [[ $(/usr/bin/id -u) -ne 0 ]]; then
    echo "Not running as root" | tee $LOG_FILE
    echo "Associate files to UGENE only for this user? [y/N]" | tee -a $LOG_FILE
    echo "(If you want associate files to UGENE for all users, then run this script with root privileges)" | tee -a $LOG_FILE
    read line
    if [[ "$line" == "y" || "$line" == "Y" ]];then
        echo 'Associating files to UGENE for this user' | tee -a $LOG_FILE
    else
        exit
    fi

    echo 'Extracting the icons to "~/.local/share/icons":'  | tee -a $LOG_FILE
    tar -xvzf icons.tar.gz -C ~/.local/share/icons 2>&1 | tee -a $LOG_FILE

    echo 'Creating the MIME types (copy application-x-ugene.xml to ~/.local/share/mime/packages):' | tee -a $LOG_FILE
    cp -f application-x-ugene.xml ~/.local/share/mime/packages/ 2>&1 | tee -a $LOG_FILE

    echo 'Updating new MIME types.' | tee -a $LOG_FILE
    update-mime-database ~/.local/share/mime/ 2>&1 | tee -a $LOG_FILE

    echo 'Adding the MIME types to the ugene.desktop file.' | tee -a $LOG_FILE
    if [ -e ~/.local/share/applications/ugene.desktop ]; then
        cp ~/.local/share/applications/ugene.desktop ~/.local/share/applications/ugene.desktop.bak
        grep -v "MimeType=" ~/.local/share/applications/ugene.desktop.bak > ~/.local/share/applications/ugene.desktop
        echo "MimeType=application/x-ugene-fa;application/x-ugene-uprj;application/x-ugene-uwl;application/x-ugene-uql;application/x-ugene-abi;application/x-ugene-aln;application/x-ugene-embl;application/x-ugene-sw;application/x-ugene-fastq;application/x-ugene-gb;application/x-ugene-gff;application/x-ugene-msf;application/x-ugene-newick;application/x-ugene-pdb;application/x-ugene-sam-bam;application/x-ugene-srfa;application/x-ugene-sto;application/x-ugene-db;application/x-ugene-scf;application/x-ugene-mmdb;application/x-ugene-hmm;" >>~/.local/share/applications/ugene.desktop
        rm ~/.local/share/applications/ugene.desktop.bak
    else
        cp ugene.desktop ~/.local/share/applications/ 2>&1 | tee -a $LOG_FILE
    fi

    echo "See log of operations in $LOG_FILE file" | tee -a $LOG_FILE
    echo "(You may need to restart your file manager to see the change.)" | tee -a $LOG_FILE
    echo 'Done. This terminal will be closed in 10 seconds.' | tee -a $LOG_FILE

    sleep 10

else
    echo "Associate files to UGENE? [y/N]" | tee $LOG_FILE
    read line
    if [[ "$line" == "y" || "$line" == "Y" ]];then
        echo 'Associating files to UGENE' | tee -a $LOG_FILE
    else
        exit
    fi

    echo 'Extracting the icons to "/usr/share/icons":' | tee -a $LOG_FILE
    tar -xvzf icons.tar.gz -C /usr/share/icons 2>&1 | tee -a $LOG_FILE

    echo 'Creating the MIME types (copy application-x-ugene.xml to /usr/share/mime/packages):' | tee -a $LOG_FILE
    cp -f application-x-ugene.xml /usr/share/mime/packages/ 2>&1 | tee -a $LOG_FILE

    echo 'Updating new MIME types.' | tee -a $LOG_FILE
    update-mime-database /usr/share/mime/ 2>&1 | tee -a $LOG_FILE

    echo 'Updating new .desktop' | tee -a $LOG_FILE
    update-desktop-database 2>&1 | tee -a $LOG_FILE

    echo 'Adding the MIME types to the ugene.desktop file.' | tee -a $LOG_FILE
    if [ -e /usr/share/applications/ugene.desktop ]; then
        cp /usr/share/applications/ugene.desktop /usr/share/applications/ugene.desktop.bak
        grep -v "MimeType=" /usr/share/applications/ugene.desktop.bak > /usr/share/applications/ugene.desktop
        echo "MimeType=application/x-ugene-fa;application/x-ugene-uprj;application/x-ugene-uwl;application/x-ugene-uql;application/x-ugene-abi;application/x-ugene-aln;application/x-ugene-embl;application/x-ugene-sw;application/x-ugene-fastq;application/x-ugene-gb;application/x-ugene-gff;application/x-ugene-msf;application/x-ugene-newick;application/x-ugene-pdb;application/x-ugene-sam-bam;application/x-ugene-srfa;application/x-ugene-sto;application/x-ugene-db;application/x-ugene-scf;application/x-ugene-mmdb;application/x-ugene-hmm;" >>/usr/share/applications/ugene.desktop
        rm /usr/share/applications/ugene.desktop.bak
    else
        cp ugene.desktop /usr/share/applications/ 2>&1 | tee -a $LOG_FILE
    fi
    

    echo "See log of operations in $LOG_FILE file" | tee -a $LOG_FILE
    echo "(You may need to restart your file manager to see the change.)" | tee -a $LOG_FILE
    echo 'Done. This terminal will be closed in 10 seconds.' | tee -a $LOG_FILE

    sleep 10
fi
exit 0
