rem
rem  get current price for a list of products
rem
FOR /F "TOKENS=1 eol=/ DELIMS=/ " %%A IN ('DATE/T') DO SET dd=%%A
FOR /F "TOKENS=1,2 eol=/ DELIMS=/ " %%A IN ('DATE/T') DO SET mm=%%B
FOR /F "TOKENS=1,2,3 eol=/ DELIMS=/ " %%A IN ('DATE/T') DO SET yyyy=%%C
rem
SET todaysdate=%yyyy%%mm%%dd%
rem
rem
for /F "usebackq delims==" %%X in (`type %1`)  do bberg %%X %%X %todaysdate% prices:n curves:n static:n cds:n currentPrices:y dbServer:spCloud
