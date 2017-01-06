#!/bin/sh

echoerr() { echo "$@" 1>&2; }

#check if file or stdin
datafilepath="datafile$$"
if [ "$#" = "1" ]
then
    cat > "$datafilepath"
elif [ "$#" = "2" ]
then
    datafilepath=$2
fi

#Code to calculate for rows
if [ $1 == -r* ]
then
    echo "calculating row stats"
    while read i #reads all lines
    do
		sum=0
		count=0
		for num in $i
		do
			sum=`expr $sum + $num` #adds number to sum total
			if [ $count == 0 ] #move vales to temp file for sorting
			then
			echo "$num" > $$tempfile #if first line then restart file
			else
			echo "$num" >> $$tempfile #otherwise add to file
			fi
			count=`expr $count + 1`
		done
		
		avg=`expr $sum / $count` #finds average
		rem=`expr $sum % $count` #checks for a remainder
		if [ `expr $rem / $count` >= .5 ]	#checks if number needs rounded up
		then
		avg=`expr $avg + 1`
		fi

		med=`expr $count / 2`
		med=`expr $med + 1`		#finds place of median value
		
		sort -n $$tempfile -o $$tempsorted #sorts temp file

		tmpcnt=0
		while read tempnum   #finds median
		do
			tmpcnt=`expr $tmpcnt + 1`
			if [ $med == $tmpcnt ]
			then
			med=$tempnum	#sets median to value stored in median slot
			fi
		done < $$tempsorted
		
		echo -e "$avg \t\t $med" #prints results
		rm -f $$tempfile   #deletes temp files
		rm -f $$tempsorted
    done < $$datafilepath
    echoerr 0
	
#calculates column info
elif [ $1 == -c* ]
then
   echo "calculating column stats";
    numrows=0
    while read i  #saves each column to separate temp files and sorts each
    do
		numcols=0
		for num in $i
		do
			echo "$num" >> $$tempfile$numcols
			sort -n $$tempfile$numcols -o $$tempfile$numcols
			numcols=`expr $numcols + 1`
		done
		numrows=`expr $numrows + 1`
    done < $$datafilepath

	med=`expr $count / 2`
	med=`expr $med + 1`		#finds place of median value
    count=0
    while ($count < $numcols)
    do
		sum=0
		tmpcnt=0
		while read line
		do
			sum=`expr $sum + $line` #sums all values from column
			tmpcnt=`expr $tmpcnt + 1`
			if [ $med == $tmpcnt ]  #finds median value
			then
			medArray[$count]=$line #makes array of medians
			fi
		done < $$tempfile$count
		avg=`expr $sum / $numrows` #finds average
		rem=`expr $sum % $numrows` #checks for a remainder
		if [ `expr $rem / $numrows` >= .5 ]	#checks if number needs rounded up
		then
		avg=`expr $avg + 1`
		fi
		avgArray[$count]=$avg  #makes array of averages
		count=`expr $count + 1`
    done

	count=0
	echo "Averages:"
	while ($count < $numcols) #prints averages
	do
		echo -e -n "${avgArray[$count]} \t"
		count=`expr $count + 1`
	done
	echo -e "\n"

	count=0
	echo "Medians:"
	while ($count < $numcols) #prints medians
	do
		echo -e -n "${medArray[$count]} \t"
		count=`expr $count + 1`
	done
	echo -e "\n"

	count=0
	while ($count < $numcols) #deletes temp files
	do
	   rm -f $$tempfile$count
	   count=`expr $count + 1`
	done
	echoerr 0
fi