/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_GUI_DIALOG_UTILS_RUNNABLES_H_
#define _U2_GUI_DIALOG_UTILS_RUNNABLES_H_

#include "api/GTGlobals.h"
#include "api/GTRegionSelector.h"
#include "GTUtilsDialog.h"

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtGui/QMessageBox>

namespace U2 {

class GTUtilsDialogRunnables {
public:
    class SmithWatermanDialogFiller : public Runnable {
    public:
        enum Button {Search, Cancel};
        SmithWatermanDialogFiller(U2OpStatus &_os, const QString& _pattern = "", const GTRegionSelector::RegionSelectorSettings& _s = GTRegionSelector::RegionSelectorSettings()) : button(Search), os(_os), pattern(_pattern), s(_s){}
        virtual void run();
        Button button;
    private:
        U2OpStatus &os;
        QString pattern;
        GTRegionSelector::RegionSelectorSettings s;
    };

    class CreateAnnotationDialogChecker : public Runnable {
    public:
        CreateAnnotationDialogChecker(U2OpStatus &_os) : os(_os){}
        virtual void run();
    private:
        U2OpStatus &os;
    };

	class CreateAnnotationDialogFiller : public Runnable {
	public:
		CreateAnnotationDialogFiller(U2OpStatus &_os, const QString &_groupName, const QString &_annotationName, const QString &_location)
			:os(_os), groupName(_groupName), annotationName(_annotationName), location(_location){}
		virtual void run();
	private:
		U2OpStatus &os;
		const QString &groupName;
		const QString &annotationName;
		const QString &location;
	};

    class ExportProjectDialogFiller : public Runnable {
    public:
        ExportProjectDialogFiller(U2OpStatus &_os, const QString &_projectFolder, const QString &_projectName)
            :os(_os), projectFolder(_projectFolder), projectName(_projectName){}
        virtual void run();
    private:
        U2OpStatus &os;
        const QString &projectFolder;
        const QString &projectName;
    };

    class ExportProjectDialogChecker : public Runnable {
    public:
        ExportProjectDialogChecker(U2OpStatus &_os, const QString &_projectName)
            :os(_os), projectName(_projectName){}
        virtual void run();
    private:
        U2OpStatus &os;
        const QString &projectName;
    };

    class SaveProjectAsDialogFiller : public Runnable {
    public:
        SaveProjectAsDialogFiller(U2OpStatus &_os, const QString &_projectName, const QString &_projectFolder, const QString &_projectFile)
            :os(_os), projectName(_projectName), projectFolder(_projectFolder), projectFile(_projectFile){}
        virtual void run();
    private:
        U2OpStatus &os;
        const QString &projectName;
        const QString &projectFolder;
        const QString &projectFile;
    };

    class MessageBoxDialogFiller : public Runnable {
    public:
        MessageBoxDialogFiller(U2OpStatus &_os, QMessageBox::StandardButton _b)
            :os(_os), b(_b){}
        virtual void run();
    private:
        U2OpStatus &os;
        QMessageBox::StandardButton b;
    };

    class PopupChooser : public Runnable {
    public:
        PopupChooser(U2OpStatus &_os, const QStringList &_namePath, GTGlobals::UseMethod _useMethod = GTGlobals::UseMouse)
            :os(_os), namePath(_namePath), useMethod(_useMethod){}
        virtual void run();
    protected:
        U2OpStatus &os;
        QStringList namePath;
        GTGlobals::UseMethod useMethod;
    };

    class RemoteDBDialogFiller : public Runnable {
    public:
        RemoteDBDialogFiller(U2OpStatus &_os, const QString _resID, int _DBItemNum, bool _pressCancel = false, const QString _saveDirPath = QString(),  GTGlobals::UseMethod _useMethod = GTGlobals::UseMouse)
            :os(_os), resID(_resID), DBItemNum(_DBItemNum), pressCancel(_pressCancel), saveDirPath(_saveDirPath), useMethod(_useMethod){}
        virtual void run();
    private:
        U2OpStatus &os;
        QString resID;
        int DBItemNum;
        bool pressCancel;
        QString saveDirPath;
        GTGlobals::UseMethod useMethod;
    };

