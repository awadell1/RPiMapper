function theta = intersectAngle(map, wallID, theta)
%INTERSECTPOINT: find the x/y coordinates of the intersection
%   of a given line vector and any known wall
%
%   INPUTS:
%   map         A RobotMap mapect
%   wallID      The index of the wall to be checked
%   theta       Angle from x-axis -> set ray heading

%   OUTPUTS:
%   theta       Intersection Angle

%Vector Pointing along ray d= [cost; sint]
sint = sin(theta);
cost = cos(theta);

% Get Wall matrix from map object
wall = map.wall(wallID, :);

%Wall starting point
ax = wall(:, 1);
ay = wall(:, 2);

%Wall End point
bx = wall(:, 3);
by = wall(:, 4);

% dot Product with perpendicular
X = - cost*(ax - bx) - sint*(ay - by);

% dot Product with ray
Y = sint*(ax - bx) - cost*(ay - by);

% Compute Theta
theta = atan2(X, Y);