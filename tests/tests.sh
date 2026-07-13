#!/bin/sh
#./configure CFLAGS="-coverage -fPIC" LDLIBS="-lgcov"
#make check
#lcov --capture --directory . --output-file coverage.info
#genhtml coverage.info --output-directory out
#firefox out/index.html
TEST_COUNT=0
TEST_SUCCESS=0
BASH_PATH="$(which bash)"
TIMEOUT="1s"

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
RESET="\033[0m"
BLUE="\033[38;2;0;127;255m"

if [[ -z ${BIN_PATH} ]]; then
    BIN_PATH="../src/42sh"
    echo "normal bin"
fi

if [[ -z ${TEST} ]]; then
    TEST="all"
    echo "testing all"
fi

run_test() {
    local description="$1"
    local input="$2"
    local expected_exit_code="$3"
    local expected_output=$($BASH_PATH -c "$input")

    ((TEST_COUNT++))

    output=$(timeout $TIMEOUT $BIN_PATH -c "$input")
    exit_code=$?

    if [[ "$output" == "$expected_output" && $exit_code == $expected_exit_code ]]; then
        echo -e "[${GREEN}OK${RESET}] $description"
        ((TEST_SUCCESS++))
    else
        echo -e "[${RED}FAIL${RESET}] $description"
        if [ $exit_code == 124 ]; then
            echo -e "${RED}>>>TIMEOUT<<<${RESET}"
        fi
        echo -e "${YELLOW}Input${RESET}: $input"
        echo -e "${YELLOW}Expected output${RESET}: $expected_output"
        echo -e "${YELLOW}Actual output${RESET}: $output"
        echo -e "${YELLOW}Expected exit code${RESET}: $expected_exit_code"
        echo -e "${YELLOW}Actual exit code${RESET}: $exit_code"
    fi
    echo
}

run_test_input() {
    local description="$1"
    local input="$2"
    local expected_exit_code="$3"
    local expected_output=$(echo "$input" | $BASH_PATH)

    ((TEST_COUNT++))

    output=$(timeout $TIMEOUT echo "$input" | $BIN_PATH)
    exit_code=$?

    if [[ "$output" == "$expected_output" && $exit_code == $expected_exit_code ]]; then
        echo -e "[${GREEN}OK${RESET}] $description"
        ((TEST_SUCCESS++))
    else
        echo -e "[${RED}FAIL${RESET}] $description"
        if [ $exit_code == 124 ]; then
            echo -e "${RED}>>>TIMEOUT<<<${RESET}"
        fi
        echo -e "${YELLOW}Input${RESET}: $input"
        echo -e "${YELLOW}Expected output${RESET}: $expected_output"
        echo -e "${YELLOW}Actual output${RESET}: $output"
        echo -e "${YELLOW}Expected exit code${RESET}: $expected_exit_code"
        echo -e "${YELLOW}Actual exit code${RESET}: $exit_code"
    fi
    echo
}

