#!/bin/bash

DESC=$(git describe 2>/dev/null)
if [ $? -ne 0 ]; then
	DESC=$(git rev-parse --short HEAD 2>/dev/null)
fi
if git status | grep -q 'nothing to commit, working tree clean'; then
	# nothing to do
	:
else
	STATUS=$( (git status; git diff) | sha256sum | cut -b 1-8 )
	DESC="$DESC+$STATUS"
fi
echo $DESC
