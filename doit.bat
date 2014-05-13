rem
rem  evaluate a list of products
rem
for /F "usebackq delims==" %%X in (`type doit.txt`)  do ConsoleApplication1 %%X %%X 3000 forceIterations
