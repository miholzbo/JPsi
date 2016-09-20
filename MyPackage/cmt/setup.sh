# echo "setup MyPackage MyPackage-00-00-00 in /afs/cern.ch/user/m/miholzbo/athena/JPsi"

if test "${CMTROOT}" = ""; then
  CMTROOT=/cvmfs/atlas.cern.ch/repo/sw/software/x86_64-slc6-gcc49-opt/20.7.7/CMT/v1r25p20160527; export CMTROOT
fi
. ${CMTROOT}/mgr/setup.sh
cmtMyPackagetempfile=`${CMTROOT}/${CMTBIN}/cmt.exe -quiet build temporary_name`
if test ! $? = 0 ; then cmtMyPackagetempfile=/tmp/cmt.$$; fi
${CMTROOT}/${CMTBIN}/cmt.exe setup -sh -pack=MyPackage -version=MyPackage-00-00-00 -path=/afs/cern.ch/user/m/miholzbo/athena/JPsi  -no_cleanup $* >${cmtMyPackagetempfile}
if test $? != 0 ; then
  echo >&2 "${CMTROOT}/${CMTBIN}/cmt.exe setup -sh -pack=MyPackage -version=MyPackage-00-00-00 -path=/afs/cern.ch/user/m/miholzbo/athena/JPsi  -no_cleanup $* >${cmtMyPackagetempfile}"
  cmtsetupstatus=2
  /bin/rm -f ${cmtMyPackagetempfile}
  unset cmtMyPackagetempfile
  return $cmtsetupstatus
fi
cmtsetupstatus=0
. ${cmtMyPackagetempfile}
if test $? != 0 ; then
  cmtsetupstatus=2
fi
/bin/rm -f ${cmtMyPackagetempfile}
unset cmtMyPackagetempfile
return $cmtsetupstatus

