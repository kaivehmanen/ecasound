# ----------------------------------------------------------------------
# File: ecasound/manual-tests/run-all.sh
# License: GPL (see ecasound/{AUTHORS,COPYING})
# ----------------------------------------------------------------------

for i in `ls test-*.sh` ; do
  ./$i
  if test $? != 0; then
    break
  fi
done

