rem
rem  get prices for a list of products
rem
rem get today
FOR /F "TOKENS=1 eol=/ DELIMS=/ " %%A IN ('DATE/T') DO SET dd=%%A
FOR /F "TOKENS=1,2 eol=/ DELIMS=/ " %%A IN ('DATE/T') DO SET mm=%%B
FOR /F "TOKENS=1,2,3 eol=/ DELIMS=/ " %%A IN ('DATE/T') DO SET yyyy=%%C
SET todaysdate=%yyyy%%mm%%dd%
rem
for /F "usebackq delims==" %%X in (`type aa_productIds.txt`)  do bberg %%X %%X %todaysdate% curves:n cds:n static:n
