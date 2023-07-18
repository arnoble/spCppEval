REM
REM  runs some UserParameters analysis (after first running FairValues, in case there are new products)
REM
REM
set argC=0
set numIter=1000
for %%x in (%*) do Set /A argC+=1
REM echo %argC%
if %argC%  gtr 2 goto ok
REM @if not '%1' == '' goto ok
@echo "usage: LastDataDate  userParametersId  #iterations   <and possibly a comma-sep list of productIds WHICH MUST BE ENCLOSED IN DOUBLE QUOTES>"
@pause
@goto done
:ok
@echo "executing scenarios"
::ECHO.|date >CURRENT2.BAT
::ECHO. | DATE | FIND "Current" > GDATE.BAT

REM cunning way to get batch file to wait for all start-ed programs to finish
REM ... The set /P command WOULD NORMALLY terminate when any one of the commands in the ( block ) outputs a line, but start commands don't show any line in THIS cmd.exe
REM ... This way, set /P keeps waiting for input until all processes started by start commands end
REM ... At that point the pipe line associated to the ( block ) is closed, so the set /P Stdin is closed and set /P command is terminated by the OS
@IF NOT "%~4" == "" GOTO HAVE_IDS
REM run fair values, for endDate = %1
(
start  C:/sites/sp/Consoleapplication1    34    7000 %3    ignoreBenchmark notIllustrative notStale only:UK100,SX5E,SPX  getMarketData updateProduct dbServer:newSp  silent   fvEndDate:%1
start  C:/sites/sp/Consoleapplication1  7001    9000 %3    ignoreBenchmark notIllustrative notStale only:UK100,SX5E,SPX  getMarketData updateProduct dbServer:newSp  silent   fvEndDate:%1
start  C:/sites/sp/Consoleapplication1  9001   10000 %3    ignoreBenchmark notIllustrative notStale only:UK100,SX5E,SPX  getMarketData updateProduct dbServer:newSp  silent   fvEndDate:%1
start  C:/sites/sp/Consoleapplication1 10001   11000 %3    ignoreBenchmark notIllustrative notStale only:UK100,SX5E,SPX  getMarketData updateProduct dbServer:newSp  silent   fvEndDate:%1
start  C:/sites/sp/Consoleapplication1 11001   14000 %3    ignoreBenchmark notIllustrative notStale only:UK100,SX5E,SPX  getMarketData updateProduct dbServer:newSp  silent   fvEndDate:%1
)  | set /P "="
REM run IPR analysis, using those fair values
(
start  C:/sites/sp/Consoleapplication1   34     7000 %3    ignoreBenchmark notIllustrative notStale only:UK100,SX5E,SPX  userParameters:%2 dbServer:newSp     silent   endDate:%1
start  C:/sites/sp/Consoleapplication1  7001    9000 %3    ignoreBenchmark notIllustrative notStale only:UK100,SX5E,SPX  userParameters:%2 dbServer:newSp     silent   endDate:%1
start  C:/sites/sp/Consoleapplication1  9001   10000 %3    ignoreBenchmark notIllustrative notStale only:UK100,SX5E,SPX  userParameters:%2 dbServer:newSp     silent   endDate:%1
start  C:/sites/sp/Consoleapplication1 10001   11000 %3    ignoreBenchmark notIllustrative notStale only:UK100,SX5E,SPX  userParameters:%2 dbServer:newSp     silent   endDate:%1
start  C:/sites/sp/Consoleapplication1 11001   14000 %3    ignoreBenchmark notIllustrative notStale only:UK100,SX5E,SPX  userParameters:%2 dbServer:newSp     silent   endDate:%1
)  | set /P "="

@echo "all done"
EXIT

REM
REM  if commandLine has productIds
REM
:HAVE_IDS
REM
REM run fair values
C:/sites/sp/Consoleapplication1  %~4  %3   getMarketData  updateProduct dbServer:newSp silent   fvEndDate:%1
REM
@echo C:/sites/sp/Consoleapplication1  %~4  %3    ignoreBenchmark userParameters:%2 dbServer:newSp     silent   endDate:%1
REM %~4 strips the argument enclosing quotes from that argument
(
start  C:/sites/sp/Consoleapplication1  %~4  %3    ignoreBenchmark userParameters:%2 dbServer:newSp     silent   endDate:%1
)  | set /P "="
:done
