#! /bin/sh

set -e 
doxygen Doxyfile
./filter_graph.py a/html/structbatadv__priv__coll__graph.dot batpriv.dot

dot -Gsplines=true -Gsep="+25,25" -Goverlap=scalexy -Gnodesep=0.6 batpriv.dot -Tpng -o batpriv.png
