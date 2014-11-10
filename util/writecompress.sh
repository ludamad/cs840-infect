set -e
rm -f build/my_pipe
mkfifo build/my_pipe
gzip --fast -c < build/my_pipe > $1 &
./run.sh -w $2 build/my_pipe
