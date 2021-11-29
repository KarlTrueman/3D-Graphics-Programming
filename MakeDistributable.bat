del /s /q Demo\*.*
rd Demo
md Demo
md Demo\Data

Xcopy ThreeGPStart\External\bin Demo\
Xcopy /E /I ThreeGPStart\Data Demo\Data\
copy x64\Release\ThreeGPStart.exe Demo\ThreeGPStart.exe