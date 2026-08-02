## Architect Query

Упал шаг: 02-build-sender
Ошибка: pio : src/main.cpp: In function 'void readButtonsAndSend()':
��ப�:1 ����:1
+ pio run 2>&1
+ ~~~~~~~~~~~~
    + CategoryInfo          : NotSpecified: (src/main.cpp: I...tonsAndSend()'::String) [], RemoteException
    + FullyQualifiedErrorId : NativeCommandError
 
src/main.cpp:58:19: error: 'connectAndSend' was not declared in this scope
         bool ok = connectAndSend(CALL_DINNER);
                   ^~~~~~~~~~~~~~
src/main.cpp:104:15: error: 'connectAndSendWithDiagnostics' was not declared in 
Данные: см. status.json
Что делать: retry/skip/stop?
