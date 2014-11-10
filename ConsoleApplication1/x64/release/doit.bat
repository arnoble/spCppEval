rem
rem  evaluate a list of products
rem
for /F "usebackq delims==" %%X in (`type %1`)  do ConsoleApplication1 %%X %%X 1 forceIterations
for /F "usebackq delims==" %%X in (`type %1`)  do ConsoleApplication1 %%X %%X 10000 forceIterations
