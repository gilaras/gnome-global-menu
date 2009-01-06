#! /bin/bash

VALA_VERSION=0.5.2

function patch {
	edit=$1;
	shift;
	for i in $*; do
		echo -n patching $i with $edit ...
		if ! sed -e "$edit" $i > $i.new; then
			echo failed.
		else
			if cmp $i $i.new; then
				echo noting was done.
				exit 1;
			else 
				echo OK.
			fi;
		fi
		mv $i.new $i; 
	done;
}

