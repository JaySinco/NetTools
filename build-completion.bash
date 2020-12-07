#!/bin/bash

_build_completions()
{
    if [ -d "dest" ]; then
        COMPREPLY=($(compgen -W "$(find dest -maxdepth 1 -name *.vcxproj | grep -v [[:upper:]] | awk -F [/.] '{print $2}')" -- "${COMP_WORDS[-1]}"))
    fi
}

complete -F _build_completions build.sh
