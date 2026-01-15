@echo off

tcc main.c .\lua_win32\lua54.dll -I.\lua_win32\include -luser32
