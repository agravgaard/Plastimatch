ls *.c* fatm/src/*.c* \
    | grep -v bstrlib.c \
    | grep -v lbfgsb_2_1.c \
    | grep -v findscu.cc \
    | grep -v movescu.cc \
    | xargs cppcheck --enable=all
