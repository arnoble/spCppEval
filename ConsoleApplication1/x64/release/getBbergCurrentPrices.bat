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
mysql -h 166.63.0.149 -u anoble -pRagtin_Mor14 --column-names=0 -e  "use sp; select productid from product p join institution i on (p.providerid=i.institutionid) where productid>34 and matured=0 and isin !=''  and (i.name like 'cube%%'  or p.BbergTicker like '%%equity%%')" > bbergCurrentPricelist.txt
rem
for /F "usebackq delims==" %%X in (`type %1`)  do bberg %%X %%X %todaysdate% prices:n curves:n static:n cds:n currentPrices:y dbServer:spCloud
