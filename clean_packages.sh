#!/bin/bash

echo "Script for cleaning binaries from stack.";
echo "Author: Gerardo Aragon-Camarasa, Jan-2013"
echo

echo "Removing backup files."
find ./ -name '*~' | xargs rm