    class ExportToSequenceFormatFiller : public Runnable {
    public:
        ExportToSequenceFormatFiller(U2OpStatus &_os, const QString &_path, const QString &_name, GTGlobals::UseMethod method);
        virtual void run();
    private:
        U2OpStatus &os;
        QString path, name;
        GTGlobals::UseMethod useMethod;
    };

	class ExportSelectedRegionFiller : public Runnable {
	public:
		ExportSelectedRegionFiller(U2OpStatus &_os, const QString &_path, const QString &_name, GTGlobals::UseMethod method);
		  virtual void run();
	private:
		U2OpStatus &os;
		QString path, name;
		GTGlobals::UseMethod useMethod;
	};


	class CopyToFileAsDialogFiller : public Runnable {
	public:
		enum FormatToUse {Genbank, GFF};
		CopyToFileAsDialogFiller(U2OpStatus &_os, const QString &_path, const QString &_name, 
                                 GTUtilsDialogRunnables::CopyToFileAsDialogFiller::FormatToUse _format, bool compressFile = true,
								 bool addToProject = false, GTGlobals::UseMethod method = GTGlobals::UseMouse);
        virtual void run();
	private:
        U2OpStatus &os;
		QString path, name;
		GTGlobals::UseMethod useMethod;
        FormatToUse format;
		bool compressFile;
        bool addToProject;
        QMap<FormatToUse, QString> comboBoxItems;
	};

    class ExportSequenceAsAlignmentFiller : public Runnable {
    public:
        enum FormatToUse {Clustalw, Fasta, Msf, Mega, Nexus, Sam, Stockholm};
        ExportSequenceAsAlignmentFiller(U2OpStatus &_os, const QString &_path, const QString &_name,
                                        GTUtilsDialogRunnables::ExportSequenceAsAlignmentFiller::FormatToUse _format, bool addDocumentToProject = false, GTGlobals::UseMethod method = GTGlobals::UseMouse);
        virtual void run();
    private:
        U2OpStatus &os;
        QString path, name;
        GTGlobals::UseMethod useMethod;
        FormatToUse format;
        bool addToProject;
        QMap<FormatToUse, QString> comboBoxItems;
    };

    class ExportSequenceOfSelectedAnnotationsFiller : public Runnable {
    public:
        enum FormatToUse {Fasta, Fastaq, Gff, Genbank};
        enum MergeOptions {SaveAsSeparate, Merge};
        ExportSequenceOfSelectedAnnotationsFiller(U2OpStatus &_os, const QString &_path, FormatToUse _format, MergeOptions _options, int _gapLength = 0,
                                                  bool _addDocToProject = true, bool _exportWithAnnotations = false, GTGlobals::UseMethod method = GTGlobals::UseMouse);
        virtual void run();
    private:
        U2OpStatus &os;
        QString path;
        int gapLength;
        FormatToUse format;
        bool addToProject;
        bool exportWithAnnotations;
        MergeOptions options;
        GTGlobals::UseMethod useMethod;
        QMap<FormatToUse, QString> comboBoxItems;
        QMap<MergeOptions, QString> mergeRadioButtons;
    };


	class CreateDocumentFiller : public Runnable {
    public:
        enum documentFormat {FASTA, Genbank};
        CreateDocumentFiller(U2OpStatus &_os, const QString &_pasteDataHere, const QString &_documentLocation,
			                 documentFormat _format, const QString &_sequenceName, GTGlobals::UseMethod method = GTGlobals::UseMouse);
        virtual void run();
	private:
        U2OpStatus &os;
        QString documentLocation;
        QString pasteDataHere;
        QString sequenceName;
        documentFormat format;
        QMap<documentFormat, QString> comboBoxItems;

        GTGlobals::UseMethod useMethod;
    };

	
	class InsertSequenceFiller : public Runnable {
	public:
		enum documentFormat {FASTA, Genbank};
        enum RegionResolvingMode {Resize, Remove, SplitJoin, SplitSeparate};

