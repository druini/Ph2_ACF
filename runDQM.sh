#!/bin/bash
#set -o nounset

#-----------------------------------------------------------------------------
# Generate DQM output for all the raw data taken during the May'16 Beamtest
# as well as in the Lab thereafter. 
#
# By default, only data files taken in the last 30 minutes will be processed.
# If you want to reprocess all the files for a particular folder, change 
# -30 to +0 in the find command, delete/rename the corresponding run_processed 
# file that you will find in the WORKDIR and listed in the array runProcList below.
# At the bottom you will also need to comment out the line(s) you do not want to
# process
#
# v1.0 - S. Sarkar  17/06/2016
# ----------------------------------------------------------------------------
let "ignore_ecode = 1"
let "min_size = 1024 * 1024" # MB
let "max_size = 500 * min_size" # MB

# Globals
WORKDIR=$(pwd)
TREEOUTDIR=/afs/cern.ch/work/c/cmstkph2/public/BeamTestDQM/Oct2016/root_files

# Define all types of available data 
# Raw Data folder
declare -a rawFileFolders=("/cmsuptrackernas/Raw/2016/October2016/xDAQ_runs"
    "/cmsuptrackernas/Raw/2016/imperial"
    "/cmsuptrackernas/Raw/2016/October2016/miniDAQ_runs")

# Output web folder
declare -a webFolders=("/afs/cern.ch/work/g/gauzinge/public/TB_DQM/Debug"
    "/afs/cern.ch/work/g/gauzinge/public/TB_DQM/LabTest"
    "/afs/cern.ch/work/g/gauzinge/public/TB_DQM/Oct2016/miniDAQ")

# exclusive command line options
declare -a optionList=(" --cbcType=4 --swap --filter --skipDebugHist"
    " --cbcType=2 --reverse"
    " --cbcType=4 --filter --tree --skipDebugHist")

# filename containing runs already processed
declare -a runProcList=("run_processed.txt" 
    "run_processed_imperial.txt"
    "run_processed_strasbourg.txt")

# function definition
function runDQM() {
  local indx=$1
  local rawFileFolder=${rawFileFolders[$indx]}
  local webFolder=${webFolders[$indx]}
  local commandOpt=${optionList[$indx]}
  local RUNLIST=${runProcList[$indx]}

  echo "--- Processing files in $rawFileFolder"
  
  ls -1 $rawFileFolder > /dev/null
  [ $? -eq 0 ] || { echo "=> Raw file folder $rawFileFolder not found/readable"; return 1; }
    
  # go to the right directory
  cd $WORKDIR
  source ./setup.sh
    
  [ -r $RUNLIST ] || touch $RUNLIST

  # find the last file (may be the one for which data taking is going on right now)
  lastfile=$(ls -1tr $rawFileFolder/*.raw 2> /dev/null | tail -1)
  [ $? -eq 0 ] || { echo "=> Strange! could not find the last file in $rawFileFolder"; return 2; }
  echo "=> Most recent file is: $lastfile"
    
  # now find the latest files (changed last hour)
  for rawfile in $(find $rawFileFolder -maxdepth 1 -name "*.raw" -cmin -300000); do
     echo "=> Processing $rawfile"
	
     basefile=$(basename $rawfile)
     basename=$(echo $basefile | sed "s/\.raw//")
	
     # form the dqm file name
     dqmfile=$basename"_dqm.root"
	
     # check if the dqm file is already available
     grep -x $dqmfile $RUNLIST > /dev/null
     [ $? -eq 0 ] && { echo "=> $dqmfile already available!"; continue; }
	
     # find modification timestamp of the raw file to check if it is closed
     let "lastmod = $(stat -c "%Y" $rawfile)"
     let "size = $(stat -c "%s" $rawfile)"
     [ $size -lt $min_size ] && { echo "=> File $rawfile too small in size($size bytes), skipping"; continue; }
	
     # convert the file but do not finalize it, i.e do not add the filename in the list of already processed files
     let "checkout = 1"
     # check if the file may still getting updated 
     if [ "$rawfile" == "$lastfile" ]; then
	let "now = $(date "+%s")"
	let "diff = now - lastmod"
	if [ $diff -lt 300 ]; then
	  echo "=> Raw file $rawfile may still be updated, size=$size bytes." 
	  let "checkout = 0"
	fi
     fi
	
     # Now convert
     echo $(pwd)
     command="bin/miniDQM --dqm --file=$rawfile --output=$webFolder --ncolumn=1 $commandOpt"
     [ $size -gt $max_size ] && command=$command" --nevt=2000000"
     echo $command
          $command  # bash seems to work this way

     # for dev version we do not use the exit code yet	
     let "ecode = $?"
     [ $ignore_ecode -gt 0 ] && let "ecode=0"
     if [ $ecode -eq 0 ]; then
	[ $checkout -eq 1 ] && echo $dqmfile >> $RUNLIST
	echo mv Rawtuple_$dqmfile $TREEOUTDIR
  	     mv Rawtuple_$dqmfile $TREEOUTDIR
     else
	echo "=> $rawfile: raw->dqm failed!"
     fi

     break
  done
} 

# Now run
# Simply comment out the other if you want to run over only one type

runDQM 0  # GlibSupervisor BT+Lab
#runDQM 1  # miniDAQ Imperial Lab 
#runDQM 2  # miniDAQ Strasbourg Lab

exit 0
