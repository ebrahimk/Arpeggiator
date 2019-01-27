#!/bin/bash


if [ "$1" == "test" ]; then
echo "Making backups..."
cp lab6_m128.c ./backup/lab6_backup.c
cp music.c ./backup/music_backup.c
cp music.h ./backup/music_backup.h

echo "copying test files..."
cp ./dev/lab6_dev.c lab6_m128.c 
cp ./dev/music_dev.c music.c
cp ./dev/music_dev.h music.h

echo "DONE" 
fi

if [ "$1" == "undo" ]; then

echo "restoring backups..."
cp ./backup/lab6_backup.c lab6_m128.c
cp ./backup/music_backup.c music.c 
cp ./backup/music_backup.h music.h

echo "DONE" 
fi
