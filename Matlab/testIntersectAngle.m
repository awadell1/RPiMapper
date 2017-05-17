% Create map
l = 50;  %Wall length
walls = [l -l l l];

map = RobotMap(walls);

err = 1e-6;

%% Deadon
theta1 = 0;
theta = map.intersectAngle(1, theta1);

% Check answer
assert(abs(theta -theta1) < err)

%% Positive Angle
theta1 = pi/6;
theta = map.intersectAngle(1, theta1);

% Check answer
assert(abs(theta -theta1) < err)

%% Negative Angle
theta1 = -pi/6;
theta = map.intersectAngle(1, theta1);

% Check answer
assert(abs(theta -theta1) < err)