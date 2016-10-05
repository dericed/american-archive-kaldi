#!/bin/bash
PREFIX=$(dirname $0)
PATHSETTER=$PREFIX/set-kaldi-path.sh
. $PATHSETTER
TMPDIR=/var/extra/audio/work

nj=8
decode_nj=4
lmw=14.0
amw=`echo $lmw | awk '{print 1/$1}'`
#amw=0.083333
QUEUE=false

dict=$1
tmplang=$2
lang=$3
phones=$4

bash utils/prepare_lang.sh --phone-symbol-table $phones $dict "<background>" $tmplang $lang
