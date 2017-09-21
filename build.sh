#!/bin/bash
gcc fat.c library.c fat_structure.h -o fat
gcc -pthread badblock_recovery.c library.c fat_structure.h -o badblock_recovery
