#!/bin/sh

if ! [ -d "env" ];
then
    python -m venv env
    source env/bin/activate
    python -m pip install -r tests/requirements.txt
fi

python tests/testsuite.py