run_test_file() {
    local description="$1"
    local input="$2"
    local expected_exit_code="$3"
    local args="$4"
    local expected_output=$($BASH_PATH "$input" "$args")

    ((TEST_COUNT++))

    output=$(timeout $TIMEOUT $BIN_PATH "$input" "$args")
    exit_code=$?

    if [[ "$output" == "$expected_output" && $exit_code == $expected_exit_code ]]; then
        echo -e "[${GREEN}OK${RESET}] $description"
        ((TEST_SUCCESS++))
    else
        echo -e "[${RED}FAIL${RESET}] $description"
        if [ $exit_code == 124 ]; then
            echo -e "${RED}>>>TIMEOUT<<<${RESET}"
        fi
        echo -e "${YELLOW}Input${RESET}: $input"
        echo -e "${YELLOW}Expected output${RESET}: $expected_output"
        echo -e "${YELLOW}Actual output${RESET}: $output"
        echo -e "${YELLOW}Expected exit code${RESET}: $expected_exit_code"
        echo -e "${YELLOW}Actual exit code${RESET}: $exit_code"
    fi
    echo
}
if [[ $TEST == "all" || $TEST == "input" ]]; then
    echo
    echo -e "${BLUE}Testing stdin input...${RESET}"
    run_test_input "Input echo Hello World" "echo Hello World" 0
    run_test_input "Input echo with semicolon" "echo foo; echo bar" 0
    run_test_input "echo esc" "echo -e \\ | cat -e" 0
    run_test_input "Input multiples echo, multiples args" "echo 1 2; echo 3 4; echo a b; echo c d" 0
    run_test_input "Input simple comment handling" "#This is a comment
    echo Hello" 0
    run_test_input "Input less simpler comment handling" "#comment
    #another comment
    echo should print
    #echp should not print" 0
    run_test_input "Input complex comment" "echo \#escaped "#"quoted not#first #commented" 0
    run_test_input "Input many args" "echo Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Egestas purus viverra accumsan in nisl nisi." 0
    run_test_input "Input echo two args" "echo arg1 arg2" 0
    run_test_input "Input echo flag -n" "echo -n foo; echo bar" 0
    run_test_input "Input echo flag -E" "echo -E foo\nbar; echo baz" 0
    run_test_input "Input echo flag -ne" "echo -ne foo\n; echo -ne bar\n; echo baz" 0
    run_test_input "Input echo flags" "echo -neE foo\nbar\tbaz; echo test" 0
    run_test_input "Input true builtin" "true" 0
    run_test_input "Input false builtin" "false" 1
    run_test_input "Input if statement (true)" \
        "if true; then echo success; else echo fail; fi" 0
    run_test_input "Input if statement (false)" \
        "if false; then echo failed; else echo success; fi" 0
    run_test_input "Input multiples if (true)" "if if echo test; then true; else false; fi; then echo all good; else how are we here; fi" 0
    run_test_input "Input multiples if (false)" "if if echo test; then false; else true; fi; then echo all good; else echo how are we here; fi" 0
    run_test_input "Input complex if" "if if if echo 1; false; then echo should not print; else true; fi; then echo should print; false; else echo should not print; fi; then echo should not print; elif echo good; true; then echo nice work; else echo not good at all; fi" 0
    run_test_input "Input simple elif" "if echo test; false; then echo not supposed to write; elif echo good; true; then echo should display; else echo not good; fi" 0
    run_test_input "Input complex elif" "if echo test; false; then echo not supposed to write; elif echo good; false; then echo should display; elif echo test still; false; then false; elif true; then echo good; else echo not good; fi" 0
    run_test_input "Input simple single quotes" "echo '; echo foo'" 0
    run_test_input "Input simple double quotes" "" 0
    run_test_input "Input empty" "" 0
    run_test_input "Input not a command" "notreal" 127
    run_test_input "Input not a command again" "notrealecho aabbc" 127
    run_test_input "Input sequence 5" "seq 5" 0
    run_test_input "Input space seq 5" "     seq     5" 0
    run_test_input "Input ls" "ls" 0
    run_test_input "Input cat" "cat ../src/42sh.c" 0
    run_test_input "Input simple redir" "echo toto > test.txt; echo foo >> test.txt; cat < test.txt;" 0
    run_test_input "Input redir" "echo foo > arev.txt > cat.txt" 0
    run_test_input "Input simple pipe" "echo pipe | cat -e" 0
    run_test_input "Input compound list" "if echo 1; echo 2; echo 3; then echo all good; else echo not good; fi" 0
    run_test_input "Input multiple pipes" "ls -l | wc | wc" 0
    run_test_input "Input redir & pipe" "echo Hello > test2.txt | cat test2.txt" 0
    run_test_input "wrong command redir" 'mangemonvier 1 2 3 > test3.txt; echo $? ; cat test3.txt' 0
    run_test_input "Input bad separators" "if true then; echo not good" 2
    run_test_input "Input bad separators 2" "if echo check; then echo still good; else echo not good fi" 2
    run_test_input "Input while false" "while false; do echo hello; done" 0
    run_test_input "Input until true" "until true; do echo hello; done" 0
    run_test_input "Input or false" "if false || false; then echo not good; else echo good; fi" 0
    run_test_input "Input or true" "if true || echo 1; then echo good; else echo not good; fi" 0
    run_test_input "Input or true false" "if echo 1 || echo 2; then echo good; else echo not good; fi" 0
    run_test_input "Input or true true" "if true || true; then echo good; else echo not good; fi" 0
    run_test_input "Input and false false" "if false && false; then echo good; else echo not good; fi" 0
    run_test_input "Input and false true" "if false && true; then echo good; else echo not good; fi" 0
    run_test_input "Input and true false" "if true && false; then echo good; else echo not good; fi" 0
    run_test_input "Input and true true" "if true && true; then echo good; else echo not good; fi" 0
    run_test_input "Input and echo" "if echo 1 && echo 2; then true; else false; fi" 0
    run_test_input "R Input or false" "if false || false; then echo not good; else echo good; fi > altres1.txt; cat -e altres1.txt;" 0
    run_test_input "R Input or true" "if true || echo 1; then echo good; else echo not good; fi > altres2.txt; cat -e altres2.txt;" 0
    run_test_input "R Input or true false" "if echo 1 || echo 2; then echo good; else echo not good; fi > altres3.txt; cat -e altres3.txt;" 0
    run_test_input '$IFS' 'echo $IFS' 0
    run_test_input '$PWD' 'echo $PWD' 0
    run_test_input '$OLDPWD' 'echo $OLDPWD' 0
    run_test_input 'simple assign' 'i=1; echo $i' 0
    run_test_input 'simple assign alt' 'variable=foo; echo ${variable}' 0
    run_test_input 'simple multiple assign' 'variable=foo v=vier; echo ${variable} $v' 0
    run_test_input 'reassign' 'variable=foo v=vier; echo ${variable} $v; variable=bar; echo ${variable}' 0
    run_test_input 'edge case' "touch '>';rm '>'; echo foo >\>; cat '>'" 0
