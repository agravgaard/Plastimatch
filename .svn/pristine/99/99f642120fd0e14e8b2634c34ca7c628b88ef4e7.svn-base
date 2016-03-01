function assignment = create_assignment(hypos, assignment_no)

assignment = [];

max_assignment_no = 1;
for t=1:length(hypos)
  max_assignment_no = max_assignment_no * size(hypos{t},1);
end

if (assignment_no >= max_assignment_no)
  return;
end

ano = assignment_no;
for t=1:length(hypos)
  assignment = [assignment, 1+mod(ano,size(hypos{t},1))];
  ano = ano - mod(ano,size(hypos{t},1));
  ano = ano / size(hypos{t},1);
end
