# This awk script translates opengl shader files,
# given as input, into a c header file which contains
# the source code of the shader as a string (named
# after the shader file). 
# Include Directives with the syntax
#   $include "<subpath_to_file>"
# are replaced by the the contents of the corresponding
# file, searched at src/shader/<subpath_to_file>.
# This search and replace procedure is performed
# recursively through all include files.

function format_c_code() {
    # Replace '\' with '\\' to avoid interpreation
    # of escape sequences
    escline = $0
    gsub(/\\/, "\\\\", escline)

    $0 = "\t\"" escline "\\n\" \\"
}

function extract_incfile() {
    # Get location of first quotation mark after $include
    if ((i = index($0, "\"")) == 0) {
        print "expected '\"' after '$include'" > "/dev/stderr"
        exit
    }
    # Get string after first quotation mark
    rem = substr($0, i+1)
    if ((i = index(rem, "\"")) == 0) {
        print "expected '\"' after '$include \"':" > "/dev/stderr"
        exit
    }
    # Get filename from string in between two quotation marks
    fname = substr(rem, 1, i-1)
    res = "core/shader/" fname
    if (system("test -f " res) == 0) {
        return res
    }
    res = "lib/shader/" fname
    if (system("test -f " res) == 0) {
        return res
    }
    print res " not found"
    exit
}

function print_include_contents(incf) {
    incf = extract_incfile()

    # Ignore already included files stored in 'included'
    if (incf in included) return
    included[incf] = 1

    while (getline < incf) {
        if ($0 ~ /^\$include/) {
            # continue recursively for other $include directives
            print_include_contents()
        }
        else {
            format_c_code()
            print
        }
    }
}

BEGIN {
    # Get variable name from shader file name 
    getfilename_cmd = "basename " ARGV[1]
    getfilename_cmd | getline fname
    split(fname, fname_, ".")
    print "#define " toupper(fname_[1]) "_" toupper(fname_[2]) "_SRC \\"
}
/^\$include/ {
    print_include_contents()
    next
}
{
    format_c_code()
    print
}
END {
    print ""
}