else
    echo "Skipped input tests"
fi

if [[ $TEST == "all" || $TEST == "args" ]]; then
    echo
    echo -e "${BLUE}Testing arguments input...${RESET}"
    run_test "Simple echo" "echo Hello World" 0
    run_test "echo with semicolon" "echo foo; echo bar" 0
    run_test "multiples echo, multiples args" "echo 1 2; echo 3 4; echo a b; echo c d" 0
    run_test "echo many flags" "echo -E -E -E -e -E -e -E -e -E -e '=>'" 0
    run_test "echo EeE" "echo '\\\\'" 0
    run_test "echo e tab" "echo -e '42\tsh'" 0
    run_test "echo E backslash n" "echo -E '42\nsh'" 0
    run_test "a lot of echo" "echo 1; echo 2; echo 3; echo 4; echo 5; echo 6; echo 7;" 0
    run_test "simple comment handling" "#This is a comment
    echo Hello" 0
    run_test "less simpler comment handling" "#comment
    #another comment
    echo should print
    #echp should not print" 0
    run_test "complex comment" 'echo \#escaped "#"quoted not#first #commented' 0
    run_test "many args" "echo Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Egestas purus viverra accumsan in nisl nisi." 0
    run_test "echo two args" "echo arg1 arg2" 0
    run_test "echo esc" "echo -e \\ | cat -e" 0
    run_tesr "echo esc2" "echo \\" 0
    run_test "echo two spaced args" "echo 1           2" 0
    run_test "echo flag -n" "echo -n foo; echo bar" 0
    run_test "echo flag -e" "echo -e fo\no" 0
    run_test "echo flag -E" "echo -E foo\nbar; echo baz" 0
    run_test "echo flag -ne" "echo -ne foo\n; echo -ne bar\n; echo baz" 0
    run_test "echo flags" "echo -neE foo\nbar\tbaz; echo test" 0
    run_test "echo flags multiple" "echo -neEEenneEnEe foo\nbar\tbaz; echo test" 0
    run_test "complex echo" "echo \' this shou\|d pr\|nt \@nd n\* problem " 0
    run_test "true builtin" "true" 0
    run_test "false builtin" "false" 1
    run_test "If statement (true)" \
        "if true; then echo success; else echo fail; fi" 0
    run_test "If statement (false)" \
        "if false; then echo failed; else echo success; fi" 0
    run_test "multiples if (true)" "if if echo test; then true; else false; fi; then echo all good; else how are we here; fi" 0
    run_test "multiples if (false)" "if if echo test; then false; else true; fi; then echo all good; else echo how are we here; fi" 0
    run_test "complex if" "if if if echo 1; false; then echo should not print; else true; fi; then echo should print; false; else echo should not print; fi; then echo should not print; elif echo good; true; then echo nice work; else echo not good at all; fi" 0
    run_test "Simple elif" "if echo test; false; then echo not supposed to write; elif echo good; true; then echo should display; else echo not good; fi" 0
    run_test "Complex elif" "if echo test; false; then echo not supposed to write; elif echo good; false; then echo should display; elif echo test still; false; then false; elif true; then echo good; else echo not good; fi" 0
    run_test "if compound list" "if echo 1; echo 2; echo 3; then echo all good; else echo not good; fi" 0
    run_test "if long cmpd list" "if true; false; true; false; true; false; then false; else echo all good; fi" 0
    run_test "complex if elif cmpd list" \
        "if if if true; false; echo 1; echo 2; false; then echo 1; true; else echo 2; echo 3; fi; then if echo 4; echo 5; false; then echo 45; true; fi; echo 2; else echo 3; ls; true; fi; then echo 789; else echo oskour; fi" 0
    run_test "redir in if" "if echo foo; then true; else false; fi > abc.txt; cat abc.txt" 0
    run_test "Single Quotes" "echo '; echo foo'" 0
    run_test "Empty" "" 0
    run_test "not a command" "notreal" 127
    run_test "not a command again" "notrealecho aabbc" 127
    run_test "sequence 5" "seq 5" 0
    run_test "space seq 5" "     seq     5" 0
    run_test "seq 2 args" "seq 5 10" 0
    run_test "seq 3 args" "seq 0 2 10" 0
    run_test "echo into echo" "echo echo Hello" 0
    run_test "simple for" "for i in 1 2 3 4; do echo 1; done" 0
    run_test "less simple for" 'for i in 1 2 3 4; do echo $i; done' 0
    mkdir qqch
    touch qqch/rien.txt
    run_test "simple cd" "cd qqch; ls -a" 0
    run_test "simple cd -" "cd qqch; cd -; ls -a" 0
    rm -rf qqch
    run_test "cd no args" "cd; ls -a" 0
    run_test "cd nonexistant" "cd fklasvblw" 1
    run_test "exit ?" "exit 0" 0
    run_test "simple bckquote" 'echo `echo foo`' 0
    run_test "expnd cmd \$()" 'echo foo $(echo bar) | cat -e' 0
    run_test "edge cmd \$()" 'echo foo $(echo baz $(echo bar) )  | cat -e' 0
    run_test "test cmd_sub" 'i=$(echo foo); echo $i | cat -e' 0
    run_test "simple export" 'est=123; export test; printenv test' 0
    run_test "multiple export" 'x=1; y=2; z=3; export x y z; printenv x y z' 0
    run_test "export inside" 'export x=10 y=20 z=30; printenv x y z' 0
    run_test "dot on new file" "echo 'echo foo' > dot.sh; . dot.sh" 0
    run_test "dot" ". tests/shell_scripts/Hello_World.sh" 0
    run_test "simple unset" 'dec=abc; echo $dec; unset dec; echo $dec' 0
    run_test "unset -v flag" 'dec=abc; echo $dec; unset -v dec; echo $dec' 0
    run_test "error flag unset" 'dec=abc; echo $dec; unset - dec; echo $dec' 0
    run_test "unset -f flag" 'foo() { echo abc; }; foo; unset -f foo; foo' 127
    run_test "empty single quotes" "''" 127
    run_test "empty double quotes" '""' 127
    run_test "! true" "! true" 1
    run_test "! false" "! false" 0
    run_test "! cat unknown" "if ! cat dkjfbngvnod; then echo good; else echo pas bon; fi" 0
    run_test "simple alias" "alias foo='echo bar' 
    alias" 0
    run_test "ls alias" "alias ll='ls -a'
    ll
    " 0
    run_test "print alias" "alias a='jsp'
    alias b='nn'
    alias a" 0
    run_test "print unalias" "alias a='jsp'
    alias b='nn'
    alias a
    unalias a
    alias a" 0
    run_test "ls" "ls" 0
    run_test "cat" "cat ../src/42sh.c" 0
    run_test "basic redir" "echo biktor > BIKTOR.txt; cat BIKTOR.txt" 0
    touch t.txt
    run_test "complex redir" "echo toto > test5.txt; echo foo >> test5.txt; cat test5.txt > t.txt; cat t.txt" 0
    run_test "error redir" "echo thomas > ship.txt >" 2
    run_test "multiple complex redir" "echo 1 2 3 > mul_redir.txt 1 2 3 > mul_redir2.txt > cat < mul_redir.txt" 0
    run_test "stderr redir" "echo Error 1>&2" 0
    run_test "redir dev/null" "echo no good >/dev/null" 0
    run_test "reverse basic redir" "> BIKTOR.txt echo biktor; cat BIKTOR.txt" 0
    run_test "reverse complex redir" "> test.txt echo toto; >> test.txt echo foo; < test.txt cat;" 0
    run_test "reverse stderr redir" "1>&2 echo Error" 0
    run_test "both sides stderr redir" "1>&2 echo Error 1>&2" 0
    run_test "redir dev/null" "echo no good >/dev/null" 0
    run_test "simple pipe" "echo pipe | cat -e" 0
    run_test "multiple pipes" "ls -l | wc | wc" 0
    run_test "wrong command redir" 'mangemonvier 1 2 3 > test6.txt; echo $? ; cat test6.txt' 0
    run_test "redir cat" "rm test7.txt; echo Hello > test7.txt; cat test7.txt" 0
    run_test "bad separators" "if true then; echo not good" 2
    run_test "bad separators 2" "if echo check; then echo still good; else echo not good fi" 2
    run_test "while false" "while false; do echo hello; done" 0
    run_test "until true" "until true; do echo hello; done" 0
    run_test "or false" "if false || false; then echo not good; else echo good; fi" 0
    run_test "or true" "if true || echo 1; then echo good; else echo not good; fi" 0
    run_test "or true false" "if echo 1 || echo 2; then echo good; else echo not good; fi" 0
    run_test "or true true" "if true || true; then echo good; else echo not good; fi" 0
    run_test "and false false" "if false && false; then echo good; else echo not good; fi" 0
    run_test "and false true" "if false && true; then echo good; else echo not good; fi" 0
    run_test "and true false" "if true && false; then echo good; else echo not good; fi" 0
    run_test "and true true" "if true && true; then echo good; else echo not good; fi" 0
    run_test "and echo" "if echo 1 && echo 2; then true; else false; fi" 0
    run_test "and or echo" "if echo 1 || echo 2 && echo 3 || false || echo 1 && true; then echo 1; else echo 2; fi" 0
    run_test "false multiples cons and" "if echo 1 && echo 2 && false && echo 3 && echo 4; then echo this should not print; else echo this should print; fi" 0
    run_test "true multiple cons and" "if echo 1 && echo 2 && true && echo 3 && echo 4; then echo this good; else echo no good; fi" 0
    run_test "simple subshell" 'a=sh; (a=42; echo -n $a); echo $a' 0
    run_test "true multiple cons and" "if echo 1 && echo 2 && true && echo 3 && echo 4; then echo this good; else echo no good; fi" 0
    run_test '$IFS' 'echo $IFS' 0
    run_test '$PWD' 'echo $PWD' 0
    run_test '$OLDPWD' 'echo $OLDPWD' 0
    run_test 'simple assign' 'i=1; echo $i' 0
    run_test 'simple assign alt' 'variable=foo; echo ${variable}' 0
    run_test 'simple multiple assign' 'variable=foo v=vier; echo ${variable} $v' 0
    run_test 'reassign' 'variable=foo v=vier; echo ${variable} $v; variable=bar; echo ${variable}' 0
    run_test 'weird_name' 'la_variable=2 ; echo ${la_variable};' 0
    run_test 'wrong variable' '1=2; echo $?;echo $1;' 0
    run_test 'echo tab' 'echo 1		2 3 soleil' 0
    run_test 'command spaces' '    	seq 1		10   	' 0
