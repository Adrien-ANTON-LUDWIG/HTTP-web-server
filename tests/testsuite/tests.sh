#!/bin/bash

function test_script()
{
    count_tests=$(( "$count_tests" + 1 ))
    request=""

    while read line && [ "$line" != '---' ]
    do
        request+="$line"
        request+='\r\n'
    done < $file

    response=`echo -ne "$request" | nc localhost 8000`
    status=`echo "$response" | awk '/HTTP\/1.1/{print $2}'`
    expected_status=$( tail -n 1 "$file" )

    if [ "$status" == "$expected_status" ]
    then
        echo -e '\e[1;32m OK \e[0m'
        count_passed_tests=$(( "$count_passed_tests" + 1))
    else
        echo -ne '\e[1;31m KO \e[0m' "in $file : expected $expected_status got $status"
        echo " - Request : $request"
        failed=1
    fi
}

failed=0
count_tests=0
count_passed_tests=0

for dir in $(find tests/testsuite/scripts/* -type d);
do
    for file in $(find "$dir"/* -type f);
    do
        test_script "$file"
    done
done

echo "Summary : $count_passed_tests / $count_tests"

exit "$failed"