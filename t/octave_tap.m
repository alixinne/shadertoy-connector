function octave_tap(test_value, message)
    if test_value
        printf("ok %s\n", message)
    else
        printf("not ok %s\n", message)
    endif
endfunction
