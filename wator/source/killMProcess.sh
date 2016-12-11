#/bin/bash
######################################################################
#
#       Kill MASS.MProcss for user
#       ---------------------------
#       Just a small script to KILL all MASS.MProcess process for the
#	user on each of the machines listed in the machinefile.txt file
#
#       By:     Richard Romanus
#       Date:   07/01/2013
#
######################################################################
filename="./machinefile.txt"

for host in `cat ${filename}`
do
	echo -e "\n${host}"
	echo -e   "============="
	tmp=`ssh ${USER}@${host} 'ps -ef  | grep "${USER}"
			' | grep "mprocess" | grep -v "grep"` 
	echo -e ">>> $tmp"

	for pNum in `echo $tmp | awk '{print $2}' `
	do
		echo -e "> kill ${pNum}" 
		ssh ${USER}@${host} "kill ${pNum}"
		echo -e "  -- DONE! \n"
	done
done

echo -e "\n"
