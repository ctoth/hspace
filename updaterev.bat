@echo off
"C:\Program Files\TortoiseHg\hg" id -n > tmprev.txt
for /F %%G in (tmprev.txt) DO set _revision=%%G
"C:\Program Files\TortoiseHg\hg" branch > tmprev.txt
for /F %%G in (tmprev.txt) DO set _branch=%%G
echo #define HS_REVISION "%_revision% %_branch%" > HSRevision.h

