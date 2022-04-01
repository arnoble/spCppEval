rem  productId  dbServer asOfDate   eg   10117 newSp  2022-03-11
IF NOT "%~1"=="" IF NOT "%~2"=="" IF NOT "%~3"=="" GOTO START
ECHO This script requires the next parameters:
ECHO - productId
ECHO - dbServer
ECHO - asOfDate
GOTO :EOF

:START
echo "use all archive data"
consoleApplication1.exe %1 %1 50000 forceIterations useProductFundingFractionFactor getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11 arcOnCurveDate:2022-03-11  arcCurveDate:2022-03-11 arcDivDate:2022-03-11 arcCorDate:2022-03-11 arcVolDate:2022-03-11  silent bump:theta:0:0:1  bumpUserId:3

echo "update vols"
consoleApplication1.exe %1 %1 50000 forceIterations useProductFundingFractionFactor getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11 arcOnCurveDate:2022-03-11  arcCurveDate:2022-03-11 arcDivDate:2022-03-11 arcCorDate:2022-03-11 silent bump:theta:0:0:1  bumpUserId:3

echo "update correlation"
consoleApplication1.exe %1 %1 50000 forceIterations useProductFundingFractionFactor getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11 arcOnCurveDate:2022-03-11  arcCurveDate:2022-03-11 arcDivDate:2022-03-11  silent bump:theta:0:0:1  bumpUserId:3

echo "update impDiv"
consoleApplication1.exe %1 %1 50000 forceIterations useProductFundingFractionFactor getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11 arcOnCurveDate:2022-03-11  arcCurveDate:2022-03-11  silent bump:theta:0:0:1  bumpUserId:3

echo "update onCurve"
consoleApplication1.exe %1 %1 50000 forceIterations useProductFundingFractionFactor getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11 arcCurveDate:2022-03-11  silent bump:theta:0:0:1  bumpUserId:3

echo "update curve"
consoleApplication1.exe %1 %1 50000 forceIterations useProductFundingFractionFactor getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11  silent  bump:theta:0:0:1  bumpUserId:3

echo "update cds"
consoleApplication1.exe %1 %1 50000 forceIterations useProductFundingFractionFactor getMarketData dbServer:%2  endDate:%3     silent  bump:theta:0:0:1  bumpUserId:3

echo "update spots"
consoleApplication1.exe %1 %1 50000 forceIterations useProductFundingFractionFactor getMarketData dbServer:%2  silent bump:theta:0:0:1  bumpUserId:3

:EOF