else
    echo "Skipped arguments tests"
fi

if [[ $TEST == "all" || $TEST == "file" ]]; then
    echo
    echo -e "${BLUE}Testing file input...${RESET}"
    run_test_file "File Hello World" "shell_scripts/Hello_World.sh" 0 ""
    run_test_file "File complex_chars" "shell_scripts/complex_chars.sh" 0 ""
    run_test_file "File semicolon" "shell_scripts/semicolon.sh" 0 ""
    run_test_file "File if_true" "shell_scripts/if_true.sh" 0 ""
    run_test_file "File if_false" "shell_scripts/if_false.sh" 0 ""
    run_test_file "File comment_handle" "shell_scripts/comment_handle.sh" 0 ""
    run_test_file "File single_quotes" "shell_scripts/single_quotes.sh" 0 ""
    run_test_file "File 42shsh42" "shell_scripts/42shsh42.sh" 0 ""
    run_test_file "File tiger_miam" "shell_scripts/tiger_miam.sh" 1 ""
    run_test_file "File list_numbers" "shell_scripts/list_numbers.sh" 0 ""
    run_test_file "File var1" 'shell_scripts/var$1.sh' 0 "1"
    run_test_file "File var2" 'shell_scripts/var$*.sh' 0 "1"
    run_test_file "File var3" 'shell_scripts/var$?.sh' 0 "1"
    run_test_file "File varUID" "shell_scripts/varuid.sh" 0 ""
    run_test_file "File PWD" "shell_scripts/varpwd.sh" 0 ""
    run_test_file "File OLDPWD" "shell_scripts/varold.sh" 0 ""
    run_test_file "File all_args" 'shell_scripts/var$@.sh' 0 "1"
    run_test_file "File simple break" "shell_scripts/simple_break.sh" 0 ""
    run_test_file "File break 2" "shell_scripts/break2.sh" 0 ""
    run_test_file "File simple continue" "shell_scripts/simple_continue.sh" 0 ""
    run_test_file "File continue 2" "shell_scripts/continue2.sh" 0 ""
    run_test_file "File simple function" "shell_scripts/simple_func.sh" 0 ""
    run_test_file "File func_in_func" "shell_scripts/func_in_func.sh" 0 ""
    run_test_file "File big house" "shell_scripts/house.sh" 0 ""
    run_test_file "File BIG" "shell_scripts/gros_prout.sh" 0 "1"
    run_test_file "File big house" "shell_scripts/house.sh" 0 ""
    run_test_file "File func override" "shell_scripts/override.sh" 0 ""
    run_test_file "File var assign" "shell_scripts/assignment.sh" 0 ""
    run_test_file "File func var" "shell_scripts/funkinox.sh" 0 ""
    run_test_file "File escape newline" "shell_scripts/escape_newline.sh" 0 ""
    run_test_file "File shell redirs" "shell_scripts/shell_redirs.sh" 0 ""
    run_test_file "File echoes" "shell_scripts/echoes.sh" 0 ""
    run_test_file "File case basic" "shell_scripts/case_simple.sh" 0 ""
    run_test_file "File case default" "shell_scripts/case_default.sh" 0 ""
    run_test_file "File case multi choice" "shell_scripts/case_multiple_choice.sh" 0 ""
    run_test_file "File case variables" "shell_scripts/case_var_choice.sh" 0 ""

else
    echo "Skipped file tests"
fi
echo -e "${BLUE}========== TEST RESULTS ==========${RESET}"
echo -e "${GREEN}Passed: ${RESET}$TEST_SUCCESS"
echo -e "${RED}Failed: ${RESET}$((TEST_COUNT - TEST_SUCCESS))"
echo -e "${BLUE}Total Tests: ${RESET}$TEST_COUNT"

rm *.txt
rm cat
rm '>'
rm dot.sh
if [[ -z ${OUTPUT_FILE} ]]; then
    echo "Done !"
else
    percentage=$((TEST_SUCCESS * 100 / TEST_COUNT))
    echo "$percentage" >"$OUTPUT_FILE"
fi
