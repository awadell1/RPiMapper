function [inBeam, cellRange] = hasBeam(g,pose, beamWidth)
% hasBeam: Check which cells contain the a triangular beam
%
%   INPUTS
%       g			A gridData object
%		pose		The pose of the beam's origin [x, y, theta]
%		beamWidth	The beamwidth of the cone (Measured between diverging edges)
%   OUTPUTS
%       inBeam		A double representing the percentage of cell corners illuminated
%		cellRange	The average distance between the cell corners and the beam's origin
%
%   Cornell University
%   MAE 4180/5180 CS 3758: Autonomous Mobile Robots
%   Homework 5
%   Wadell, Alexius

%% Convert Grid Edge Coordinates to polar about the given pose
%Get coordinates of each corner
[X, Y] = g.cellCorners;

% Translate to current position
Xl = X - pose(1);
Yl = Y - pose(2);

%Convert to Polar
[theta, r] = cart2pol(Xl, Yl);

% Rotate to current orientation
theta = mod(theta - pose(3) +pi, 2*pi) -pi;

%% Find Cells within beam

% Check if corner is within beam
inBeamCorner = double(abs(theta) <= beamWidth/2);

% Interpolate corner state to decide if cell is within beam
[cellX, cellY] = g.cellCenter;
%inBeamCell =  interp2(X', Y', inBeamCorner', cellX, cellY, 'nearest');
inBeamCell = (inBeamCorner(1:end-1, 1:end-1) +....
			inBeamCorner(2:end, 1:end-1) +...
			inBeamCorner(1:end-1, 2:end) +....
			inBeamCorner(2:end, 2:end))/4;

% Also return cell distance from beam
cellRange = (r(1:end-1, 1:end-1) +....
			r(2:end, 1:end-1) +....
			r(1:end-1, 2:end) +....
			r(2:end, 2:end))/4;

%% Return as gridData objects
inBeam = gridData(g.edgeX, g.edgeY, inBeamCell);
cellRange = gridData(g.edgeX, g.edgeY, cellRange);
end