call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
call C:\Python\Python311\python.exe -m venv venv311
call venv311\Scripts\activate.bat
pip install -r requirements.txt
mkdir build
set PYTHONPATH=%cd%\python_modules\jupedsim;%cd%\build\lib\Release
cd build
echo %PYTHONPATH%
cmake .. -DBUILD_TESTS=ON
cmake --build . --config Release
cmake --build . --config Release --target unittests
cd ..
python -m build -w
