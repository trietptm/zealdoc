#
# fxg - find, xargs and grep
# Usage:
# $1 - word you want to lookup
#
function fxg()
{
	if [ $# -ne 1 ]; then
		echo 'Usage:'
		echo 'fxg $word_you_lookup'
		exit 1
	fi
	find . -name .svn -prune -o -name .pc -prune -o -name CVS -prune -o -print | xargs grep $1
	return 0
}