del /s /q x64\Release\*.*
del /s /q x64\Debug\*.*
del /s /q .vs\*.*
del /s /q ThreeGPStart\x64\Release\*.*
del /s /q ThreeGPStart\x64\Debug\*.*
del /s /q ThreeGPStart\Debug
del /s /q ThreeGPStart\Release
del /s /q Debug\*.*
del /s /q Release\*.*

rd /s /q x64
rd /s /q .vs
rd /s /q Debug
rd /s /q Release
rd /s /q ThreeGPStart\Debug
rd /s /q ThreeGPStart\Release
rd /s /q ThreeGPStart\x64