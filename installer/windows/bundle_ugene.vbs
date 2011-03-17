Const ForReading = 1

Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objFile = objFSO.OpenTextFile("src\\ugene_version.pri", ForReading)

i = 0
verStr = ""
Do Until objFile.AtEndOfStream
    strLine = objFile.ReadLine
    If InStr(strLine, "UGENE_VERSION=") > 0 Then
        cutPos = InStr(strLine,"=")
        verLen = Len(strLine) - cutPos + 1
        verStr = Mid(strLine, cutPos + 1, verLen)
    End If
Loop

Wscript.Echo verStr

Set WshShell = CreateObject("WScript.Shell")
curDir = objFSO.GetParentFolderName(Wscript.ScriptFullName)
Wscript.Echo curDir


runStr = curDir & "\\bundle_binary_zip.cmd " & verStr
Wscript.Echo runStr
WshShell.Run runStr,1,False
   
objFile.Close


