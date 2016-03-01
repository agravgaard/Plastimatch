function template_library = make_cyl_template_library(ws)

w = 2;
th_step = 2.5;
th_list = [0:th_step:180-th_step] * pi / 180 ;
l_step = 2;
l_list = [5,6:l_step:14];
%% l_list = 6:l_step:14;
for li=1:length(l_list)
  for thi=1:length(th_list)
    len = l_list(li);
    th = th_list(thi);
%%     pat_parms = [w,len,th,0,0,2,-1,1,w+5,3];
    pat_parms = [w,len,th,0,0,2,-1,1,w+6,3];
    [p1,w1] = cyl_template (ws,pat_parms);
    pattern_cache{li,thi,1} = p1;
    pattern_cache{li,thi,2} = w1;
    pattern_cache{li,thi,3} = (size(p1)-1)/2;
  end
end

template_library.l_step = l_step;
template_library.l_list = l_list;
template_library.th_step = th_step;
template_library.th_list = th_list;
template_library.template = pattern_cache;
