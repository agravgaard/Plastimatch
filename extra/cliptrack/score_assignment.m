function assignment_prob = score_assignment (hypos, assignment, rowcol_cur)

%% The clips shouldn't overlap
assignment_penalty_thresh = 100;

%% All the clips should move in the same direction
motion_variance_thresh = 100;

assignment_array = [];
for t=1:length(assignment)
  assignment_array = [assignment_array;hypos{t}(assignment(t),:)];
end

assignment_prob = prod(assignment_array(:,3));
for t=1:length(assignment)
  d = find_sq_distances (assignment_array(t,1:2),...
			 assignment_array([1:t-1,t+1:size(assignment_array,1)],1:2));
  if (min(d) <= assignment_penalty_thresh)
    assignment_prob = 0;
  end
  d = assignment_array(:,1:2) - rowcol_cur;
  if (sum(std(d).^2) > motion_variance_thresh)
    assignment_prob = 0;
  end
end
