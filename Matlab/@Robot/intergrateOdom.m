function [pose, J] = intergrateOdom(pose0, u, dt)
%	intergrateOdom: Estimates the robot's current pose using dead-reckoning
%	INPUTS
%		pose0		Previous robot configuration,  1x3 [x y theta]
%		u			Control input to robot [dDistance, dAngle]
%		dt			Time step between pose and pose0
%
%	OUTPUTS
%		pose		Updated pose based on control input
%		J			Jacobian of predition model

%Extract Parameters
V = u(:, 1);
w = u(:, 2);
theta = pose0(:, 3);

if nargin == 2
	dt = 1;
end

% Compute turning Radius
r = V ./ w;

% Compute new Pose
pose = [-r.*sin(theta) + r.*sin(theta + w*dt)  ,...
		 r.*cos(theta) - r.*cos(theta + w*dt) ,...
		 w.*dt] + pose0;

% Correct for r = inf
driveFwd = r == inf;
if any(driveFwd)
	pose(driveFwd,:) =  pose0(driveFwd,:);
	pose(driveFwd, 1:2) = pose(driveFwd, 1:2) + ...
		repmat(V(driveFwd), 1, 2) .*...
		[cos(theta(driveFwd)), sin(theta(driveFwd))];
end

if nargout == 2
	% Compute Jacobian
	J = [	1, 0, (V.*(cos(theta + dt*w) - cos(theta)))./w;...
			0, 1, (V.*(sin(theta + dt*w) - sin(theta)))./w;...
			0, 0,										1];
	J(driveFwd) = eye(3);
end
end