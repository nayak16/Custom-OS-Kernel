#!/bin/bash

# This is a shell script for keeping the support code
# for the 15-410 kernel up to date.
#
# Ivan Jager (aij)
#
###########################################################
# Usage:
#    update.sh <how> [query]
#
#       <how>:
#              afs - update files by comparing md5sums
#                    taken from course afs space. If
#                    files don't match digests, grab
#                    updated files from course afs space.
#
#              web - same but from course webspace.
#
#              offline - build support libraries using
#                    code present in local directories.
#                    This method is *not* recommended for
#                    typical use.
#
###########################################################


# location of official support code and libraries
AFS_PATH=${AFS_PATH-/afs/cs.cmu.edu/academic/class/15410-s16/Web/update/proj4}
WEB_URL=${WEB_URL-'http://www.cs.cmu.edu/~410-s16/update/proj4'}

# the name of the file containing the MD5s for the files we want to update
WHAT=all.md5s

#fetches a file from the update area, either to a specific destination, or
# to the same relative filename if no destination is specified
function fetch () {
    local mode=$1
    local from=$2
    local to=${3:-$2}
    # make the directory to put the file in if it doesn't exist
    test -d `dirname "$to"` || mkdir -p `dirname "$to"`

    case "$METHOD" in
	afs)
        cp "$AFS_PATH/$from" "$to.tmp" && mv "$to.tmp" "$to" ;;
	web)
        wget -nv "$WEB_URL/$from" -O "$to.tmp" && mv "$to.tmp" "$to" ;;
	query)
	    test -f "$to" && echo "$from hasn't been updated since `stat -c %y $to`" || echo "File $to is missing"
	    NEEDS_UPDATING=true
	    return ;;
	*) echo fetch: method invalid: $METHOD; exit 2 ;;
    esac &&
    chmod $mode "$to"
}

# reads a list of md5s to check and update from stdin
function do_update () {
    rc=0
    while read mode md5 filename
    #for filename in `md5sum -c | tee >(cat >&3) | grep -v OK$ | sed 's/: FAILED.*//g'`
    do
	current=`md5sum "$filename" | cut -d ' ' -f 1`
	if [ "$current" != "$md5" ]
	then
	    echo Update "$filename"
	    fetch $mode "$filename" || rc=$?
	fi
    done 3>&1
    test -z "$NEEDS_UPDATING" || { /bin/echo -e '****\nUpdates are available. Please run make update sometime soon.\n****\7'; sleep 1; /bin/echo -e '\7'; sleep 1; /bin/echo -e '\7'; }
    return $rc
}


# find out what method we were told to use
# One of afs, web, or offline. (rsync coming "soon")
case $1 in
    '')
	{ test -d $AFS_PATH && METHOD=afs ; } || \
	{ which wget >/dev/null 2>&1 && METHOD=web ; } || \
	METHOD=offline
	;;
    afs|web|offline)
	METHOD=$1
	;;
    *)
	echo usage: $0 'afs|web|offline' '[query]'
	exit 9
	;;
esac

if [ "$METHOD" = offline ] ; then
    echo UPDATE METHOD IS OFFLINE. This is not recommended.
    echo Be sure to update and make sure your code works before handing it in.
    sleep 3
    exit
fi

# this could be changed to not make a copy when updating from AFS
fetch 644 $WHAT .md5s || { echo Failed to fetch the list of MD5s.; exit 1; }
echo "# Fetched by $0 $@ at `date`" >>.md5s

test "$2" = query && METHOD=query

grep -v '^#' .md5s | do_update

