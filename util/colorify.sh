set -e

E='([\d\.ms\(\)]+)[ \t]*'

trap "" INT

colout "^$E$E$E$E$E$E$E" \
        yellow,white,blue,red,red,red,yellow \
        normal,normal,bold,faint,normal,bold,normal \
    | colout 'dist:.*' red normal \
    | colout '\*\*\*\* END.*\*\*\*\*' white \
    | colout '\*\*\*\* START.*\*\*\*\*' white \
    | colout '(func )(.*)(\..*):' yellow,white,yellow faint,faint,faint \
    | colout '(func )(.*):' yellow,white faint,faintl \
\
    | colout '\-\-\-\-.*' white bold \
    | colout 'Analysis complete!' white faint \
    | colout 'Saved.*' cyan bold \
    | colout 'Saved.*' cyan bold \
    | colout 'Will be reading.*' green bold \
    | colout 'Will be writing.*' green bold \
    | colout '^Simulation.*:.*' white  \
    | colout 'Analysis took .*ms\.' white faint \
    | colout '(average\s+)([\.\d\,]+)(ms)' cyan,white,cyan normal,normal,normal \
    | colout '(total\s+)([\.\d\,]+)(ms)' cyan,white,cyan normal,bold,normal \
    | colout '>>' white bold \
    | colout '(calls\s+)([\.\d\,]+)' cyan,white normal,normal \
    | colout '(max\s+)([\.\d\,]+)(ms)' cyan,white,cyan normal,normal,normal \
    | colout '(std\.dev\s+)\+-([\.\d\,]+)(ms), \+-([\.\d\,]+%)' cyan,white,cyan,white normal,normal,normal,bold \
    | colout '\(.*ms\)' cyan normal \
    | colout 'Total \d+\.\d+' white bold \
    | colout 'Creating .*' red bold \
    | colout 'Connected .*' yellow bold \
    | colout 'Preprocessed .*' red bold \
    | colout 'Simulated .*' green bold \
    | colout 'Rate \d+\.\d+' white bold \
