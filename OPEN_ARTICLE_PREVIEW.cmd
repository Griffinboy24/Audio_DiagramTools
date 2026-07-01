@echo off
setlocal
cd /d "%~dp0"
python scripts\serve_preview_lab.py 8066 --project hise-dsp-buffer --open
pause
