#!/bin/bash

DESC=$(git describe --tags 2>/dev/null)
REV=$(git rev-parse --short HEAD 2>/dev/null)
if [ -n "$DESC" ]; then
	DESC="${DESC}-${REV}"
else
	DESC="$REV"
fi
if git status -uno | grep -q 'nothing to commit'; then
	# nothing to do
	:
else
	STATUS=$( (git status; git diff) | sha256sum | cut -b 1-8 )
	DESC="$DESC+$STATUS"
fi
echo $DESC
