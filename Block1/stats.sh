#!/bin/bash
function cleanup { rm -f $$*; exit 0; }	#removes temp files
trap cleanup SIGHUP SIGINT SIGTERM	#traps kill command
echoerr() { echo "$@" 1>&2; }	#to print errors to screen
function err { echoerr "./stats cannot read $datafilepath"; exit 1; }	#echo out error messages
printCol(){ echo $1; while read line; do echo -e -n "$line\t"; done < $2; echo -e "\n"; }	#prints column data
makeTempFiles(){ colCount=1; rowCount=1
	while [[ $colCount < $numCols ]]; do echo cut =-c$colCount $$datafilepath >> $$tempcol$colCount; colCount=`expr $colCount + 1`; done	#creates temp files for each column
	while [[ $rowCount < $numRows ]]; do echo head -$rowCount $datafilepath | tail -1 | tr "\t" "\n" >> $$temprow$rowCount; rowCount=`expr $rowCount + 1`; done; }	#creates temp files for each row
findMedSumAvg(){ tmpcnt=1; sum=0;
	sort -n $1 -o $$tempSorted
	while read num; do
		if [[ $tmpcnt == $(( ($2 / 2) + 1 )) ]]; then med=$num; fi	#saves median value
		tmpcnt=`expr $tmpcnt + 1`; sum=`expr $sum + $num` #adds number to sum total
	done < $$tempSorted;
	avg=$(( ($sum + ( $2 / 2)) / $2 )); }	#finds average
datafilepath="$$datafile"
if [[ wc -c $datafilepath == 0 ]]; then echoerr "FILE IS EMPTY"; exit 1; fi;	#checks if file is empty
if [ "$#" = "0" ]; then err	#if nothing is passed in
elif [ "$#" = "1" ]; then cat > "$datafilepath"		#if it should be read from stdin
elif [ "$#" = "2" ]; then datafilepath=$2		#if it should be read from provided file
else err; fi	#if too many options are provided
if [[ ! -e $datafilepath ]] || [[ ! -r $datafilepath ]] || [[ ! -f $datafilepath ]]; then err;  fi;	#checks that file exists and has read permission
numCols=$((head -n1 myfile.txt | grep -o " " | wc -l)); numRows=$((wc -l $datafilepath)); makeTempFiles
rowCount=1; colCount=1
if [[ $1 == -r* ]]; then	#Code to calculate for rows
	echo -e "Average\tMedian"	#print headers
	while [[ $rowCount < $numRows ]]; do
		findMedSumAvg $$temprow$rowCount $numCols	#passes temp file and number of columns
		echo -e "$avg\t$med"	#prints results
		rowCount=`expr $rowCount + 1`
	done
elif [[ $1 == -c* ]]; then	#Code to calculate for columns
	while [[ $colCount < $numCols ]]; do
		findMedSumAvg $$tempcol$colCount $numRows	#passes temp file and number of rows
		echo -e $med >> $$tempMedian	#adds medians to temp file
		echo -e $avg >> $$tempAverage	#adds averages to temp file
		colCount=`expr $colCount + 1`
	done
	printCol "Averages:" $$tempAverage
	printCol "Medians:" $$tempMedian
else err; fi	#prints error if neither rows nor columns are specified
cleanup	#deletes temp files