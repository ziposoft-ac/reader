#!/bin/bash
#! /bin/bash

if [ "$#" -ne 1 ]
then
  echo "Error: provide executable"
  exit 1
fi

EXE=$1
#valgrind -s --leak-check=full --track-origins=yes --tool=massif --stacks=yes ${EXE}
valgrind -s --leak-check=full --track-origins=yes --show-leak-kinds=all  ${EXE}
