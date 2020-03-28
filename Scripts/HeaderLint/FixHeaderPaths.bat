@echo off

rem 	$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $

rem 	Grab the lint script from here: https://github.com/coderespawn/ue4-code-headers-lint
rem 	The script parses all the engine header files and plugin header files and 
rem 	includes them from project relative paths, as required by marketplace build system
rem		Be sure to commit your changes before you run this script!

python D:\gamedev\ue4\ue4-code-headers-lint\fix_header_abs.py header_lint.pf.json "../../Source"
pause