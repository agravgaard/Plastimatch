function f = search_metric (x)

    global curves;
    global depths;
    global energies;
    global sobp_range;
    sobp = curves * x;

    flat_region = depths>=sobp_range(1) & depths<=sobp_range(2);
    other_region = ~flat_region;

    % Should be close to 15 in flat region
    diff = abs(sobp(flat_region)-15);

    % Should be less than 15 in other region
    excess = sobp(other_region) - 15;
    excess = (excess > 0) .* excess;

    % Blend of max and MS difference
    f_max = 0.5 * max(diff);
    f_mse = 0.05 * diff'* diff;
    f_excess = 0.1 * excess' * excess;
    f1 = f_max + f_mse + f_excess;
    disp (sprintf ('%f %f %f', f_max, f_mse, f_excess));

    % Penalty for negative weights
    xmin = min(x);
    f2 = 0;
    if (xmin < 0.01)
        f2 = 10000 * (1.01 - exp(xmin));
    end

    f = max(f1,f2);
