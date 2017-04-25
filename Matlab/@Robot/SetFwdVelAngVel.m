function SetFwdVelAngVel(robot, FwdVel, AngVel)
% SetFwdVelAngVel: Set FwdVel and AngVel of the Create Robot
%	INPUT

% Compute Wheel Speeds
d = 2*robot.wheel2center;

wheelVel = [1/2,	1/2;....
			-1/d,	1/d] \ [FwdVel; AngVel];

% Send Wheel Speeds to Robot
robot.sendPacket('wheelSpeed,%0.3f,%0.3f', wheelVel(1), wheelVel(2))