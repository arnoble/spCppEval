rem  productId  dbServer asOfDate   eg   10117 newSp  2022-03-11

echo "use all archive data"
consoleApplication1.exe %1 %1 40000 getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11 arcOnCurveDate:2022-03-11  arcCurveDate:2022-03-11 arcDivDate:2022-03-11 arcCorDate:2022-03-11 arcVolDate:2022-03-11  silent

echo "update vols"
consoleApplication1.exe %1 %1 40000 getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11 arcOnCurveDate:2022-03-11  arcCurveDate:2022-03-11 arcDivDate:2022-03-11 arcCorDate:2022-03-11 silent

echo "update correlation"
consoleApplication1.exe %1 %1 40000 getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11 arcOnCurveDate:2022-03-11  arcCurveDate:2022-03-11 arcDivDate:2022-03-11  silent

echo "update impDiv"
consoleApplication1.exe %1 %1 40000 getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11 arcOnCurveDate:2022-03-11  arcCurveDate:2022-03-11  silent

echo "update onCurve"
consoleApplication1.exe %1 %1 40000 getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11 arcCurveDate:2022-03-11  silent

echo "update curve"
consoleApplication1.exe %1 %1 40000 getMarketData dbServer:%2  endDate:%3     arcCdsDate:2022-03-11  silent

echo "update cds"
consoleApplication1.exe %1 %1 40000 getMarketData dbServer:%2  endDate:%3     silent

echo "update spots"
consoleApplication1.exe %1 %1 40000 getMarketData dbServer:%2  silent
