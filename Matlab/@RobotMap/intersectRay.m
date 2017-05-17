function [dist, wallID] = intersectRay(map, xo, yo, theta)
%INTERSECTRAY: find the intersection point of a ray and the nearest wall
%   INPUTS:
%   map         A RobotMap mapect
%   x,y         Starting coordinates of the ray to be checked
%               for intersection will all walls
%   theta       Angle from x-axis -> set ray heading

%   OUTPUTS:
%   dist        Distance to intersection from ray's origin
%   wallID      Index of the wall the ray intersected with

% Get Wall matrix from map object
wall = map.wall;

%Wall starting point
ax = wall(:, 1);
ay = wall(:, 2);

%Wall End point
bx = wall(:, 3);
by = wall(:, 4);

dist = nan(1, length(xo));
wallID = nan(1, length(xo));

for i = 1:length(xo)
    %Vector Pointing perpendicular to ray d= [cost; sint]
    sint = sin(theta(i));
    cost = cos(theta(i));
    
    %Denominator Term
    den = ay*cost - by*cost - ax*sint + bx*sint;
    
    % Solve for Intersection
    t1 = -(ax.*by - ay.*bx + ay.*xo - ax.*yo - by.*xo + bx.*yo)./den;
    t2 = (ay.*cost - ax.*sint - cost.*yo + sint.*xo)./den;
    
    % Check for intersection
    isect = (t1 >= 0) & (t2 >=0) & (t2 <= 1) & abs(den) >= 1e-6;
    isect = find(isect);
    
    if any(isect)
        % Find closet intersection
        [dist(i), I] = min(t1(isect));
        wallID(i) = isect(I);
    end
end
end