        InsertSequenceFiller(U2OpStatus &_os, const QString &_pasteDataHere, RegionResolvingMode _regionResolvingMode = Resize, int _insertPosition = 1,
							 const QString &_documentLocation = "", 
							 documentFormat _format = FASTA, bool _saveToNewFile = false, bool _mergeAnnotations = false,
							 GTGlobals::UseMethod method = GTGlobals::UseMouse);
        virtual void run();
    private:
        U2OpStatus &os;
        QString pasteDataHere;
        RegionResolvingMode regionResolvingMode;
        int insertPosition;
        QString documentLocation;
        documentFormat format;
        bool saveToNewFile;
        bool mergeAnnotations;
        GTGlobals::UseMethod useMethod;

        QMap<documentFormat, QString> comboBoxItems;
	};


    class ExportAnnotationsFiller : public Runnable {
    public:
        enum fileFormat {genbank, gff, csv};
        ExportAnnotationsFiller(U2OpStatus &_os, const QString &_exportToFile, fileFormat _format, bool _saveSequencesUnderAnnotations = true,
			                    bool _saveSequenceNames = true, GTGlobals::UseMethod method = GTGlobals::UseMouse);
        virtual void run();
    private:
        U2OpStatus &os;
        QString exportToFile;
        fileFormat format;
        QMap<fileFormat, QString> comboBoxItems;
        bool saveSequencesUnderAnnotations;
        bool saveSequenceNames;

        GTGlobals::UseMethod useMethod;
    };


    class selectSequenceRegionDialogFiller : public Runnable {
    public:
        enum RangeType {Single, Multiple};
        selectSequenceRegionDialogFiller(U2OpStatus &_os, int *_len);
        selectSequenceRegionDialogFiller(U2OpStatus &_os);
        selectSequenceRegionDialogFiller(U2OpStatus &_os, int _minVal, int _maxVal);
        selectSequenceRegionDialogFiller(U2OpStatus &_os, const QString &range);
        selectSequenceRegionDialogFiller(U2OpStatus &_os, int _length, bool selectFromBegin = true);

        virtual void run();
    private:
        U2OpStatus &os;
        RangeType rangeType;
        bool selectAll, fromBegin;
        int minVal, maxVal, length, *len;
        QString multipleRange;
    };


	class RemovePartFromSequenceDialogFiller : public Runnable {
	public:
		enum RemoveType {Remove, Resize};
		enum FormatToUse {FASTA, Genbank};

		RemovePartFromSequenceDialogFiller(U2OpStatus &_os, QString _range);
		RemovePartFromSequenceDialogFiller(U2OpStatus &_os,RemoveType _removeType, bool _saveNew, const QString &_saveToFile, FormatToUse _format);
		RemovePartFromSequenceDialogFiller(U2OpStatus &_os, RemoveType _removeType);

		virtual void run();
	private:
		U2OpStatus &os;
		QString range;
		RemoveType removeType;
		FormatToUse format;
		bool saveNew;
		QString saveToFile;
		QMap<FormatToUse, QString> comboBoxItems;
	};


	class SequenceReadingModeSelectorDialogFiller : public Runnable {
	public:
		enum ReadingMode {Separate, Merge, Join, Align};

		SequenceReadingModeSelectorDialogFiller(U2OpStatus &_os, ReadingMode _mode = Separate, int _bases=10):
		os(_os), readingMode(_mode), bases(_bases) {}

		virtual void run();
	private:
		U2OpStatus &os;
		ReadingMode readingMode;
		int bases;
	};

    class ZoomToRangeDialogFiller : public Runnable {
    public:
        ZoomToRangeDialogFiller(U2OpStatus &_os, int min, int max) : os(_os), minVal(min), maxVal(max){}
        virtual void run();
    private:
        U2OpStatus &os;
        int minVal, maxVal;
    };

    class GoToDialogFiller : public Runnable {
    public:
        GoToDialogFiller(U2OpStatus &_os, int _goTo) : os(_os), goTo(_goTo){}
        virtual void run();
    private:
        U2OpStatus &os;
        int goTo;
    };
};

} // namespace

#endif
