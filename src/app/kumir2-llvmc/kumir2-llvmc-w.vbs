strLlvmc = Replace(WScript.ScriptFullName, WScript.ScriptName, "kumir2-llvmc.exe")
strFile = WScript.Arguments.Item(0)
strTargetFile = Replace(strFile, ".kum", ".exe")
strCommand = strLlvmc & " " & strFile
message = "��������� ���� " & strTargetFile & "..."
CreateObject("WScript.Shell").Run strCommand, 0, False
CreateObject("WScript.Shell").Popup message, 2, "�����", 64


