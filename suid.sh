#!/bin/sh -e

su root -c 'chown root:wheel npmount; chmod 4755 npmount'
ls -l npmount
