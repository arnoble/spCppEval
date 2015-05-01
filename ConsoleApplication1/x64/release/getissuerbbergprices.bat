@set alright = true
@echo %alright%
@if '%1' == ''  set alright = false
@if '%2' == ''  set alright = false
@if '%3' == ''  set alright = false
@echo %alright%
@if %alright% == true goto ok
@echo "Usage: YYYYmmdd issuerName filenameForProductIds"
@goto done
:ok
rem
rem  get current price for a list of products
rem
SET todaysdate=%1
SET issuername=%2
rem
rem
mysql -h 166.63.0.149 -u anoble -pRagtin_Mor14 --column-names=0 -e  "use sp; select productid from product p join institution i on (p.counterpartyid=i.institutionid) where productid>34 and matured=0 and isin !=''  and (i.name like '%issuername%%%' )" > %3
rem
for /F "usebackq delims==" %%X in (`type %3`)  do bberg %%X %%X %todaysdate% prices:n curves:n static:n cds:n currentPrices:y dbServer:spCloud
:done