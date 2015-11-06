# Need to make sure that the reference to master is available.
# We know that HEAD is checked out so that the tests on that source can be run.

# configure the sh environment to run scripts from Git's bin dir
ls &> /dev/null # checking for ls script on the path
if [ $? -ne 0 ]; then
    PATH=/bin:/usr/bin:$PATH
fi

ERRFILE=jenkins.check_eol.sh.err
rm -f $ERRFILE

git diff --name-only `git merge-base origin/master HEAD` HEAD | grep -v -E "(test/.*\\.js|\\.cmd|\\.baseline)" | xargs -I % ./jenkins.check_file_eol.sh %

if [ -e $ERRFILE ]; then # if error file exists then there were errors
    >&2 echo "--- ERRORS ---" # leading >&2 means echo to stderr
    cat $ERRFILE 1>&2 # send stdout to stderr for formatting as error output
    exit 1 # tell the caller there was an error (so Jenkins will fail the CI task)
else
    echo "--- NO PROBLEMS DETECTED ---"
fi
