% Create map
l = 2;  %Wall length
walls = [0 0 l 0;...
         l 0 l l;...
         l l 0 l;...
         0 l 0 0];

map = RobotMap(walls);

%% Deadon
[dist, ~] = map.intersectRay(l/2, l/2, 0);

% Check answer
assert(dist == l/2)

%% Angle
theta = pi/6;
[dist] = map.intersectRay(l/2, l/2, theta);

% Check Answer
assert(abs(dist - l/(2*cos(theta))) < 1e-6)

%% Corner
theta = pi/4;
[dist, wall] = map.intersectRay(l/2, l/2, theta);

% Check Answer
assert(abs(dist - l/(2*cos(theta))) < 1e-6)

%% Corner Transition
dt = 1e-1;
theta = pi/4;
[~, wall1] = map.intersectRay(l/2, l/2, theta+dt);
[~, wall2] = map.intersectRay(l/2, l/2, theta-dt);

% Check Answer
assert(wall1 ~= wall2)