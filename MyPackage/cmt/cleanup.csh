# echo "cleanup MyPackage MyPackage-00-00-00 in /afs/cern.ch/user/m/miholzbo/athena/JPsi"

if ( $?CMTROOT == 0 ) then
  setenv CMTROOT /cvmfs/atlas.cern.ch/repo/sw/software/x86_64-slc6-gcc49-opt/20.7.7/CMT/v1r25p20160527
endif
source ${CMTROOT}/mgr/setup.csh
set cmtMyPackagetempfile=`${CMTROOT}/${CMTBIN}/cmt.exe -quiet build temporary_name`
if $status != 0 then
  set cmtMyPackagetempfile=/tmp/cmt.$$
endif
${CMTROOT}/${CMTBIN}/cmt.exe cleanup -csh -pack=MyPackage -version=MyPackage-00-00-00 -path=/afs/cern.ch/user/m/miholzbo/athena/JPsi  $* >${cmtMyPackagetempfile}
if ( $status != 0 ) then
  echo "${CMTROOT}/${CMTBIN}/cmt.exe cleanup -csh -pack=MyPackage -version=MyPackage-00-00-00 -path=/afs/cern.ch/user/m/miholzbo/athena/JPsi  $* >${cmtMyPackagetempfile}"
  set cmtcleanupstatus=2
  /bin/rm -f ${cmtMyPackagetempfile}
  unset cmtMyPackagetempfile
  exit $cmtcleanupstatus
endif
set cmtcleanupstatus=0
source ${cmtMyPackagetempfile}
if ( $status != 0 ) then
  set cmtcleanupstatus=2
endif
/bin/rm -f ${cmtMyPackagetempfile}
unset cmtMyPackagetempfile
exit $cmtcleanupstatus

