#!/bin/sh

help() {
    echo "How to use test suite:"
    echo "\tCreate a file with the format test_[a-z]+.hy in the tests directory"
    echo "\tMake the first line a comment with the expected exit code: E.g. // exit 1"
    echo "\tIf there's any stdout you want to test, create a file with the same name but with .txt instead of .hy"
    echo "\tThe script will then do a diff compare to make sure it's the same"
    echo "\tNo need to provide the .txt file if there's no output to test"
    echo
}


## Variables 
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RESET='\033[0m'
PASS=0
FAIL=0
TOTAL=0

# Helpful details for shell script
if [ "$#" -ne "0" ]
then
    if [ "$1" = "--help" ] || [ "$1" = "-h" ]
    then 
        help
    fi
fi


for file in tests/*
do
    file=$(basename $file)

    # Ignoring any other random files that aren't .txt or .hy
    if ! (echo $file | grep -Eq "test_[a-z]+\.(hy|txt)$")
    then
        echo "Skipping over ${YELLOW}${file}${RESET}: invalid filename"
        continue
    fi

    # If it's not a .hy file, ignore
    if ! echo $file | grep -Eq "\.hy$"
    then 
        continue
    fi


    EXIT_CODE=$(head -n1 tests/"$file" | grep -Eo "[0-9]+$")
    if  [ "$EXIT_CODE" = "" ]
    then 
        echo "Skipping over ${YELLOW}${file}${RESET}: please provide exit code in first line"
        continue
    fi

    # Attempt to build the executable
    ./build/hydro tests/"$file" >> /dev/null >&2

    # Build failed
    if [ "$?" -ne "0" ]
    then
        echo "${RED}Build error${RESET} for ${YELLOW}${file}${RESET}, counting as fail"
        FAIL=$((FAIL + 1))
        TOTAL=$((TOTAL + 1)) 
        continue
    fi 

    # Now execute commands and compare to the test_one.txt file
    rootName=$(echo "$file" | grep -Eo "^test_[a-z]+")
    if [ -f "tests/${rootName}.txt" ]
    then 
        # Also need to compare stdout
        ./out > "tests/current_output"
        if [ "$EXIT_CODE" -ne "$?" ]
        then 
            echo "${RED}Fail${RESET} for ${YELLOW}${file}${RESET}, differing exit codes"
            FAIL=$((FAIL + 1))
        else

            if diff "tests/current_output" "tests/${rootName}.txt" > /dev/null
            then
                # They are identical
                echo "${GREEN}Pass${RESET} for ${YELLOW}${file}${RESET}, same exit code and stdout"
                PASS=$((PASS + 1))
            else
                # They aren't identical
                echo "${RED}Fail${RESET} for ${YELLOW}${file}${RESET}, differing stdout, same exit code"
                FAIL=$((FAIL + 1))
            fi 
            TOTAL=$((TOTAL + 1)) 
        fi

    else
        # Don't bother comparing stdout
        ./out 
        if [ "$EXIT_CODE" -eq "$?" ]
        then
            echo "${GREEN}Pass${RESET} for ${YELLOW}${file}${RESET}, same exit code"
            PASS=$((PASS + 1))
        else 
            echo "${RED}Fail${RESET} for ${YELLOW}${file}${RESET}, differing exit codes"
            FAIL=$((FAIL + 1))
        fi 
        TOTAL=$((TOTAL + 1)) 
    fi

done

if [ -f "tests/current_output" ]
then 
    rm tests/current_output
fi

echo
echo "Test suite completed: "
echo "\t${GREEN}${PASS} passed${RESET}"
echo "\t${RED}${FAIL} failed${RESET}"
echo "\t${TOTAL} total"

