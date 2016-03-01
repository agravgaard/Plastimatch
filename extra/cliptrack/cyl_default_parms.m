function parms = cyl_default_parms()

%% How often to pause and display to the screen
parms.display_rate = 0; 

%% Window size of pattern
parms.ws = 23;

%% Search window size (search size 50 is not enough for some cases)
parms.ss = 70;

%% Template library
ws = parms.ws;
parms.template_type = 'cylinder';
parms.template_library = make_cyl_template_library(ws);
