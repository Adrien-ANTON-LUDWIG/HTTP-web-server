#!/bin/sh

if [ "$1" -eq "--nologs" ] then
    find ./config_tests -name '*.json' -not '(' -exec cd ../ && ./spider -t tests/{} >> /dev/null 2>> /dev/null \; -exec echo 'Dry run: ' {} ' passed' \; ')' -a -exec echo 'Dry run: ' {} ' failed' \;
else
    find ./config_tests -name '*.json' -not '(' -exec cd ../ && ./spider -t tests/{} \; -exec echo 'Dry run: ' {} ' passed' \; ')' -a -exec echo 'Dry run: ' {} ' failed' \;
fi