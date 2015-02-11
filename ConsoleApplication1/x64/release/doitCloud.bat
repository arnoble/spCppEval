rem
rem  evaluate a list of products to spCloud
rem
for /F "usebackq delims==" %%X in (`type %1`)  do ConsoleApplication1 %%X %%X 1 forceIterations dbServer:spCloud
for /F "usebackq delims==" %%X in (`type %1`)  do ConsoleApplication1 %%X %%X 10000 forceIterations  dbServer:spCloud
