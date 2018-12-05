#!/usr/bin/python3

import os
import sys as sys
import re

reg=re.compile('^[a-zA-Z]+$')

with open('pg29765.txt') as f:
    for line in f:
        if len(line.split()) == 1:
            if reg.match(line) and len(line) > 5:
                line = re.sub('\n', '', line)
                print(line)

